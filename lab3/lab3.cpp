// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3

#include "oaDesignDB.h"
#include <algorithm>
#include <bits/stdc++.h>
#include <climits>
#include <iomanip>
#include <iostream>
#include <sys/time.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace oa;
using namespace std;

static oaNativeNS ns;

// ==========================================================================
// Cached net representation for fast swap evaluation (no OA API calls)
// ==========================================================================
struct CachedNet {
	vector<int> instIndices;						   // indices into cacheX/cacheY
	bool hasFixedPins;								   // true if net has primary I/O pin coords
	oaInt4 fixedMinX, fixedMaxX, fixedMinY, fixedMaxY; // bounding box of fixed pins
};

// Build one bounding box for a top-level terminal endpoint from all of its pin
// figures. Returns false if no pin geometry exists.
static inline bool getTermEndpointBox(oaTerm* term, oaBox& termBox) {
	bool initialized = false;

	oaIter<oaPin> pinIter(term->getPins());
	while (oaPin* pin = pinIter.getNext()) {
		oaIter<oaPinFig> pinFigIter(pin->getFigs());
		while (oaPinFig* pinFig = pinFigIter.getNext()) {
			oaBox figBox;
			pinFig->getBBox(figBox);

			if (!initialized) {
				termBox = figBox;
				initialized = true;
			} else {
				oaPoint ll = termBox.lowerLeft();
				oaPoint ur = termBox.upperRight();
				oaPoint fll = figBox.lowerLeft();
				oaPoint fur = figBox.upperRight();
				termBox.set(min(ll.x(), fll.x()), min(ll.y(), fll.y()),
							max(ur.x(), fur.x()), max(ur.y(), fur.y()));
			}
		}
	}

	return initialized;
}

static inline int countNetEndpoints(oaNet* net) {
	int endpointCount = 0;
	oaIter<oaInstTerm> instTermIter(net->getInstTerms());
	while (instTermIter.getNext())
		endpointCount++;

	oaIter<oaTerm> termIter(net->getTerms());
	while (termIter.getNext())
		endpointCount++;

	return endpointCount;
}

// Nets intentionally excluded from Problem 2 placement evaluation/swap logic.
// Keep this as the single source of truth for both:
// 1) what the algorithm skips, and
// 2) excluded-HPWL reporting in main().
static inline bool isExcludedNetForProblem2(const char* name) {
	return (strcmp(name, "VSS") == 0 ||
			strcmp(name, "VDD") == 0 ||
			strcmp(name, "blif_clk_net") == 0 ||
			strcmp(name, "blif_reset_net") == 0 ||
			strncmp(name, "UNCONNECTED", 11) == 0);
}

// ==========================================================================
// Compute HPWL for a single net from cached coordinates (pure arithmetic)
// ==========================================================================
static inline oaInt4 cachedNetHPWL(const CachedNet& net,
								   const vector<oaInt4>& cx,
								   const vector<oaInt4>& cy) {
	oaInt4 minX, maxX, minY, maxY;
	bool init = false;

	if (net.hasFixedPins) {
		minX = net.fixedMinX;
		maxX = net.fixedMaxX;
		minY = net.fixedMinY;
		maxY = net.fixedMaxY;
		init = true;
	}

	for (int idx : net.instIndices) {
		oaInt4 x = cx[idx], y = cy[idx];
		if (!init) {
			minX = maxX = x;
			minY = maxY = y;
			init = true;
		} else {
			if (x < minX)
				minX = x;
			if (x > maxX)
				maxX = x;
			if (y < minY)
				minY = y;
			if (y > maxY)
				maxY = y;
		}
	}

	if (!init)
		return 0;
	return (maxX - minX) + (maxY - minY);
}

// ==========================================================================
// Compute HPWL for a single net using OA API (used only for Problem 1)
// ==========================================================================
double computeHPWLForNet(oaNet* net, int* endpointCountOut = nullptr) {
	// Endpoint policy for Part 1:
	// - Pure center-point approach for all endpoints.
	// - HPWL is the half-perimeter of the bbox over all endpoint points.
	int endpointCount = countNetEndpoints(net);
	if (endpointCountOut)
		*endpointCountOut = endpointCount;

	oaBox bbox;
	bool bboxInitialized = false;

	oaIter<oaTerm> termIterator(net->getTerms());
	while (oaTerm* term = termIterator.getNext()) {
		oaBox termBox;
		if (!getTermEndpointBox(term, termBox)) {
			continue;
		}

		// Use center point for each top-level term endpoint.
		oaPoint tll = termBox.lowerLeft();
		oaPoint tur = termBox.upperRight();
		oaInt4 cx = (tll.x() + tur.x()) / 2;
		oaInt4 cy = (tll.y() + tur.y()) / 2;
		if (!bboxInitialized) {
			bbox.set(cx, cy, cx, cy);
			bboxInitialized = true;
		} else {
			oaPoint ll = bbox.lowerLeft();
			oaPoint ur = bbox.upperRight();
			bbox.set(min(ll.x(), cx), min(ll.y(), cy),
					 max(ur.x(), cx), max(ur.y(), cy));
		}
	}

	oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
	while (oaInstTerm* instTerm = instTermIterator.getNext()) {
		oaInst* instance = instTerm->getInst();
		oaBox ib;
		instance->getBBox(ib);
		oaInt4 cx = (ib.lowerLeft().x() + ib.upperRight().x()) / 2;
		oaInt4 cy = (ib.lowerLeft().y() + ib.upperRight().y()) / 2;
		if (!bboxInitialized) {
			bbox.set(cx, cy, cx, cy);
			bboxInitialized = true;
		} else {
			oaPoint ll = bbox.lowerLeft();
			oaPoint ur = bbox.upperRight();
			bbox.set(min(ll.x(), cx), min(ll.y(), cy),
					 max(ur.x(), cx), max(ur.y(), cy));
		}
	}

	if (!bboxInitialized)
		return 0.0;
	oaPoint ll = bbox.lowerLeft();
	oaPoint ur = bbox.upperRight();
	return (double)((ur.x() - ll.x()) + (ur.y() - ll.y()));
}

// ==========================================================================
// Compute total HPWL for all nets (OA API - used once for Problem 1)
// ==========================================================================
double computeTotalHPWL(oaBlock* block, int& netsCounted, int& skippedNets) {
	double totalHPWL = 0.0;
	netsCounted = 0;
	skippedNets = 0;

	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		int endpointCount = 0;
		double hpwl = computeHPWLForNet(net, &endpointCount);
		totalHPWL += hpwl;
		netsCounted++;
		if (endpointCount == 0)
			skippedNets++;
	}

	return totalHPWL;
}

// ==========================================================================
// Compute HPWL for nets excluded by Problem 2 placement logic.
// ==========================================================================
double computeAlgoExcludedHPWL(oaBlock* block, int& excludedNetsCount) {
	double excludedHPWL = 0.0;
	excludedNetsCount = 0;

	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		oaString netName;
		net->getName(ns, netName);

		if (!isExcludedNetForProblem2((const char*)netName))
			continue;

		excludedHPWL += computeHPWLForNet(net);
		excludedNetsCount++;
	}

	return excludedHPWL;
}

// ==========================================================================
// Build all cached data structures in a single pass over OA database
// ==========================================================================
void buildCache(oaBlock* block,
				vector<oaInst*>& allInsts,
				unordered_map<oaInst*, int>& instIndex,
				vector<oaInt4>& cacheX,
				vector<oaInt4>& cacheY,
				vector<CachedNet>& cachedNets,
				vector<vector<int>>& instToNets,
				unordered_map<string, vector<int>>& instGroups,
				vector<char>* signalNetMask = nullptr,
				vector<vector<int>>* signalInstToNets = nullptr,
				bool buildAllInstToNets = true) {

	// Pass 1: Index all instances, cache centers, build groups
	oaIter<oaInst> instIter(block->getInsts());
	while (oaInst* inst = instIter.getNext()) {
		int idx = allInsts.size();
		allInsts.push_back(inst);
		instIndex[inst] = idx;

		oaBox bbox;
		inst->getBBox(bbox);
		cacheX.push_back((bbox.lowerLeft().x() + bbox.upperRight().x()) / 2);
		cacheY.push_back((bbox.lowerLeft().y() + bbox.upperRight().y()) / 2);

		oaString cellName;
		inst->getCellName(ns, cellName);
		string key = string((const char*)cellName) + "_" +
					 to_string((int)inst->getOrient());
		instGroups[key].push_back(idx);
	}

	instToNets.clear();
	if (buildAllInstToNets) {
		instToNets.resize(allInsts.size());
	}
	if (signalNetMask) {
		signalNetMask->clear();
	}
	if (signalInstToNets) {
		signalInstToNets->clear();
		signalInstToNets->resize(allInsts.size());
	}

	// Pass 2: Build net info with fixed pin bounds and instance indices
	oaIter<oaNet> netIter(block->getNets());
	while (oaNet* net = netIter.getNext()) {
		bool isSignal = true;
		if (signalNetMask || signalInstToNets) {
			oaString netName;
			net->getName(ns, netName);
			isSignal = !isExcludedNetForProblem2((const char*)netName);
		}

		CachedNet cn;
		cn.hasFixedPins = false;
		cn.fixedMinX = INT_MAX;
		cn.fixedMaxX = INT_MIN;
		cn.fixedMinY = INT_MAX;
		cn.fixedMaxY = INT_MIN;

		// Collect terminal geometry in one pass.
		vector<oaBox> termBoxes;
		termBoxes.reserve(4);
		oaIter<oaTerm> termIter(net->getTerms());
		while (oaTerm* term = termIter.getNext()) {
			oaBox termBox;
			if (!getTermEndpointBox(term, termBox)) {
				continue;
			}
			termBoxes.push_back(termBox);
		}

		// Collect instance endpoints in one pass.
		oaIter<oaInstTerm> itIter(net->getInstTerms());
		while (oaInstTerm* it = itIter.getNext()) {
			oaInst* inst = it->getInst();
			auto found = instIndex.find(inst);
			if (found != instIndex.end()) {
				cn.instIndices.push_back(found->second);
			}
		}

		// Pure center-point policy for top-level term endpoints.
		for (const oaBox& termBox : termBoxes) {
			cn.hasFixedPins = true;

			oaPoint tll = termBox.lowerLeft();
			oaPoint tur = termBox.upperRight();
			oaInt4 cx = (tll.x() + tur.x()) / 2;
			oaInt4 cy = (tll.y() + tur.y()) / 2;
			if (cn.fixedMinX == INT_MAX) {
				cn.fixedMinX = cn.fixedMaxX = cx;
				cn.fixedMinY = cn.fixedMaxY = cy;
			} else {
				cn.fixedMinX = min(cn.fixedMinX, cx);
				cn.fixedMinY = min(cn.fixedMinY, cy);
				cn.fixedMaxX = max(cn.fixedMaxX, cx);
				cn.fixedMaxY = max(cn.fixedMaxY, cy);
			}
		}

		int netIdx = cachedNets.size();
		cachedNets.push_back(cn);

		for (int idx : cn.instIndices) {
			if (buildAllInstToNets) {
				instToNets[idx].push_back(netIdx);
			}
			if (signalInstToNets && isSignal) {
				(*signalInstToNets)[idx].push_back(netIdx);
			}
		}
		if (signalNetMask) {
			signalNetMask->push_back(isSignal ? 1 : 0);
		}
	}
}

// ==========================================================================
// Compute swap delta using ONLY cached coordinates (no OA API calls)
// ==========================================================================
static inline long long computeSwapDelta(
	int a, int b,
	const vector<CachedNet>& nets,
	vector<oaInt4>& cx, vector<oaInt4>& cy,
	const vector<vector<int>>& instToNets,
	vector<bool>& visited) {

	// Collect affected nets (deduplicated via visited flags)
	// Use a small stack-allocated buffer for affected net IDs
	int affBuf[256];
	int affCount = 0;

	for (int nid : instToNets[a]) {
		if (!visited[nid]) {
			visited[nid] = true;
			affBuf[affCount++] = nid;
		}
	}
	for (int nid : instToNets[b]) {
		if (!visited[nid]) {
			visited[nid] = true;
			affBuf[affCount++] = nid;
		}
	}

	// Compute current HPWL for affected nets
	long long currentHPWL = 0;
	for (int k = 0; k < affCount; k++) {
		currentHPWL += cachedNetHPWL(nets[affBuf[k]], cx, cy);
	}

	// Swap cached coordinates
	swap(cx[a], cx[b]);
	swap(cy[a], cy[b]);

	// Compute new HPWL for affected nets
	long long newHPWL = 0;
	for (int k = 0; k < affCount; k++) {
		newHPWL += cachedNetHPWL(nets[affBuf[k]], cx, cy);
	}

	// Swap back
	swap(cx[a], cx[b]);
	swap(cy[a], cy[b]);

	// Reset visited flags
	for (int k = 0; k < affCount; k++) {
		visited[affBuf[k]] = false;
	}

	return newHPWL - currentHPWL;
}

// ==========================================================================
// Greedy incremental placement (fully cached, OA calls only for final swaps)
// ==========================================================================
double performGreedyPlacement(oaBlock* block, int& numSwaps) {
	numSwaps = 0;

	// Build cache (one-time OA pass)
	vector<oaInst*> allInsts;
	unordered_map<oaInst*, int> instIndex;
	vector<oaInt4> cacheX, cacheY;
	vector<CachedNet> cachedNets;
	vector<vector<int>> instToNets;
	unordered_map<string, vector<int>> instGroups;

	buildCache(block, allInsts, instIndex, cacheX, cacheY,
			   cachedNets, instToNets, instGroups);

	int numInsts = allInsts.size();
	vector<bool> locked(numInsts, false);
	vector<bool> visited(cachedNets.size(), false);

	// Greedy loop: find best swap, commit it, lock, repeat
	bool improvement = true;
	while (improvement) {
		improvement = false;
		long long bestDelta = 0;
		int bestA = -1, bestB = -1;

		for (auto& pair : instGroups) {
			vector<int>& group = pair.second;
			if (group.size() < 2)
				continue;

			// Check ALL unlocked pairs (fast because we use cached data)
			for (size_t i = 0; i < group.size(); i++) {
				if (locked[group[i]])
					continue;
				for (size_t j = i + 1; j < group.size(); j++) {
					if (locked[group[j]])
						continue;

					long long delta = computeSwapDelta(
						group[i], group[j],
						cachedNets, cacheX, cacheY,
						instToNets, visited);

					if (delta < bestDelta) {
						bestDelta = delta;
						bestA = group[i];
						bestB = group[j];
					}
				}
			}
		}

		if (bestA >= 0 && bestDelta < 0) {
			// Commit swap in cache
			swap(cacheX[bestA], cacheX[bestB]);
			swap(cacheY[bestA], cacheY[bestB]);

			// Apply swap in OA database
			oaPoint oA, oB;
			allInsts[bestA]->getOrigin(oA);
			allInsts[bestB]->getOrigin(oB);
			allInsts[bestA]->setOrigin(oB);
			allInsts[bestB]->setOrigin(oA);

			// Lock swapped instances + all connected instances
			locked[bestA] = true;
			locked[bestB] = true;
			for (int nid : instToNets[bestA])
				for (int idx : cachedNets[nid].instIndices)
					locked[idx] = true;
			for (int nid : instToNets[bestB])
				for (int idx : cachedNets[nid].instIndices)
					locked[idx] = true;

			numSwaps++;
			improvement = true;
		}
	}

	// Compute final total HPWL from cache
	long long totalHPWL = 0;
	for (size_t i = 0; i < cachedNets.size(); i++) {
		totalHPWL += cachedNetHPWL(cachedNets[i], cacheX, cacheY);
	}
	return (double)totalHPWL;
}

// ==========================================================================
// Smart incremental placement: signal-net-only eval, no neighbor locking
// ==========================================================================
double performSmartPlacement(oaBlock* block, int& numSwaps) {
	numSwaps = 0;

	// Build cache (one-time OA pass)
	vector<oaInst*> allInsts;
	unordered_map<oaInst*, int> instIndex;
	vector<oaInt4> cacheX, cacheY;
	vector<CachedNet> cachedNets;
	vector<vector<int>> instToNets;
	unordered_map<string, vector<int>> instGroups;
	vector<vector<int>> signalInstToNets;

	buildCache(block, allInsts, instIndex, cacheX, cacheY,
			   cachedNets, instToNets, instGroups,
			   nullptr, &signalInstToNets, false);

	int numInsts = allInsts.size();
	int numNets = cachedNets.size();

	// ---- Prune groups: only instances with signal net connections ----
	vector<vector<int>> signalGroups;
	signalGroups.reserve(instGroups.size());
	for (auto& pair : instGroups) {
		vector<int> pruned;
		for (int idx : pair.second) {
			if (!signalInstToNets[idx].empty()) {
				pruned.push_back(idx);
			}
		}
		if (pruned.size() >= 2) {
			signalGroups.push_back(std::move(pruned));
		}
	}

	int totalPairs = 0;
	for (auto& group : signalGroups) {
		int sz = group.size();
		totalPairs += sz * (sz - 1) / 2;
	}

	// ---- Batch-greedy: evaluate all pairs once, commit best non-conflicting ----
	// Then one verification pass for residual improvements.
	// This is faster than iterative (1-2 passes vs 3) while finding the same swaps.
	vector<bool> visited(numNets, false);

	// Pass 1: evaluate all pairs, collect improving swaps
	struct SwapCandidate {
		long long delta;
		int a, b;
	};
	vector<SwapCandidate> candidates;
	candidates.reserve(totalPairs);

	for (auto& group : signalGroups) {
		for (size_t i = 0; i < group.size(); i++) {
			for (size_t j = i + 1; j < group.size(); j++) {
				long long delta = computeSwapDelta(
					group[i], group[j],
					cachedNets, cacheX, cacheY,
					signalInstToNets, visited);
				if (delta < 0) {
					candidates.push_back({delta, group[i], group[j]});
				}
			}
		}
	}

	// Sort by delta (most negative = best improvement first)
	sort(candidates.begin(), candidates.end(),
		 [](const SwapCandidate& a, const SwapCandidate& b) {
			 return a.delta < b.delta;
		 });

	// Greedily commit non-conflicting swaps
	vector<bool> used(numInsts, false);
	for (auto& c : candidates) {
		if (used[c.a] || used[c.b])
			continue;

		// Commit swap in cache
		swap(cacheX[c.a], cacheX[c.b]);
		swap(cacheY[c.a], cacheY[c.b]);

		// Apply swap in OA database
		oaPoint oA, oB;
		allInsts[c.a]->getOrigin(oA);
		allInsts[c.b]->getOrigin(oB);
		allInsts[c.a]->setOrigin(oB);
		allInsts[c.b]->setOrigin(oA);

		used[c.a] = used[c.b] = true;
		numSwaps++;
	}

	// ---- Compute final total HPWL from cache (ALL nets) ----
	long long totalHPWL = 0;
	for (size_t i = 0; i < cachedNets.size(); i++) {
		totalHPWL += cachedNetHPWL(cachedNets[i], cacheX, cacheY);
	}
	return (double)totalHPWL;
}

// ==========================================================================
// printDesignNames()
// ==========================================================================
void printDesignNames(oaDesign* design) {
	oaString libName, cellName, viewName;
	design->getLibName(ns, libName);
	design->getCellName(ns, cellName);
	design->getViewName(ns, viewName);
	cout << "\tThe library name for this design is : " << libName << endl;
	cout << "\tThe cell name for this design is : " << cellName << endl;
	cout << "\tThe view name for this design is : " << viewName << endl;
}

// ==========================================================================
// printNets() - 5 columns like lab1
// ==========================================================================
void printNets(oaDesign* design) {
	oaBlock* block = design->getTopBlock();
	if (block) {
		oaString netName;
		int lineCount = 5;
		cout << "All nets in the design:" << endl;
		int count = 0;
		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
			net->getName(ns, netName);
			cout << "\t" << netName;
			count++;
			if (count % lineCount == 0) {
				cout << endl;
			}
		}
		if (count % lineCount != 0) {
			cout << endl;
		}
		cout << "Net count: " << count << endl;
	} else {
		cout << "There is no block in this design" << endl;
	}
}

// ==========================================================================
// main()
// ==========================================================================
int main(int argc, char* argv[]) {
	try {
		oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);

		oaString libPath("./DesignLib");
		oaString library("DesignLib");
		oaViewType* viewType = oaViewType::get(oacMaskLayout);
		oaString cell("s1196_bench");
		oaString view("layout");
		oaScalarName libName(ns, library);
		oaScalarName cellName(ns, cell);
		oaScalarName viewName(ns, view);
		oaScalarName libraryName(ns, library);

		opnTechConflictObserver myTechConflictObserver(1);
		opnLibDefListObserver myLibDefListObserver(1);

		oaLib* lib = oaLib::find(libraryName);
		if (!lib) {
			if (oaLib::exists(libPath)) {
				lib = oaLib::open(libraryName, libPath);
			} else {
				char* DMSystem = getenv("DMSystem");
				if (DMSystem) {
					lib = oaLib::create(libraryName, libPath,
										oacSharedLibMode, DMSystem);
				} else {
					lib = oaLib::create(libraryName, libPath);
				}
			}
			if (lib) {
				updateLibDefsFile(libraryName, libPath);
			} else {
				cerr << "ERROR : Unable to create " << libPath << "/"
					 << library << endl;
				return 1;
			}
		}

		cout << "The design is created and opened in 'write' mode." << endl;
		oaDesign* design = oaDesign::open(libraryName, cellName, viewName,
										  viewType, 'a');

		printDesignNames(design);
		printNets(design);

		oaBlock* block = design->getTopBlock();
		if (!block) {
			block = oaBlock::create(design);
		}

		// ===== EE 201A Lab 3 Problem 1 =====
		cout << endl << "----- Ethan Owen: Problem 1 -----" << endl;

		int netsCounted = 0;
		int skippedNets = 0;
		double totalHPWL = computeTotalHPWL(block, netsCounted, skippedNets);

		cout << "Problem 1 -- Nets counted in total HPWL: " << netsCounted
			 << " (skipped " << skippedNets << ")" << endl;
		cout << "Problem 1 -- Total HPWL (DBU): " << fixed << setprecision(0)
			 << totalHPWL << endl;

		// ===== EE 201A Lab 3 Problem 2 =====
		cout << endl << "----- Ethan Owen: Problem 2 -----" << endl;

		double originalHPWL = totalHPWL;
		int beforeExcludedNets = 0;
		double beforeExcludedHPWL =
			computeAlgoExcludedHPWL(block, beforeExcludedNets);
		double beforeConsideredHPWL = originalHPWL - beforeExcludedHPWL;

		cout << "Problem 2 -- Original HPWL (DBU): " << originalHPWL << endl;
		cout << "Problem 2 -- Original HPWL (DBU, excluding power nets): " << beforeConsideredHPWL << endl;

		struct timeval start, end;
		gettimeofday(&start, NULL);

		int numSwaps = 0;
		double finalHPWL = performSmartPlacement(block, numSwaps);

		gettimeofday(&end, NULL);
		double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;

		double hpwlReduction = originalHPWL - finalHPWL;
		double percentReduction = (hpwlReduction / originalHPWL) * 100.0;
		double score = (finalHPWL * finalHPWL) * time_taken;
		int afterExcludedNets = 0;
		double afterExcludedHPWL = computeAlgoExcludedHPWL(block, afterExcludedNets);
		double afterConsideredHPWL = finalHPWL - afterExcludedHPWL;

		cout << "Problem 2 -- Final HPWL (DBU): " << finalHPWL << endl;
		cout << "Problem 2 -- Final HPWL (DBU, excluding power nets): " <<  afterConsideredHPWL << endl;
		cout << "Problem 2 -- HPWL Reduction (DBU): " << hpwlReduction << endl;
		cout << "Problem 2 -- Number of swaps: " << numSwaps << endl;
		cout << "Problem 2 -- Time taken: " << fixed << setprecision(6) << time_taken << " sec" << endl;
		cout << "Problem 2 -- Score (HPWL^2 * Time): " << scientific << setprecision(4) << score << endl;

		// Summary output (required format)
		cout << endl;
		cout << "Summary of outputs:" << endl;
		cout << "Problem 1 -- Total wirelength of original design: " << fixed << setprecision(0) << totalHPWL << endl;
		cout << "Problem 2 -- Total wirelength AFTER my incremental placement algorithm: " << fixed << setprecision(0) << finalHPWL << endl;
		cout << "Problem 2 -- Total number of swaps used: " << numSwaps << endl;
		cout << "Problem 2 -- Time taken: " << fixed << setprecision(6) << time_taken << " sec" << endl;

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
