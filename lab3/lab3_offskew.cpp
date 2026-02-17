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
	// - >2 endpoints: HPWL of the smallest bbox including all endpoints.
	// - <=2 endpoints: center-point approach.
	int endpointCount = countNetEndpoints(net);
	if (endpointCountOut)
		*endpointCountOut = endpointCount;

	bool useFullBBox = (endpointCount > 2);

	oaBox bbox;
	bool bboxInitialized = false;

	oaIter<oaTerm> termIterator(net->getTerms());
	while (oaTerm* term = termIterator.getNext()) {
		oaBox termBox;
		if (!getTermEndpointBox(term, termBox)) {
			continue;
		}

		if (useFullBBox) {
			// For nets with >2 endpoints: use full endpoint bbox coverage.
			if (!bboxInitialized) {
				bbox = termBox;
				bboxInitialized = true;
			} else {
				oaPoint ll = bbox.lowerLeft();
				oaPoint ur = bbox.upperRight();
				oaPoint tll = termBox.lowerLeft();
				oaPoint tur = termBox.upperRight();
				bbox.set(min(ll.x(), tll.x()), min(ll.y(), tll.y()),
						 max(ur.x(), tur.x()), max(ur.y(), tur.y()));
			}
		} else {
			// For nets with <=2 endpoints: use center point.
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

		// Collect terminal geometry and endpoint count in one pass.
		int termCount = 0;
		vector<oaBox> termBoxes;
		termBoxes.reserve(4);
		oaIter<oaTerm> termIter(net->getTerms());
		while (oaTerm* term = termIter.getNext()) {
			termCount++;
			oaBox termBox;
			if (!getTermEndpointBox(term, termBox)) {
				continue;
			}
			termBoxes.push_back(termBox);
		}

		// Collect instance endpoints in one pass.
		int instTermCount = 0;
		oaIter<oaInstTerm> itIter(net->getInstTerms());
		while (oaInstTerm* it = itIter.getNext()) {
			instTermCount++;
			oaInst* inst = it->getInst();
			auto found = instIndex.find(inst);
			if (found != instIndex.end()) {
				cn.instIndices.push_back(found->second);
			}
		}

		bool useFullBBox = ((termCount + instTermCount) > 2);
		for (const oaBox& termBox : termBoxes) {
			cn.hasFixedPins = true;
			if (useFullBBox) {
				// For nets with >2 endpoints: use full bounding box.
				if (cn.fixedMinX == INT_MAX) {
					cn.fixedMinX = termBox.lowerLeft().x();
					cn.fixedMaxX = termBox.upperRight().x();
					cn.fixedMinY = termBox.lowerLeft().y();
					cn.fixedMaxY = termBox.upperRight().y();
				} else {
					cn.fixedMinX = min(cn.fixedMinX, termBox.lowerLeft().x());
					cn.fixedMinY = min(cn.fixedMinY, termBox.lowerLeft().y());
					cn.fixedMaxX = max(cn.fixedMaxX, termBox.upperRight().x());
					cn.fixedMaxY = max(cn.fixedMaxY, termBox.upperRight().y());
				}
			} else {
				// For nets with <=2 endpoints: use center point.
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
	vector<int>& seenNets, int& seenToken,
	vector<int>& affectedNets) {

	if (seenToken == INT_MAX) {
		fill(seenNets.begin(), seenNets.end(), 0);
		seenToken = 1;
	}
	int token = seenToken++;

	affectedNets.clear();
	affectedNets.reserve(instToNets[a].size() + instToNets[b].size());

	for (int nid : instToNets[a]) {
		if (seenNets[nid] != token) {
			seenNets[nid] = token;
			affectedNets.push_back(nid);
		}
	}
	for (int nid : instToNets[b]) {
		if (seenNets[nid] != token) {
			seenNets[nid] = token;
			affectedNets.push_back(nid);
		}
	}

	long long currentHPWL = 0;
	for (int nid : affectedNets) {
		currentHPWL += cachedNetHPWL(nets[nid], cx, cy);
	}

	swap(cx[a], cx[b]);
	swap(cy[a], cy[b]);

	long long newHPWL = 0;
	for (int nid : affectedNets) {
		newHPWL += cachedNetHPWL(nets[nid], cx, cy);
	}

	swap(cx[a], cx[b]);
	swap(cy[a], cy[b]);

	return newHPWL - currentHPWL;
}

struct NetEndpointSummary {
	long long instSumX = 0;
	long long instSumY = 0;
	int instCount = 0;
	bool hasFixed = false;
	long long fixedCenterX = 0;
	long long fixedCenterY = 0;
};

struct BucketPoint {
	int instIdx;
	oaInt4 x;
	oaInt4 y;
};

struct BucketKdNode {
	int pointIdx = -1;
	int left = -1;
	int right = -1;
	oaInt4 minX = 0;
	oaInt4 maxX = 0;
	oaInt4 minY = 0;
	oaInt4 maxY = 0;
	int axis = 0;
};

struct BucketKdTree {
	vector<BucketPoint> points;
	vector<BucketKdNode> nodes;
	int root = -1;
};

static int buildBucketKdRec(BucketKdTree& tree, vector<int>& order,
							int lo, int hi, int depth) {
	if (lo >= hi) {
		return -1;
	}

	int axis = depth & 1;
	int mid = lo + ((hi - lo) / 2);

	nth_element(order.begin() + lo, order.begin() + mid, order.begin() + hi,
				[&](int lhs, int rhs) {
					if (axis == 0) {
						if (tree.points[lhs].x == tree.points[rhs].x) {
							return tree.points[lhs].y < tree.points[rhs].y;
						}
						return tree.points[lhs].x < tree.points[rhs].x;
					}
					if (tree.points[lhs].y == tree.points[rhs].y) {
						return tree.points[lhs].x < tree.points[rhs].x;
					}
					return tree.points[lhs].y < tree.points[rhs].y;
				});

	int nodeIdx = tree.nodes.size();
	tree.nodes.push_back(BucketKdNode());

	int left = buildBucketKdRec(tree, order, lo, mid, depth + 1);
	int right = buildBucketKdRec(tree, order, mid + 1, hi, depth + 1);

	const BucketPoint& p = tree.points[order[mid]];
	BucketKdNode node;
	node.pointIdx = order[mid];
	node.left = left;
	node.right = right;
	node.axis = axis;
	node.minX = node.maxX = p.x;
	node.minY = node.maxY = p.y;

	if (left >= 0) {
		const BucketKdNode& ln = tree.nodes[left];
		node.minX = min(node.minX, ln.minX);
		node.maxX = max(node.maxX, ln.maxX);
		node.minY = min(node.minY, ln.minY);
		node.maxY = max(node.maxY, ln.maxY);
	}
	if (right >= 0) {
		const BucketKdNode& rn = tree.nodes[right];
		node.minX = min(node.minX, rn.minX);
		node.maxX = max(node.maxX, rn.maxX);
		node.minY = min(node.minY, rn.minY);
		node.maxY = max(node.maxY, rn.maxY);
	}

	tree.nodes[nodeIdx] = node;
	return nodeIdx;
}

static void buildBucketKdTree(BucketKdTree& tree) {
	tree.nodes.clear();
	tree.root = -1;
	if (tree.points.empty()) {
		return;
	}

	vector<int> order(tree.points.size());
	iota(order.begin(), order.end(), 0);
	tree.nodes.reserve(tree.points.size());
	tree.root = buildBucketKdRec(tree, order, 0, order.size(), 0);
}

static inline long long pointToNodeBoxDist2(const BucketKdNode& node,
											long long tx, long long ty) {
	long long dx = 0;
	if (tx < node.minX) {
		dx = (long long)node.minX - tx;
	} else if (tx > node.maxX) {
		dx = tx - (long long)node.maxX;
	}

	long long dy = 0;
	if (ty < node.minY) {
		dy = (long long)node.minY - ty;
	} else if (ty > node.maxY) {
		dy = ty - (long long)node.maxY;
	}

	return dx * dx + dy * dy;
}

static void nearestUnlockedInBucketRec(const BucketKdTree& tree, int nodeIdx,
									   long long tx, long long ty,
									   int excludeInst,
									   const vector<char>& locked,
									   int& bestInst, long long& bestDist2) {
	if (nodeIdx < 0) {
		return;
	}

	const BucketKdNode& node = tree.nodes[nodeIdx];
	if (pointToNodeBoxDist2(node, tx, ty) > bestDist2) {
		return;
	}

	const BucketPoint& p = tree.points[node.pointIdx];
	if (p.instIdx != excludeInst && !locked[p.instIdx]) {
		long long dx = (long long)p.x - tx;
		long long dy = (long long)p.y - ty;
		long long d2 = dx * dx + dy * dy;
		if (d2 < bestDist2 || (d2 == bestDist2 &&
							   (bestInst < 0 || p.instIdx < bestInst))) {
			bestDist2 = d2;
			bestInst = p.instIdx;
		}
	}

	int nearChild = node.left;
	int farChild = node.right;
	if (node.axis == 0) {
		if (tx > p.x) {
			nearChild = node.right;
			farChild = node.left;
		}
	} else if (ty > p.y) {
		nearChild = node.right;
		farChild = node.left;
	}

	if (nearChild >= 0) {
		nearestUnlockedInBucketRec(tree, nearChild, tx, ty, excludeInst,
								   locked, bestInst, bestDist2);
	}
	if (farChild >= 0 &&
		pointToNodeBoxDist2(tree.nodes[farChild], tx, ty) <= bestDist2) {
		nearestUnlockedInBucketRec(tree, farChild, tx, ty, excludeInst,
								   locked, bestInst, bestDist2);
	}
}

static int findNearestUnlockedPartner(const BucketKdTree& tree,
									  long long tx, long long ty,
									  int excludeInst,
									  const vector<char>& locked) {
	if (tree.root < 0) {
		return -1;
	}

	int bestInst = -1;
	long long bestDist2 = LLONG_MAX;
	nearestUnlockedInBucketRec(tree, tree.root, tx, ty, excludeInst,
							   locked, bestInst, bestDist2);
	return bestInst;
}

struct OffskewConfig {
	int topKPartners;
	int maxPasses;
	double timeBudgetSec;
	int minDelta;
	int maxRankedPerPass;
	int kicksPerPass;
	int maxKickDelta;
	int refineRounds;
	int maxPairsPerRound;
};

static inline int envInt(const char* name, int defaultValue,
						 int minValue, int maxValue) {
	const char* raw = getenv(name);
	if (!raw || !*raw) {
		return defaultValue;
	}
	char* end = nullptr;
	long parsed = strtol(raw, &end, 10);
	if (end == raw || *end != '\0') {
		return defaultValue;
	}
	if (parsed < minValue) {
		return minValue;
	}
	if (parsed > maxValue) {
		return maxValue;
	}
	return (int)parsed;
}

static inline double envDouble(const char* name, double defaultValue,
							   double minValue, double maxValue) {
	const char* raw = getenv(name);
	if (!raw || !*raw) {
		return defaultValue;
	}
	char* end = nullptr;
	double parsed = strtod(raw, &end);
	if (end == raw || *end != '\0') {
		return defaultValue;
	}
	if (parsed < minValue) {
		return minValue;
	}
	if (parsed > maxValue) {
		return maxValue;
	}
	return parsed;
}

static OffskewConfig loadOffskewConfig() {
	OffskewConfig cfg;
	cfg.topKPartners =
		envInt("OFFSKEW_TOPK", 6, 1, 256);
	cfg.maxPasses =
		envInt("OFFSKEW_MAX_PASSES", 4, 1, 1000);
	cfg.timeBudgetSec =
		envDouble("OFFSKEW_TIME_BUDGET_SEC", 0.10, 0.001, 300.0);
	cfg.minDelta =
		envInt("OFFSKEW_MIN_DELTA", 1, 1, INT_MAX);
	cfg.maxRankedPerPass =
		envInt("OFFSKEW_MAX_RANKED_PER_PASS", 120, 0, INT_MAX);
	cfg.kicksPerPass =
		envInt("OFFSKEW_KICKS_PER_PASS", 0, 0, 1000000);
	cfg.maxKickDelta =
		envInt("OFFSKEW_MAX_KICK_DELTA", 2500, 1, INT_MAX);
	cfg.refineRounds =
		envInt("OFFSKEW_REFINE_ROUNDS", 0, 0, 1000000);
	cfg.maxPairsPerRound =
		envInt("OFFSKEW_MAX_PAIRS_PER_ROUND", 200000, 1, INT_MAX);
	return cfg;
}

static inline void collectTopKBucketCandidates(
	int instA,
	const vector<int>& bucket,
	const vector<char>& passLocked,
	const vector<oaInt4>& cacheX,
	const vector<oaInt4>& cacheY,
	long long targetX, long long targetY,
	int topK,
	vector<int>& candidates) {
	priority_queue<pair<long long, int>> nearest;
	for (int instB : bucket) {
		if (instB == instA || passLocked[instB]) {
			continue;
		}
		long long dx = (long long)cacheX[instB] - targetX;
		long long dy = (long long)cacheY[instB] - targetY;
		long long dist2 = dx * dx + dy * dy;

		if ((int)nearest.size() < topK) {
			nearest.push({dist2, instB});
		} else if (dist2 < nearest.top().first ||
				   (dist2 == nearest.top().first &&
					instB < nearest.top().second)) {
			nearest.pop();
			nearest.push({dist2, instB});
		}
	}

	candidates.clear();
	candidates.reserve(nearest.size());
	while (!nearest.empty()) {
		candidates.push_back(nearest.top().second);
		nearest.pop();
	}
	reverse(candidates.begin(), candidates.end());
}

// ==========================================================================
// Offskew centroid-tension placer: multi-pass, top-k partner exploration
// ==========================================================================
double performCentroidTensionOffskewPlacement(oaBlock* block, int& numSwaps) {
	numSwaps = 0;
	OffskewConfig cfg = loadOffskewConfig();

	vector<oaInst*> allInsts;
	unordered_map<oaInst*, int> instIndex;
	vector<oaInt4> cacheX, cacheY;
	vector<CachedNet> cachedNets;
	vector<vector<int>> instToNets;
	unordered_map<string, vector<int>> instGroups;
	vector<char> signalNetMask;
	vector<vector<int>> signalInstToNets;

	buildCache(block, allInsts, instIndex, cacheX, cacheY,
			   cachedNets, instToNets, instGroups,
			   &signalNetMask, &signalInstToNets, false);

	int numInsts = allInsts.size();
	int numNets = cachedNets.size();

	vector<vector<int>> typeBuckets;
	typeBuckets.reserve(instGroups.size());
	vector<int> instToBucket(numInsts, -1);
	for (auto& bucketEntry : instGroups) {
		if (bucketEntry.second.size() < 2) {
			continue;
		}
		int bucketId = typeBuckets.size();
		typeBuckets.push_back(bucketEntry.second);
		for (int idx : bucketEntry.second) {
			instToBucket[idx] = bucketId;
		}
	}

	vector<NetEndpointSummary> netSummaries(numNets);
	for (int nid = 0; nid < numNets; nid++) {
		if (!signalNetMask[nid]) {
			continue;
		}
		const CachedNet& net = cachedNets[nid];
		NetEndpointSummary& summary = netSummaries[nid];
		summary.instCount = net.instIndices.size();
		if (net.hasFixedPins) {
			summary.hasFixed = true;
			summary.fixedCenterX =
				((long long)net.fixedMinX + (long long)net.fixedMaxX) / 2;
			summary.fixedCenterY =
				((long long)net.fixedMinY + (long long)net.fixedMaxY) / 2;
		}
	}

	vector<long long> centroidX(numInsts, 0);
	vector<long long> centroidY(numInsts, 0);
	vector<long long> tension2(numInsts, 0);
	vector<int> rankedInsts;
	rankedInsts.reserve(numInsts);

	vector<char> passLocked(numInsts, 0);
	vector<int> seenNets(numNets, 0);
	vector<int> affectedNets;
	int seenToken = 1;
	vector<int> candidates;

	struct timeval algoStart;
	gettimeofday(&algoStart, NULL);

	for (int pass = 0; pass < cfg.maxPasses; pass++) {
		struct timeval now;
		gettimeofday(&now, NULL);
		double elapsedSec =
			(now.tv_sec - algoStart.tv_sec) +
			(now.tv_usec - algoStart.tv_usec) * 1e-6;
		if (elapsedSec >= cfg.timeBudgetSec) {
			break;
		}

		for (int nid = 0; nid < numNets; nid++) {
			if (!signalNetMask[nid]) {
				continue;
			}
			NetEndpointSummary& summary = netSummaries[nid];
			summary.instSumX = 0;
			summary.instSumY = 0;
			for (int idx : cachedNets[nid].instIndices) {
				summary.instSumX += cacheX[idx];
				summary.instSumY += cacheY[idx];
			}
		}

		rankedInsts.clear();
		for (int idx = 0; idx < numInsts; idx++) {
			if (signalInstToNets[idx].empty()) {
				continue;
			}

			long double weightedSumX = 0.0;
			long double weightedSumY = 0.0;
			long long totalWeight = 0;

			for (int nid : signalInstToNets[idx]) {
				const NetEndpointSummary& summary = netSummaries[nid];
				int netWeight = summary.instCount - 1;
				long long netSumX = summary.instSumX - cacheX[idx];
				long long netSumY = summary.instSumY - cacheY[idx];

				if (summary.hasFixed) {
					netWeight++;
					netSumX += summary.fixedCenterX;
					netSumY += summary.fixedCenterY;
				}
				if (netWeight <= 0) {
					continue;
				}

				weightedSumX += netSumX;
				weightedSumY += netSumY;
				totalWeight += netWeight;
			}

			if (totalWeight <= 0) {
				continue;
			}

			long long tx = llround(weightedSumX / (long double)totalWeight);
			long long ty = llround(weightedSumY / (long double)totalWeight);
			centroidX[idx] = tx;
			centroidY[idx] = ty;

			long long dx = tx - (long long)cacheX[idx];
			long long dy = ty - (long long)cacheY[idx];
			tension2[idx] = dx * dx + dy * dy;
			rankedInsts.push_back(idx);
		}

		sort(rankedInsts.begin(), rankedInsts.end(),
			 [&](int a, int b) {
				 if (tension2[a] != tension2[b]) {
					 return tension2[a] > tension2[b];
				 }
				 return a < b;
			 });

		fill(passLocked.begin(), passLocked.end(), 0);
		int swapsThisPass = 0;
		int kicksThisPass = 0;
		int rankedLimit = cfg.maxRankedPerPass > 0 ?
			min((int)rankedInsts.size(), cfg.maxRankedPerPass) :
			(int)rankedInsts.size();

		for (int ri = 0; ri < rankedLimit; ri++) {
			int instA = rankedInsts[ri];
			if (passLocked[instA]) {
				continue;
			}

			gettimeofday(&now, NULL);
			elapsedSec =
				(now.tv_sec - algoStart.tv_sec) +
				(now.tv_usec - algoStart.tv_usec) * 1e-6;
			if (elapsedSec >= cfg.timeBudgetSec) {
				break;
			}

			int bucketId = instToBucket[instA];
			if (bucketId < 0) {
				continue;
			}
			const vector<int>& bucket = typeBuckets[bucketId];
			if (bucket.size() < 2) {
				continue;
			}

			collectTopKBucketCandidates(
				instA, bucket, passLocked, cacheX, cacheY,
				centroidX[instA], centroidY[instA],
				cfg.topKPartners, candidates);
			if (candidates.empty()) {
				continue;
			}

			long long bestDelta = 0;
			int bestB = -1;
			for (int instB : candidates) {
				long long delta = computeSwapDelta(
					instA, instB, cachedNets, cacheX, cacheY,
					signalInstToNets, seenNets, seenToken, affectedNets);
				if (delta < bestDelta) {
					bestDelta = delta;
					bestB = instB;
				}
			}

			bool takeSwap = false;
			if (bestB >= 0 && bestDelta <= -cfg.minDelta) {
				takeSwap = true;
			} else if (bestB >= 0 &&
					   cfg.kicksPerPass > kicksThisPass &&
					   bestDelta > 0 &&
					   bestDelta <= cfg.maxKickDelta) {
				takeSwap = true;
				kicksThisPass++;
			}
			if (!takeSwap) {
				continue;
			}

			swap(cacheX[instA], cacheX[bestB]);
			swap(cacheY[instA], cacheY[bestB]);

			oaPoint oA, oB;
			allInsts[instA]->getOrigin(oA);
			allInsts[bestB]->getOrigin(oB);
			allInsts[instA]->setOrigin(oB);
			allInsts[bestB]->setOrigin(oA);

			passLocked[instA] = 1;
			passLocked[bestB] = 1;
			numSwaps++;
			swapsThisPass++;
		}

		if (swapsThisPass == 0) {
			break;
		}
	}

	if (cfg.refineRounds > 0) {
		vector<pair<int, int>> legalPairs;
		size_t pairCount = 0;
		for (const vector<int>& bucket : typeBuckets) {
			if (bucket.size() < 2) {
				continue;
			}
			pairCount += (bucket.size() * (bucket.size() - 1ULL)) / 2ULL;
		}
		legalPairs.reserve(pairCount);
		for (const vector<int>& bucket : typeBuckets) {
			for (size_t i = 0; i < bucket.size(); i++) {
				for (size_t j = i + 1; j < bucket.size(); j++) {
					legalPairs.push_back({bucket[i], bucket[j]});
				}
			}
		}

		if (!legalPairs.empty()) {
			for (int round = 0; round < cfg.refineRounds; round++) {
				struct timeval now;
				gettimeofday(&now, NULL);
				double elapsedSec =
					(now.tv_sec - algoStart.tv_sec) +
					(now.tv_usec - algoStart.tv_usec) * 1e-6;
				if (elapsedSec >= cfg.timeBudgetSec) {
					break;
				}

				size_t evalPairs = min(legalPairs.size(),
									   (size_t)cfg.maxPairsPerRound);
				size_t start = (round * evalPairs) % legalPairs.size();

				long long bestDelta = 0;
				int bestA = -1;
				int bestB = -1;
				for (size_t k = 0; k < evalPairs; k++) {
					const pair<int, int>& p =
						legalPairs[(start + k) % legalPairs.size()];
					long long delta = computeSwapDelta(
						p.first, p.second, cachedNets, cacheX, cacheY,
						signalInstToNets, seenNets, seenToken, affectedNets);
					if (delta < bestDelta) {
						bestDelta = delta;
						bestA = p.first;
						bestB = p.second;
					}
				}

				if (bestA < 0) {
					break;
				}

				swap(cacheX[bestA], cacheX[bestB]);
				swap(cacheY[bestA], cacheY[bestB]);

				oaPoint oA, oB;
				allInsts[bestA]->getOrigin(oA);
				allInsts[bestB]->getOrigin(oB);
				allInsts[bestA]->setOrigin(oB);
				allInsts[bestB]->setOrigin(oA);
				numSwaps++;
			}
		}
	}

	long long totalHPWL = 0;
	for (const CachedNet& net : cachedNets) {
		totalHPWL += cachedNetHPWL(net, cacheX, cacheY);
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
		double finalHPWL = performCentroidTensionOffskewPlacement(block, numSwaps);

		gettimeofday(&end, NULL);
		double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;

		double hpwlReduction = originalHPWL - finalHPWL;
		int afterExcludedNets = 0;
		double afterExcludedHPWL = computeAlgoExcludedHPWL(block, afterExcludedNets);
		double afterConsideredHPWL = finalHPWL - afterExcludedHPWL;
		double score = (afterConsideredHPWL * afterConsideredHPWL) * time_taken;

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
