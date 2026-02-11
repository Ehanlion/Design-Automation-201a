// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3

// *****************************************************************************
// Lab 3: Incremental Placement Using OpenAccess
//
// Problem 1: Compute total HPWL for ALL nets in the design
// Problem 2: Incremental placement via cell swapping (placeholder)
// *****************************************************************************

#include "oaDesignDB.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sys/time.h>

#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace oa;
using namespace std;

static oaNativeNS ns;

// ****************************************************************************
// printDesignNames()
// ****************************************************************************
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

// ****************************************************************************
// printNets()
// ****************************************************************************
void printNets(oaDesign* design) {
	oaBlock* block = design->getTopBlock();

	if (block) {
		oaString netName;
		int count = 0;

		cout << "The following nets exist in this design." << endl;

		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
			net->getName(ns, netName);
			cout << "\t" << netName;
			count++;
			if (count % 5 == 0) cout << endl;
		}
		if (count % 5 != 0) cout << endl;
		cout << "Total net count: " << count << endl;
	} else {
		cout << "There is no block in this design" << endl;
	}
}

// ****************************************************************************
// computeHPWLForNet()
//
// Computes HPWL for a single net by finding the bounding box of all endpoints.
// For instance terminals: uses instance bounding box center as pin location.
// For primary I/O terminals: follows oaTerm -> oaPin -> oaPinFig path.
// Returns HPWL in DBU, or -1 if no valid endpoints found.
// ****************************************************************************
double computeHPWLForNet(oaNet* net) {
	oaInt4 minX = 0, minY = 0, maxX = 0, maxY = 0;
	bool initialized = false;

	// Helper lambda to expand bounding box with a point
	auto expandBBox = [&](oaInt4 x, oaInt4 y) {
		if (!initialized) {
			minX = maxX = x;
			minY = maxY = y;
			initialized = true;
		} else {
			if (x < minX) minX = x;
			if (x > maxX) maxX = x;
			if (y < minY) minY = y;
			if (y > maxY) maxY = y;
		}
	};

	// Process instance terminals (oaInstTerm)
	// Use instance bounding box center as pin location approximation
	oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
	while (oaInstTerm* instTerm = instTermIterator.getNext()) {
		oaInst* instance = instTerm->getInst();

		oaBox instanceBBox;
		instance->getBBox(instanceBBox);
		oaPoint ll = instanceBBox.lowerLeft();
		oaPoint ur = instanceBBox.upperRight();

		oaInt4 centerX = (ll.x() + ur.x()) / 2;
		oaInt4 centerY = (ll.y() + ur.y()) / 2;

		expandBBox(centerX, centerY);
	}

	// Process primary I/O terminals (oaTerm -> oaPin -> oaPinFig)
	oaIter<oaTerm> termIterator(net->getTerms());
	while (oaTerm* term = termIterator.getNext()) {
		oaIter<oaPin> pinIterator(term->getPins());
		while (oaPin* pin = pinIterator.getNext()) {
			oaIter<oaPinFig> pinFigIterator(pin->getFigs());
			while (oaPinFig* pinFig = pinFigIterator.getNext()) {
				oaBox pinBox;
				pinFig->getBBox(pinBox);

				// Use center of pin figure bounding box
				oaPoint ll = pinBox.lowerLeft();
				oaPoint ur = pinBox.upperRight();
				oaInt4 centerX = (ll.x() + ur.x()) / 2;
				oaInt4 centerY = (ll.y() + ur.y()) / 2;

				expandBBox(centerX, centerY);
			}
		}
	}

	if (!initialized) {
		return -1;
	}

	return (double)((maxX - minX) + (maxY - minY));
}

// ****************************************************************************
// computeTotalHPWL()
//
// Computes total HPWL for ALL nets in the design (including power, ground,
// clock, floating, etc.) as required by Lab 3 Problem 1.
// ****************************************************************************
double computeTotalHPWL(oaDesign* design) {
	oaBlock* block = design->getTopBlock();
	if (!block) {
		cout << "ERROR: No block found in the design." << endl;
		return 0;
	}

	double totalHPWL = 0.0;
	int netCount = 0;
	int validNetCount = 0;
	int skippedNets = 0;

	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		netCount++;
		double hpwl = computeHPWLForNet(net);
		if (hpwl >= 0) {
			totalHPWL += hpwl;
			validNetCount++;
		} else {
			// Net has no physical endpoints (floating net)
			skippedNets++;
		}
	}

	cout << "\tTotal nets processed: " << netCount << endl;
	cout << "\tNets with valid HPWL: " << validNetCount << endl;
	cout << "\tNets without endpoints (skipped): " << skippedNets << endl;
	cout << "\tTotal HPWL: " << fixed << setprecision(0) << totalHPWL << " DBU" << endl;

	return totalHPWL;
}

// ****************************************************************************
// main()
// ****************************************************************************
int main(int argc, char* argv[]) {

	try {
		// Initialize OA with data model 3
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

		// Setup observers
		opnTechConflictObserver myTechConflictObserver(1);
		opnLibDefListObserver myLibDefListObserver(1);

		// Open or create library
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
				cerr << "ERROR : Unable to create " << libPath << "/";
				cerr << library << endl;
				return (1);
			}
		}

		// Open the design in read mode
		cout << "The design is created and opened in 'read' mode." << endl;
		oaDesign* design =
			oaDesign::open(libraryName, cellName, viewName, viewType, 'r');

		// Print design info
		printDesignNames(design);
		printNets(design);

		// Get the TopBlock
		oaBlock* block = design->getTopBlock();
		if (!block) {
			block = oaBlock::create(design);
		}

		// =====================================================================
		// EE 201A Lab 3 Problem 1: Total HPWL for all nets
		// =====================================================================
		cout << endl
			 << "----- Ethan Owen: Problem 1 -----" << endl;

		double totalHPWL = computeTotalHPWL(design);

		// =====================================================================
		// EE 201A Lab 3 Problem 2: Incremental placement
		// =====================================================================
		cout << endl
			 << "----- Ethan Owen: Problem 2 -----" << endl;
		struct timeval start, end;
		gettimeofday(&start, NULL);

		// Problem 2 placeholder -- incremental placement not yet implemented
		int totalSwaps = 0;
		double improvedHPWL = totalHPWL;

		gettimeofday(&end, NULL);
		double time_taken;
		time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;

		// Output answers
		cout << endl;
		cout << "Problem 1 -- Total wirelength of original design: "
			 << fixed << setprecision(0) << totalHPWL << " DBU" << endl;
		cout << "Problem 2 -- Total wirelength AFTER my incremental placement "
				"algorithm:  "
			 << fixed << setprecision(0) << improvedHPWL << " DBU" << endl;
		cout << "Problem 2 -- Total number of swaps used:  " << totalSwaps
			 << endl;
		cout << "Problem 2 -- Time taken:  " << fixed << time_taken
			 << setprecision(6);
		cout << " sec" << endl;

		// Close design and library
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
