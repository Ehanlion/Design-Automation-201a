// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3
//
// Problem 1:
//   Compute total HPWL for all nets in the design.
//
// Problem 2:
//   Creative incremental placement approach:
//   Iterated local search with swap-only legal moves.
//   - Exhaustive steepest-descent to local minimum
//   - Controlled "kick" swaps to escape local minima
//   - Track and restore globally best HPWL placement

#include "oaDesignDB.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
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
	bool movable;
};

struct NetData {
	oaNet* net;
	vector<int> instIndices;
	vector<Point2D> termPoints;
	double hpwl;
};

struct SwapEvaluation {
	int instA;
	int instB;
	double oldAffectedHPWL;
	double newAffectedHPWL;
	double improvement;
	vector<int> affectedNets;
	vector<double> affectedNewHPWL;
};

struct PlacementSnapshot {
	vector<oaPoint> origins;
	vector<oaInt4> centerX;
	vector<oaInt4> centerY;
};

struct OptimizerConfig {
	int maxRounds;
	int maxSteepestSwapsPerRound;
	int kicksPerRound;
	int kickCandidatePool;
	double maxRuntimeSec;
};

struct IncrementalStats {
	int swapsApplied;
	int swapsUsedForBest;
	int steepestRoundsCompleted;
	int localMinimaEscapes;
	long long legalPairsEvaluated;
	long long legalPairsAvailable;
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

double elapsedSeconds(const timeval& start, const timeval& end) {
	return (double)(end.tv_sec - start.tv_sec) +
		   (double)(end.tv_usec - start.tv_usec) * 1e-6;
}

bool runtimeExceeded(const timeval& algoStart, double maxRuntimeSec) {
	timeval now;
	gettimeofday(&now, NULL);
	return elapsedSeconds(algoStart, now) >= maxRuntimeSec;
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
	if (!a.movable || !b.movable) {
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

void capturePlacementSnapshot(const vector<InstData>& instances,
							  PlacementSnapshot& snapshot) {
	snapshot.origins.resize(instances.size());
	snapshot.centerX.resize(instances.size());
	snapshot.centerY.resize(instances.size());
	for (int i = 0; i < (int)instances.size(); i++) {
		snapshot.origins[i] = instances[i].origin;
		snapshot.centerX[i] = instances[i].centerX;
		snapshot.centerY[i] = instances[i].centerY;
	}
}

void restorePlacementSnapshot(vector<InstData>& instances,
							  const PlacementSnapshot& snapshot) {
	for (int i = 0; i < (int)instances.size(); i++) {
		if (instances[i].origin.x() != snapshot.origins[i].x() ||
			instances[i].origin.y() != snapshot.origins[i].y()) {
			instances[i].inst->setOrigin(snapshot.origins[i]);
		}
		instances[i].origin = snapshot.origins[i];
		instances[i].centerX = snapshot.centerX[i];
		instances[i].centerY = snapshot.centerY[i];
	}
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

void buildLegalSwapPairs(const vector<InstData>& instances,
						 vector<pair<int, int>>& legalPairs) {
	legalPairs.clear();
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
}

void evaluateSwap(int instA, int instB, const vector<InstData>& instances,
				  const vector<NetData>& nets, vector<int>& marks, int& token,
				  vector<int>& affected, vector<double>& candidateNewHPWL,
				  double& oldAffectedHPWL, double& newAffectedHPWL,
				  double& improvement) {
	if (token == numeric_limits<int>::max()) {
		fill(marks.begin(), marks.end(), 0);
		token = 1;
	}
	collectAffectedNets(instances, instA, instB, marks, token, affected);
	token++;

	oldAffectedHPWL = 0.0;
	newAffectedHPWL = 0.0;
	candidateNewHPWL.clear();
	candidateNewHPWL.reserve(affected.size());

	for (int netId : affected) {
		double oldHPWL = normalizeHPWL(nets[netId].hpwl);
		double swappedHPWL = computeNetHPWL(nets[netId], instances, instA, instB);
		oldAffectedHPWL += oldHPWL;
		newAffectedHPWL += normalizeHPWL(swappedHPWL);
		candidateNewHPWL.push_back(swappedHPWL);
	}

	improvement = oldAffectedHPWL - newAffectedHPWL;
}

void commitEvaluatedSwap(const SwapEvaluation& eval, vector<InstData>& instances,
						 vector<NetData>& nets, double& totalHPWL) {
	applySwap(instances, eval.instA, eval.instB);
	for (int k = 0; k < (int)eval.affectedNets.size(); k++) {
		nets[eval.affectedNets[k]].hpwl = eval.affectedNewHPWL[k];
	}
	totalHPWL += (eval.newAffectedHPWL - eval.oldAffectedHPWL);
}

int runSteepestDescentRound(const vector<pair<int, int>>& legalPairs,
							vector<InstData>& instances, vector<NetData>& nets,
							double& totalHPWL, const OptimizerConfig& config,
							vector<int>& marks, int& token, vector<int>& affected,
							vector<double>& candidateNewHPWL,
							IncrementalStats& stats, const timeval& algoStart) {
	const double improvementEps = 1e-6;
	int improvingSwaps = 0;

	while (improvingSwaps < config.maxSteepestSwapsPerRound &&
		   !runtimeExceeded(algoStart, config.maxRuntimeSec)) {
		SwapEvaluation bestSwap;
		bestSwap.instA = -1;
		bestSwap.instB = -1;
		bestSwap.improvement = 0.0;

		for (const pair<int, int>& legalPair : legalPairs) {
			double oldAffectedHPWL = 0.0;
			double newAffectedHPWL = 0.0;
			double improvement = 0.0;
			evaluateSwap(legalPair.first, legalPair.second, instances, nets, marks,
						 token, affected, candidateNewHPWL, oldAffectedHPWL,
						 newAffectedHPWL, improvement);
			stats.legalPairsEvaluated++;

			if (improvement > bestSwap.improvement + improvementEps) {
				bestSwap.instA = legalPair.first;
				bestSwap.instB = legalPair.second;
				bestSwap.oldAffectedHPWL = oldAffectedHPWL;
				bestSwap.newAffectedHPWL = newAffectedHPWL;
				bestSwap.improvement = improvement;
				bestSwap.affectedNets = affected;
				bestSwap.affectedNewHPWL = candidateNewHPWL;
			}
		}

		if (bestSwap.instA < 0 || bestSwap.instB < 0 ||
			bestSwap.improvement <= improvementEps) {
			break;
		}

		commitEvaluatedSwap(bestSwap, instances, nets, totalHPWL);
		stats.swapsApplied++;
		improvingSwaps++;
	}

	return improvingSwaps;
}

bool applyEscapeKick(const vector<pair<int, int>>& legalPairs,
					 vector<InstData>& instances, vector<NetData>& nets,
					 double& totalHPWL, const OptimizerConfig& config,
					 vector<int>& marks, int& token, vector<int>& affected,
					 vector<double>& candidateNewHPWL, IncrementalStats& stats,
					 mt19937_64& rng) {
	const double improvementEps = 1e-6;
	vector<SwapEvaluation> bestKickCandidates;
	bestKickCandidates.reserve(config.kickCandidatePool);

	for (const pair<int, int>& legalPair : legalPairs) {
		double oldAffectedHPWL = 0.0;
		double newAffectedHPWL = 0.0;
		double improvement = 0.0;
		evaluateSwap(legalPair.first, legalPair.second, instances, nets, marks,
					 token, affected, candidateNewHPWL, oldAffectedHPWL,
					 newAffectedHPWL, improvement);
		stats.legalPairsEvaluated++;

		// Kick phase should prefer near-neutral or low-penalty moves.
		if (improvement > improvementEps) {
			continue;
		}

		SwapEvaluation candidate;
		candidate.instA = legalPair.first;
		candidate.instB = legalPair.second;
		candidate.oldAffectedHPWL = oldAffectedHPWL;
		candidate.newAffectedHPWL = newAffectedHPWL;
		candidate.improvement = improvement;
		candidate.affectedNets = affected;
		candidate.affectedNewHPWL = candidateNewHPWL;

		if ((int)bestKickCandidates.size() < config.kickCandidatePool) {
			bestKickCandidates.push_back(candidate);
		} else {
			int worstIdx = 0;
			for (int i = 1; i < (int)bestKickCandidates.size(); i++) {
				if (bestKickCandidates[i].improvement <
					bestKickCandidates[worstIdx].improvement) {
					worstIdx = i;
				}
			}
			if (candidate.improvement > bestKickCandidates[worstIdx].improvement) {
				bestKickCandidates[worstIdx] = candidate;
			}
		}
	}

	if (bestKickCandidates.empty()) {
		return false;
	}

	uniform_int_distribution<int> pickKick(
		0, (int)bestKickCandidates.size() - 1);
	const SwapEvaluation& chosenKick = bestKickCandidates[pickKick(rng)];

	commitEvaluatedSwap(chosenKick, instances, nets, totalHPWL);
	stats.swapsApplied++;
	stats.localMinimaEscapes++;
	return true;
}

IncrementalStats runIteratedLocalSearch(vector<InstData>& instances,
										vector<NetData>& nets, double& totalHPWL,
										const OptimizerConfig& config) {
	IncrementalStats stats;
	stats.swapsApplied = 0;
	stats.swapsUsedForBest = 0;
	stats.steepestRoundsCompleted = 0;
	stats.localMinimaEscapes = 0;
	stats.legalPairsEvaluated = 0;
	stats.legalPairsAvailable = 0;

	if (instances.empty() || nets.empty()) {
		return stats;
	}

	// -------------------------------------------------------------------------
	// STEP 1: Build the complete legal swap space once.
	// -------------------------------------------------------------------------
	vector<pair<int, int>> legalPairs;
	buildLegalSwapPairs(instances, legalPairs);
	stats.legalPairsAvailable = (long long)legalPairs.size();
	if (legalPairs.empty()) {
		return stats;
	}

	PlacementSnapshot bestSnapshot;
	capturePlacementSnapshot(instances, bestSnapshot);
	double bestHPWL = totalHPWL;
	int swapsAtBest = 0;

	vector<int> marks(nets.size(), 0);
	int token = 1;
	vector<int> affected;
	vector<double> candidateNewHPWL;

	timeval algoStart;
	gettimeofday(&algoStart, NULL);
	uint64_t seed = (uint64_t)algoStart.tv_sec * 1000000ull +
					(uint64_t)algoStart.tv_usec + (uint64_t)legalPairs.size();
	mt19937_64 rng(seed);

	// -------------------------------------------------------------------------
	// STEP 2: Iterated local search.
	//   2a) Exhaustive steepest descent to local optimum.
	//   2b) Controlled kicks to escape local optimum.
	//   2c) Keep global best placement observed across all rounds.
	// -------------------------------------------------------------------------
	for (int round = 0; round < config.maxRounds; round++) {
		if (runtimeExceeded(algoStart, config.maxRuntimeSec)) {
			break;
		}

		int improvingSwaps = runSteepestDescentRound(
			legalPairs, instances, nets, totalHPWL, config, marks, token, affected,
			candidateNewHPWL, stats, algoStart);
		stats.steepestRoundsCompleted++;

		if (totalHPWL + 1e-6 < bestHPWL) {
			bestHPWL = totalHPWL;
			swapsAtBest = stats.swapsApplied;
			capturePlacementSnapshot(instances, bestSnapshot);
		}

		bool kicked = false;
		if (round + 1 < config.maxRounds && !runtimeExceeded(algoStart, config.maxRuntimeSec)) {
			for (int kickIdx = 0; kickIdx < config.kicksPerRound; kickIdx++) {
				if (runtimeExceeded(algoStart, config.maxRuntimeSec)) {
					break;
				}
				if (!applyEscapeKick(legalPairs, instances, nets, totalHPWL, config,
									 marks, token, affected, candidateNewHPWL, stats,
									 rng)) {
					break;
				}
				kicked = true;
			}
		}

		if (!kicked && improvingSwaps == 0) {
			break;
		}
	}

	// -------------------------------------------------------------------------
	// STEP 3: Restore globally best placement and return best-achieved stats.
	// -------------------------------------------------------------------------
	if (bestHPWL + 1e-6 < totalHPWL) {
		restorePlacementSnapshot(instances, bestSnapshot);
		for (int netId = 0; netId < (int)nets.size(); netId++) {
			nets[netId].hpwl = computeNetHPWL(nets[netId], instances);
		}
		totalHPWL = bestHPWL;
	}

	stats.swapsUsedForBest = swapsAtBest;
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

		// Build data used by both Problem 1 and Problem 2.
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
		timeval start;
		timeval end;
		gettimeofday(&start, NULL);

		OptimizerConfig config;
		config.maxRounds = 10;
		config.maxSteepestSwapsPerRound = 600;
		config.kicksPerRound = 2;
		config.kickCandidatePool = 16;
		config.maxRuntimeSec = 45.0;

		IncrementalStats stats =
			runIteratedLocalSearch(instances, nets, totalHPWL, config);

		gettimeofday(&end, NULL);
		double timeTakenSec = elapsedSeconds(start, end);
		double scoreMetric = totalHPWL * totalHPWL * timeTakenSec;

		cout << endl;
		cout << "Problem 1 -- Total wirelength of original design: "
			 << fixed << setprecision(0) << originalTotalHPWL << " DBU" << endl;
		cout << "Problem 2 -- Total wirelength AFTER my incremental placement "
				"algorithm:  "
			 << fixed << setprecision(0) << totalHPWL << " DBU" << endl;
		cout << "Problem 2 -- Total number of swaps used:  "
			 << stats.swapsUsedForBest << endl;
		cout << "Problem 2 -- Legal swap pairs available:  "
			 << stats.legalPairsAvailable << endl;
		cout << "Problem 2 -- Legal swap pairs evaluated:  "
			 << stats.legalPairsEvaluated << endl;
		cout << "Problem 2 -- Steepest-descent rounds:  "
			 << stats.steepestRoundsCompleted << endl;
		cout << "Problem 2 -- Local-minimum escape kicks:  "
			 << stats.localMinimaEscapes << endl;
		cout << "Problem 2 -- Time taken:  " << fixed << setprecision(6)
			 << timeTakenSec << " sec" << endl;
		cout << "Problem 2 -- Score metric (Total HPWL^2 * runtime):  "
			 << scientific << setprecision(6) << scoreMetric << endl;

		design->close();
		lib->close();

	} catch (oaCompatibilityError& ex) {
		handleFBCError(ex);
		return 1;

	} catch (oaException& excp) {
		cout << "ERROR: " << excp.getMsg() << endl;
		return 1;
	}

	return 0;
}
