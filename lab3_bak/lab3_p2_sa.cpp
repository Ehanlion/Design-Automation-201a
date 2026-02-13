// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3
//
// Iteration 2 for Problem 2:
// Simulated annealing with swap-only legal moves.
// Constraints enforced:
//   1) Swaps only between identical cells (same master design).
//   2) Swaps only between instances with identical orientation.
//   3) No movement operation other than direct location swap.

#include "oaDesignDB.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <sys/time.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace oa;
using namespace std;

static oaNativeNS ns;

struct Point2D {
	oaInt4 x;
	oaInt4 y;
};

struct InstData {
	oaInst* inst;
	oaDesign* master;
	oaOrient orient;
	oaPoint origin;
	oaInt4 centerX;
	oaInt4 centerY;
	string cellName;
	vector<int> netIndices;
	bool locked;
	bool movable;
};

struct NetData {
	oaNet* net;
	vector<int> instIndices;
	vector<Point2D> termPoints;
	double hpwl;
};

struct IncrementalStats {
	int swapsApplied;
	long long legalPairsEvaluated;
};

void printDesignNames(oaDesign* design) {
	oaString libName;
	oaString cellName;
	oaString viewName;

	design->getLibName(ns, libName);
	design->getCellName(ns, cellName);
	design->getViewName(ns, viewName);

	cout << "\tThe library name for this design is : " << libName << endl;
	cout << "\tThe cell name for this design is : " << cellName << endl;
	cout << "\tThe view name for this design is : " << viewName << endl;
}

void getInstanceCenter(oaInst* inst, oaInt4& centerX, oaInt4& centerY) {
	oaBox instanceBBox;
	inst->getBBox(instanceBBox);
	oaPoint ll = instanceBBox.lowerLeft();
	oaPoint ur = instanceBBox.upperRight();
	centerX = (ll.x() + ur.x()) / 2;
	centerY = (ll.y() + ur.y()) / 2;
}

double normalizeHPWL(double hpwl) {
	return (hpwl < 0.0) ? 0.0 : hpwl;
}

double computeNetHPWL(const NetData& netData, const vector<InstData>& instances,
					  int swapInstA = -1, int swapInstB = -1) {
	bool initialized = false;
	oaInt4 minX = 0;
	oaInt4 minY = 0;
	oaInt4 maxX = 0;
	oaInt4 maxY = 0;

	auto expandBBox = [&](oaInt4 x, oaInt4 y) {
		if (!initialized) {
			minX = maxX = x;
			minY = maxY = y;
			initialized = true;
			return;
		}
		if (x < minX) minX = x;
		if (x > maxX) maxX = x;
		if (y < minY) minY = y;
		if (y > maxY) maxY = y;
	};

	for (int instIdx : netData.instIndices) {
		int effectiveIdx = instIdx;
		if (instIdx == swapInstA) {
			effectiveIdx = swapInstB;
		} else if (instIdx == swapInstB) {
			effectiveIdx = swapInstA;
		}
		expandBBox(instances[effectiveIdx].centerX, instances[effectiveIdx].centerY);
	}

	for (const Point2D& p : netData.termPoints) {
		expandBBox(p.x, p.y);
	}

	if (!initialized) {
		return -1.0;
	}
	return (double)((maxX - minX) + (maxY - minY));
}

bool isLegalSwapPair(const InstData& a, const InstData& b) {
	if (!a.movable || !b.movable || a.locked || b.locked) {
		return false;
	}

	bool sameCell = false;
	if (a.master && b.master) {
		sameCell = (a.master == b.master);
	} else {
		sameCell = (a.cellName == b.cellName);
	}
	if (!sameCell) {
		return false;
	}

	return ((oaOrientEnum)a.orient == (oaOrientEnum)b.orient);
}

void collectAffectedNets(const vector<InstData>& instances, int instA, int instB,
						 vector<int>& marks, int token, vector<int>& affected) {
	affected.clear();

	auto addFromInst = [&](int instIdx) {
		for (int netId : instances[instIdx].netIndices) {
			if (marks[netId] != token) {
				marks[netId] = token;
				affected.push_back(netId);
			}
		}
	};

	addFromInst(instA);
	addFromInst(instB);
}

void applySwap(vector<InstData>& instances, int instA, int instB) {
	oaPoint originA = instances[instA].origin;
	oaPoint originB = instances[instB].origin;

	instances[instA].inst->setOrigin(originB);
	instances[instB].inst->setOrigin(originA);

	instances[instA].origin = originB;
	instances[instB].origin = originA;
	swap(instances[instA].centerX, instances[instB].centerX);
	swap(instances[instA].centerY, instances[instB].centerY);
}

void buildPlacementData(oaDesign* design, vector<InstData>& instances,
						vector<NetData>& nets) {
	instances.clear();
	nets.clear();

	oaBlock* block = design->getTopBlock();
	if (!block) {
		return;
	}

	unordered_map<oaInst*, int> instIndex;

	oaIter<oaInst> instIterator(block->getInsts());
	while (oaInst* inst = instIterator.getNext()) {
		InstData instData;
		instData.inst = inst;
		instData.master = inst->getMaster();
		instData.orient = inst->getOrient();
		inst->getOrigin(instData.origin);
		getInstanceCenter(inst, instData.centerX, instData.centerY);

		oaString cellName;
		inst->getCellName(ns, cellName);
		instData.cellName = string((const char*)cellName);

		oaPlacementStatusEnum placementStatus =
			(oaPlacementStatusEnum)inst->getPlacementStatus();
		instData.movable = !(placementStatus == oacFixedPlacementStatus ||
							 placementStatus == oacLockedPlacementStatus);
		instData.locked = false;

		int idx = (int)instances.size();
		instances.push_back(instData);
		instIndex[inst] = idx;
	}

	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		int netId = (int)nets.size();
		NetData netData;
		netData.net = net;

		oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
		while (oaInstTerm* instTerm = instTermIterator.getNext()) {
			oaInst* inst = instTerm->getInst();
			auto found = instIndex.find(inst);
			if (found == instIndex.end()) {
				continue;
			}
			int instIdx = found->second;
			netData.instIndices.push_back(instIdx);
			instances[instIdx].netIndices.push_back(netId);
		}

		oaIter<oaTerm> termIterator(net->getTerms());
		while (oaTerm* term = termIterator.getNext()) {
			oaIter<oaPin> pinIterator(term->getPins());
			while (oaPin* pin = pinIterator.getNext()) {
				oaIter<oaPinFig> pinFigIterator(pin->getFigs());
				while (oaPinFig* pinFig = pinFigIterator.getNext()) {
					oaBox pinBox;
					pinFig->getBBox(pinBox);
					oaPoint ll = pinBox.lowerLeft();
					oaPoint ur = pinBox.upperRight();
					Point2D p;
					p.x = (ll.x() + ur.x()) / 2;
					p.y = (ll.y() + ur.y()) / 2;
					netData.termPoints.push_back(p);
				}
			}
		}

		netData.hpwl = computeNetHPWL(netData, instances);
		nets.push_back(netData);
	}

	for (InstData& instData : instances) {
		sort(instData.netIndices.begin(), instData.netIndices.end());
		instData.netIndices.erase(
			unique(instData.netIndices.begin(), instData.netIndices.end()),
			instData.netIndices.end());
	}
}

double computeTotalHPWLAndStats(const vector<NetData>& nets, int& validNets,
								int& skippedNets) {
	double total = 0.0;
	validNets = 0;
	skippedNets = 0;

	for (const NetData& netData : nets) {
		if (netData.hpwl < 0.0) {
			skippedNets++;
		} else {
			validNets++;
		}
		total += normalizeHPWL(netData.hpwl);
	}

	return total;
}

IncrementalStats runSimulatedAnnealingPlacement(vector<InstData>& instances,
												vector<NetData>& nets,
												double& totalHPWL) {
	IncrementalStats stats;
	stats.swapsApplied = 0;
	stats.legalPairsEvaluated = 0;

	if (instances.empty() || nets.empty()) {
		return stats;
	}

	vector<pair<int, int>> legalPairs;
	for (int i = 0; i < (int)instances.size(); i++) {
		if (!instances[i].movable) {
			continue;
		}
		for (int j = i + 1; j < (int)instances.size(); j++) {
			if (!instances[j].movable) {
				continue;
			}
			if (isLegalSwapPair(instances[i], instances[j])) {
				legalPairs.emplace_back(i, j);
			}
		}
	}

	if (legalPairs.empty()) {
		return stats;
	}

	struct timeval wallStart;
	gettimeofday(&wallStart, NULL);

	uint64_t seed = (uint64_t)wallStart.tv_sec * 1000000ull +
					(uint64_t)wallStart.tv_usec + (uint64_t)legalPairs.size();
	mt19937_64 rng(seed);
	uniform_real_distribution<double> unitDist(0.0, 1.0);
	uniform_int_distribution<size_t> pairDist(0, legalPairs.size() - 1);

	vector<int> marks(nets.size(), 0);
	vector<int> affected;
	vector<double> candidateHPWL;
	int token = 1;

	auto collectSwapCost = [&](int instA, int instB, double& oldAffected,
							   double& newAffected) {
		if (token == INT_MAX) {
			fill(marks.begin(), marks.end(), 0);
			token = 1;
		}
		collectAffectedNets(instances, instA, instB, marks, token, affected);
		token++;

		oldAffected = 0.0;
		newAffected = 0.0;
		candidateHPWL.clear();
		candidateHPWL.reserve(affected.size());

		for (int netId : affected) {
			oldAffected += normalizeHPWL(nets[netId].hpwl);
			double swappedHPWL = computeNetHPWL(nets[netId], instances, instA, instB);
			candidateHPWL.push_back(swappedHPWL);
			newAffected += normalizeHPWL(swappedHPWL);
		}
	};

	int sampleCount = min((int)legalPairs.size(), 256);
	double worsenDeltaSum = 0.0;
	int worsenDeltaCount = 0;
	for (int s = 0; s < sampleCount; s++) {
		const pair<int, int>& move = legalPairs[pairDist(rng)];
		double oldAffected = 0.0;
		double newAffected = 0.0;
		collectSwapCost(move.first, move.second, oldAffected, newAffected);
		double deltaCost = newAffected - oldAffected;
		if (deltaCost > 0.0) {
			worsenDeltaSum += deltaCost;
			worsenDeltaCount++;
		}
	}

	double temperature = 1.0;
	if (worsenDeltaCount > 0) {
		temperature = max(1.0, (worsenDeltaSum / (double)worsenDeltaCount) * 0.20);
	} else {
		temperature = max(1.0, totalHPWL * 1e-6);
	}

	const double minTemperature = max(1e-2, temperature * 1e-3);
	const double coolingAlpha = 0.95;
	const int movesPerTemp = max(100, min(1200, (int)legalPairs.size() / 8 + 1));
	const long long maxMoves =
		min(250000LL, max(50000LL, (long long)legalPairs.size() * 20LL));
	const double maxRuntimeSec = 8.0;

	double bestHPWL = totalHPWL;
	vector<oaPoint> bestOrigins(instances.size());
	vector<oaInt4> bestCenterX(instances.size());
	vector<oaInt4> bestCenterY(instances.size());

	auto captureBestPlacement = [&]() {
		for (int i = 0; i < (int)instances.size(); i++) {
			bestOrigins[i] = instances[i].origin;
			bestCenterX[i] = instances[i].centerX;
			bestCenterY[i] = instances[i].centerY;
		}
	};
	captureBestPlacement();

	// Warm-start SA with a few steepest-descent swaps before annealing.
	const int warmStartMaxSwaps = 24;
	for (int warmStep = 0; warmStep < warmStartMaxSwaps; warmStep++) {
		double warmStartBestImprove = 0.0;
		int warmStartA = -1;
		int warmStartB = -1;
		vector<int> warmStartAffected;
		vector<double> warmStartHPWL;

		for (const pair<int, int>& move : legalPairs) {
			double oldAffected = 0.0;
			double newAffected = 0.0;
			collectSwapCost(move.first, move.second, oldAffected, newAffected);
			stats.legalPairsEvaluated++;

			double improve = oldAffected - newAffected;
			if (improve > warmStartBestImprove + 1e-6) {
				warmStartBestImprove = improve;
				warmStartA = move.first;
				warmStartB = move.second;
				warmStartAffected = affected;
				warmStartHPWL = candidateHPWL;
			}
		}

		if (warmStartA < 0 || warmStartB < 0 || warmStartBestImprove <= 1e-6) {
			break;
		}

		applySwap(instances, warmStartA, warmStartB);
		for (int k = 0; k < (int)warmStartAffected.size(); k++) {
			nets[warmStartAffected[k]].hpwl = warmStartHPWL[k];
		}
		totalHPWL -= warmStartBestImprove;
		stats.swapsApplied++;
		if (totalHPWL + 1e-6 < bestHPWL) {
			bestHPWL = totalHPWL;
			captureBestPlacement();
		}
	}

	long long totalMovesTried = 0;
	int stagnationEpochs = 0;
	int swapsAtBest = stats.swapsApplied;

	while (temperature > minTemperature && totalMovesTried < maxMoves &&
		   stagnationEpochs < 12) {
		bool acceptedAnyMove = false;
		bool improvedBest = false;

		for (int moveIdx = 0; moveIdx < movesPerTemp; moveIdx++) {
			struct timeval now;
			gettimeofday(&now, NULL);
			double elapsedSec = (now.tv_sec - wallStart.tv_sec) +
								(now.tv_usec - wallStart.tv_usec) * 1e-6;
			if (elapsedSec >= maxRuntimeSec || totalMovesTried >= maxMoves) {
				stagnationEpochs = 12;
				break;
			}

			const pair<int, int>& move = legalPairs[pairDist(rng)];
			double oldAffected = 0.0;
			double newAffected = 0.0;
			collectSwapCost(move.first, move.second, oldAffected, newAffected);

			totalMovesTried++;
			stats.legalPairsEvaluated++;
			double deltaCost = newAffected - oldAffected;

			bool acceptMove = false;
			if (deltaCost < -1e-6) {
				acceptMove = true;
			} else if (fabs(deltaCost) <= 1e-6) {
				acceptMove = (unitDist(rng) < 0.05);
			} else {
				double acceptanceProb = exp(-deltaCost / temperature);
				acceptMove = (unitDist(rng) < acceptanceProb);
			}
			if (!acceptMove) {
				continue;
			}

			acceptedAnyMove = true;
			applySwap(instances, move.first, move.second);

			for (int k = 0; k < (int)affected.size(); k++) {
				nets[affected[k]].hpwl = candidateHPWL[k];
			}

			totalHPWL += deltaCost;
			stats.swapsApplied++;

			if (totalHPWL + 1e-6 < bestHPWL) {
				bestHPWL = totalHPWL;
				captureBestPlacement();
				swapsAtBest = stats.swapsApplied;
				improvedBest = true;
			}
		}

		if (!acceptedAnyMove && !improvedBest) {
			stagnationEpochs++;
		} else {
			stagnationEpochs = 0;
		}
		temperature *= coolingAlpha;
	}

	if (bestHPWL + 1e-6 < totalHPWL) {
		for (int i = 0; i < (int)instances.size(); i++) {
			if (instances[i].origin.x() != bestOrigins[i].x() ||
				instances[i].origin.y() != bestOrigins[i].y()) {
				instances[i].inst->setOrigin(bestOrigins[i]);
			}
			instances[i].origin = bestOrigins[i];
			instances[i].centerX = bestCenterX[i];
			instances[i].centerY = bestCenterY[i];
		}
		for (int netId = 0; netId < (int)nets.size(); netId++) {
			nets[netId].hpwl = computeNetHPWL(nets[netId], instances);
		}
		totalHPWL = bestHPWL;
	}
	stats.swapsApplied = swapsAtBest;

	return stats;
}

int main(int argc, char* argv[]) {
	try {
		oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);

		oaString libPath("./DesignLib");
		oaString library("DesignLib");
		oaViewType* viewType = oaViewType::get(oacMaskLayout);
		oaString cell("s1196_bench");
		oaString view("layout");
		oaScalarName libraryName(ns, library);
		oaScalarName cellName(ns, cell);
		oaScalarName viewName(ns, view);

		opnTechConflictObserver myTechConflictObserver(1);
		opnLibDefListObserver myLibDefListObserver(1);

		oaLib* lib = oaLib::find(libraryName);
		if (!lib) {
			if (oaLib::exists(libPath)) {
				lib = oaLib::open(libraryName, libPath);
			} else {
				char* DMSystem = getenv("DMSystem");
				if (DMSystem) {
					lib = oaLib::create(libraryName, libPath, oacSharedLibMode, DMSystem);
				} else {
					lib = oaLib::create(libraryName, libPath);
				}
			}
			if (lib) {
				updateLibDefsFile(libraryName, libPath);
			} else {
				cerr << "ERROR: Unable to create/open " << libPath << "/" << library
					 << endl;
				return 1;
			}
		}

		cout << "The design is created and opened in 'append' mode." << endl;
		oaDesign* design =
			oaDesign::open(libraryName, cellName, viewName, viewType, 'a');

		printDesignNames(design);

		vector<InstData> instances;
		vector<NetData> nets;
		buildPlacementData(design, instances, nets);

		int validNets = 0;
		int skippedNets = 0;
		double totalHPWL = computeTotalHPWLAndStats(nets, validNets, skippedNets);
		double originalTotalHPWL = totalHPWL;

		cout << endl
			 << "----- Ethan Owen: Problem 1 -----" << endl;
		cout << "\tTotal nets processed: " << nets.size() << endl;
		cout << "\tNets with valid HPWL: " << validNets << endl;
		cout << "\tNets without endpoints (counted as 0 HPWL): " << skippedNets
			 << endl;
		cout << "\tTotal HPWL: " << fixed << setprecision(0) << totalHPWL
			 << " DBU" << endl;

		cout << endl
			 << "----- Ethan Owen: Problem 2 -----" << endl;
		struct timeval start;
		struct timeval end;
		gettimeofday(&start, NULL);

		IncrementalStats stats =
			runSimulatedAnnealingPlacement(instances, nets, totalHPWL);

		gettimeofday(&end, NULL);
		double timeTakenSec = (end.tv_sec - start.tv_sec) * 1e6;
		timeTakenSec = (timeTakenSec + (end.tv_usec - start.tv_usec)) * 1e-6;

		double scoreMetric = totalHPWL * totalHPWL * timeTakenSec;

		cout << endl;
		cout << "Problem 1 -- Total wirelength of original design: "
			 << fixed << setprecision(0) << originalTotalHPWL << " DBU"
			 << endl;
		cout << "Problem 2 -- Total wirelength AFTER my incremental placement "
				"algorithm:  "
			 << fixed << setprecision(0) << totalHPWL << " DBU" << endl;
		cout << "Problem 2 -- Total number of swaps used:  " << stats.swapsApplied
			 << endl;
		cout << "Problem 2 -- Legal swap pairs evaluated:  "
			 << stats.legalPairsEvaluated << endl;
		cout << "Problem 2 -- Time taken:  " << fixed << setprecision(6)
			 << timeTakenSec << " sec" << endl;
		cout << "Problem 2 -- Score metric (Total HPWL^2 * runtime):  "
			 << scientific << setprecision(6) << scoreMetric << endl;

		design->close();
		lib->close();

	} catch (oaCompatibilityError& ex) {
		handleFBCError(ex);
		exit(1);
	} catch (oaException& excp) {
		cout << "ERROR: " << excp.getMsg() << endl;
		exit(1);
	}

	return 0;
}
