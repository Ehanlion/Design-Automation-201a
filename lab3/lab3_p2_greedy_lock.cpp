// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3
//
// Iteration 1 for Problem 2:
// Greedy best-swap algorithm with locking.
// Constraints enforced:
//   1) Swaps only between identical cells (same master design).
//   2) Swaps only between instances with identical orientation.
//   3) No movement operation other than direct location swap.

#include "oaDesignDB.h"
#include <algorithm>
#include <climits>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <sys/time.h>
#include <unordered_map>
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

IncrementalStats runGreedyIncrementalPlacement(vector<InstData>& instances,
											   vector<NetData>& nets,
											   double& totalHPWL) {
	IncrementalStats stats;
	stats.swapsApplied = 0;
	stats.legalPairsEvaluated = 0;

	if (instances.empty() || nets.empty()) {
		return stats;
	}

	vector<int> marks(nets.size(), 0);
	vector<int> affected;
	int token = 1;

	while (true) {
		double bestDelta = 0.0;
		int bestA = -1;
		int bestB = -1;
		vector<int> bestAffected;

		for (int i = 0; i < (int)instances.size(); i++) {
			if (!instances[i].movable || instances[i].locked) {
				continue;
			}
			for (int j = i + 1; j < (int)instances.size(); j++) {
				if (!instances[j].movable || instances[j].locked) {
					continue;
				}
				if (!isLegalSwapPair(instances[i], instances[j])) {
					continue;
				}

				stats.legalPairsEvaluated++;

				if (token == INT_MAX) {
					fill(marks.begin(), marks.end(), 0);
					token = 1;
				}
				collectAffectedNets(instances, i, j, marks, token, affected);
				token++;

				double oldAffectedHPWL = 0.0;
				double newAffectedHPWL = 0.0;

				for (int netId : affected) {
					oldAffectedHPWL += normalizeHPWL(nets[netId].hpwl);
					double newHPWL = computeNetHPWL(nets[netId], instances, i, j);
					newAffectedHPWL += normalizeHPWL(newHPWL);
				}

				double delta = oldAffectedHPWL - newAffectedHPWL;
				if (delta > bestDelta + 1e-6) {
					bestDelta = delta;
					bestA = i;
					bestB = j;
					bestAffected = affected;
				}
			}
		}

		if (bestA < 0 || bestB < 0 || bestDelta <= 1e-6) {
			break;
		}

		oaPoint originA = instances[bestA].origin;
		oaPoint originB = instances[bestB].origin;
		instances[bestA].inst->setOrigin(originB);
		instances[bestB].inst->setOrigin(originA);

		instances[bestA].origin = originB;
		instances[bestB].origin = originA;
		swap(instances[bestA].centerX, instances[bestB].centerX);
		swap(instances[bestA].centerY, instances[bestB].centerY);

		for (int netId : bestAffected) {
			double oldHPWL = normalizeHPWL(nets[netId].hpwl);
			double updatedHPWL = computeNetHPWL(nets[netId], instances);
			nets[netId].hpwl = updatedHPWL;
			double newHPWL = normalizeHPWL(updatedHPWL);
			totalHPWL += (newHPWL - oldHPWL);
		}

		instances[bestA].locked = true;
		instances[bestB].locked = true;
		for (int netId : bestAffected) {
			for (int instIdx : nets[netId].instIndices) {
				instances[instIdx].locked = true;
			}
		}

		stats.swapsApplied++;
		cout << "\tApplied swap #" << stats.swapsApplied
			 << " with HPWL improvement " << fixed << setprecision(0) << bestDelta
			 << " DBU" << endl;
	}

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
			runGreedyIncrementalPlacement(instances, nets, totalHPWL);

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
