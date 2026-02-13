// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3
//
// Purpose:
// Evaluate all nets for legal swap candidates under the strict rule:
// 1) identical cell type/master
// 2) identical instance orientation
//
// This file only evaluates legal swap candidates and reports them.
// It does not move instances.

#include "oaDesignDB.h"
#include <algorithm>
#include <climits>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <sys/time.h>
#include <utility>
#include <vector>

#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace oa;
using namespace std;

static oaNativeNS ns;

struct InstInfo {
	string instName;
	string cellName;
	oaDesign* master;
	oaOrient orient;
};

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
	while (instTermIter.getNext()) {
		endpointCount++;
	}

	oaIter<oaTerm> termIter(net->getTerms());
	while (termIter.getNext()) {
		endpointCount++;
	}

	return endpointCount;
}

static double computeHPWLForNet(oaNet* net, int* endpointCountOut = nullptr) {
	int endpointCount = countNetEndpoints(net);
	if (endpointCountOut) {
		*endpointCountOut = endpointCount;
	}

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

	if (!bboxInitialized) {
		return 0.0;
	}
	oaPoint ll = bbox.lowerLeft();
	oaPoint ur = bbox.upperRight();
	return static_cast<double>((ur.x() - ll.x()) + (ur.y() - ll.y()));
}

static double computeTotalHPWL(oaBlock* block, int& netsCounted,
							   int& skippedNets) {
	double totalHPWL = 0.0;
	netsCounted = 0;
	skippedNets = 0;

	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		int endpointCount = 0;
		double hpwl = computeHPWLForNet(net, &endpointCount);
		totalHPWL += hpwl;
		netsCounted++;
		if (endpointCount == 0) {
			skippedNets++;
		}
	}

	return totalHPWL;
}

static inline bool sameCellAndOrient(const InstInfo& a, const InstInfo& b) {
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

static void printDesignNames(oaDesign* design) {
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
					lib = oaLib::create(libraryName, libPath, oacSharedLibMode,
										DMSystem);
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

		cout << "The design is created and opened in 'read' mode." << endl;
		oaDesign* design = oaDesign::open(libraryName, cellName, viewName,
										  viewType, 'r');

		printDesignNames(design);

		oaBlock* block = design->getTopBlock();
		if (!block) {
			cerr << "ERROR: There is no block in this design." << endl;
			design->close();
			lib->close();
			return 1;
		}

		cout << endl
			 << "----- Ethan Owen: Problem 1 -----" << endl;

		int netsCounted = 0;
		int skippedNets = 0;
		double totalHPWL = computeTotalHPWL(block, netsCounted, skippedNets);

		cout << "Problem 1 -- Nets counted in total HPWL: " << netsCounted
			 << " (skipped " << skippedNets << ")" << endl;
		cout << "Problem 1 -- Total HPWL (DBU): " << fixed << setprecision(0)
			 << totalHPWL << endl;

		cout << endl
			 << "----- Ethan Owen: Problem 2 -----" << endl;

		double originalHPWL = totalHPWL;
		cout << "Problem 2 -- Original HPWL (DBU): " << fixed
			 << setprecision(0) << originalHPWL << endl;

		struct timeval start;
		struct timeval end;
		gettimeofday(&start, NULL);

		int totalNets = 0;
		int netsWithLegalPairs = 0;
		int totalLegalPairs = 0;
		int maxPairsOnSingleNet = 0;
		string maxPairNetName;

		cout << "Problem 2 -- Evaluating legal swap candidates by net" << endl;
		cout << "Problem 2 -- Rule: same cell and same orientation only"
			 << endl;
		cout << "Problem 2 -- Note: this run only evaluates pairs and does not"
			 << " move instances." << endl;

		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
			totalNets++;

			vector<InstInfo> instances;
			oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
			while (oaInstTerm* instTerm = instTermIterator.getNext()) {
				oaInst* inst = instTerm->getInst();
				if (!inst) {
					continue;
				}

				InstInfo info;
				oaString instName;
				oaString cellNameStr;
				inst->getName(ns, instName);
				inst->getCellName(ns, cellNameStr);
				info.instName = string((const char*)instName);
				info.cellName = string((const char*)cellNameStr);
				info.master = inst->getMaster();
				info.orient = inst->getOrient();
				instances.push_back(info);
			}

			int legalPairCount = 0;
			map<int, int> orientationPairCounts;
			for (int i = 0; i < static_cast<int>(instances.size()); i++) {
				for (int j = i + 1; j < static_cast<int>(instances.size()); j++) {
					if (sameCellAndOrient(instances[i], instances[j])) {
						legalPairCount++;
						int orientVal =
							static_cast<int>((oaOrientEnum)instances[i].orient);
						orientationPairCounts[orientVal]++;
					}
				}
			}

			if (legalPairCount == 0) {
				continue;
			}

			netsWithLegalPairs++;
			totalLegalPairs += legalPairCount;

			oaString netName;
			net->getName(ns, netName);
			string netNameStd = string((const char*)netName);

			if (legalPairCount > maxPairsOnSingleNet) {
				maxPairsOnSingleNet = legalPairCount;
				maxPairNetName = netNameStd;
			}

			cout << "Net: " << netNameStd << "  legalPairs=" << legalPairCount
				 << "  orientations=";
			bool firstOrientation = true;
			for (const auto& entry : orientationPairCounts) {
				if (!firstOrientation) {
					cout << ",";
				}
				cout << entry.first << ":" << entry.second;
				firstOrientation = false;
			}
			cout << endl;
		}

		gettimeofday(&end, NULL);
		double timeTaken = (end.tv_sec - start.tv_sec) * 1e6;
		timeTaken = (timeTaken + (end.tv_usec - start.tv_usec)) * 1e-6;

		double finalHPWL = originalHPWL;
		double hpwlReduction = originalHPWL - finalHPWL;
		double score = (finalHPWL * finalHPWL) * timeTaken;
		int numSwapsUsed = 0;

		cout << "Problem 2 -- Reduced HPWL (DBU): " << fixed
			 << setprecision(0) << finalHPWL << endl;
		cout << "Problem 2 -- HPWL Reduction (DBU): " << fixed
			 << setprecision(0) << hpwlReduction << endl;
		cout << "Problem 2 -- Total nets scanned: " << totalNets << endl;
		cout << "Problem 2 -- Nets with legal swap pairs: "
			 << netsWithLegalPairs << endl;
		cout << "Problem 2 -- Total legal swap pairs found: "
			 << totalLegalPairs << endl;
		if (maxPairsOnSingleNet > 0) {
			cout << "Problem 2 -- Max legal pairs on one net: "
				 << maxPairsOnSingleNet << " (" << maxPairNetName << ")"
				 << endl;
		}
		cout << "Problem 2 -- Time taken: " << fixed << setprecision(6)
			 << timeTaken << " sec" << endl;
		cout << "Problem 2 -- Score (HPWL^2 * Time): " << scientific
			 << setprecision(4) << score << endl;

		cout << endl;
		cout << "Problem 1 -- Total wirelength of original design: "
			 << fixed << setprecision(0) << totalHPWL << endl;
		cout << "Problem 2 -- Total wirelength AFTER my incremental "
				"placement algorithm:  "
			 << fixed << setprecision(0) << finalHPWL << endl;
		cout << "Problem 2 -- Total number of swaps used:  "
			 << numSwapsUsed << endl;
		cout << "Problem 2 -- Time taken:  " << fixed << setprecision(6)
			 << timeTaken;
		cout << " sec" << endl;

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
