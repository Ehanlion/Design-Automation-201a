// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 1 Test File

#include "oaDesignDB.h"
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// Includes from a class library - helper utilities for OpenAccess
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace std;
using namespace oa;
static oaNativeNS ns;

// Function prototypes
void setupOpenAccess();
oaDesign* openDesign();
void printNets(oaDesign* design);
vector<int> computeAverageFanout(oaDesign* design);
double computeAverage(vector<int> arr);

/*
 * Main function
 * Tests the setupOpenAccess function
 */
int main() {
	cout << "Testing for Lab1" << endl;
	try {
		// Problem 1: setup OpenAccess
        cout << "Problem 1: setup OpenAccess" << endl;
		setupOpenAccess();
		oaDesign* design = openDesign();
        printNets(design);
        cout << endl;

		// Problem 2: Compute Average Fanout
        cout << "Problem 2: Compute Average Fanout" << endl;
		vector<int> fanoutArray = computeAverageFanout(design);
		double averageFanout = computeAverage(fanoutArray);
		cout << "The average fanout is " << averageFanout << endl;
        cout << endl;
	} catch (oaException& excp) {
		cout << "ERROR: " << excp.getMsg() << endl;
		exit(1);
	}
	return 0;
}

/*
 * Sets up the OpenAccess environment
 */
void setupOpenAccess() {
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
				lib = oaLib::create(libraryName, libPath, oacSharedLibMode, DMSystem);
			} else {
				lib = oaLib::create(libraryName, libPath);
			}
		}
		if (lib) {
			updateLibDefsFile(libraryName, libPath);
		}
	}
}

/*
 * Open the design
 */
oaDesign* openDesign() {
	// First we need to get the strings here
	string libPathName = "./DesignLib";
	string libraryName = "DesignLib";
	string cellName = "s1196_bench";
	string viewName = "layout";

	// Then we need to convert the strings to const char *
	const char* libPathNameC = libPathName.c_str();
	const char* libraryNameC = libraryName.c_str();
	const char* cellNameC = cellName.c_str();
	const char* viewNameC = viewName.c_str();

	// Then we need to create the oaScalarName objects
	oaScalarName oaLib(ns, libraryNameC);
	oaScalarName oaCell(ns, cellNameC);
	oaScalarName oaView(ns, viewNameC);
	oaScalarName oaLibrary(ns, libraryNameC);

	// Then we need the view type
	oaViewType* viewType = oaViewType::get(oacMaskLayout);

	// Then we can open the design
	oaDesign* design = oaDesign::open(oaLibrary, oaCell, oaView, viewType, 'r');

	return design;
}

/*
 * Print all the nets and net count
 */
void printNets(oaDesign* design) {
	oaBlock* block = design->getTopBlock();
	if (block) {
		oaString netName;
		cout << "The following nets exist in this design." << endl;
		// Print out nets but do 3 per line
		int count = 0;
		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
			net->getName(ns, netName);
			cout << "\t" << netName;
			count++;
			if (count % 3 == 0) {
				cout << endl;
			}
		}
		// Output remaining nets if not multiple of 3
		if (count % 3 != 0) {
			cout << endl;
		}

        cout << "Net count: " << count << endl;
	} else {
		cout << "There is no block in this design" << endl;
	}
}

/*
 * Computes the average fanout of the design
 * Parameters:
 *   design - Pointer to the design to compute the average fanout of
 * Returns:
 *   Vector of ints containing the fanout values for each net
 */
vector<int> computeAverageFanout(oaDesign* design) {
	vector<int> fanoutArray;

	// Get the top block of the design
	oaBlock* block = design->getTopBlock();
	if (!block) {
		block = oaBlock::create(design);
	}

	return fanoutArray;
}

/*
 * Compute the average of an array of integers
 */
double computeAverage(vector<int> arr) {
	int sum = 0;
	double average = 0.0;

	// Loop through the array and sum the values
	for (int i = 0; i < arr.size(); i++) {
		sum += arr[i];
	}
	average = (double)sum / arr.size();
	return average;
}