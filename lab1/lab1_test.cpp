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
void printFilteredNets(oaDesign* design);
vector<int> computeAverageFanout(oaDesign* design);
vector<int> computeAverageFanoutFiltered(oaDesign* design);
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
		printFilteredNets(design);
		cout << endl;

		// Problem 2: Compute Average Fanout
		cout << "Problem 2: Compute Average Fanout" << endl;
		vector<int> fanoutArray = computeAverageFanout(design);
		vector<int> filteredFanoutArray = computeAverageFanoutFiltered(design);
		double averageFanout = computeAverage(fanoutArray);
		double averageFilteredFanout = computeAverage(filteredFanoutArray);
		cout << "The average fanout is " << averageFanout << endl;
		cout << "The average filtered fanout is " << averageFilteredFanout << endl;
		cout << endl;

		// Problem 3: ...
		// TODO: do Part 3 for test
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
	if (!block) {
		block = oaBlock::create(design);
	}
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
* Print out the nets that are being filtered out
* Filtered nets are: VDD, VSS, blif_clk_net, blif_reset_net, tie1, tie0
*/
void printFilteredNets(oaDesign* design) {
	oaBlock* block = design->getTopBlock();
	if (!block) {
		block = oaBlock::create(design);
	}
	if (block) {
		oaString netName;
		cout << "Nets to be filtered out:" << endl;
		int count = 0;
		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
			oaString netName;
			net->getName(ns, netName);
			if (netName == "VDD" ||
				netName == "VSS" ||
				netName == "blif_clk_net" ||
				netName == "blif_reset_net" ||
				netName == "tie1" ||
				netName == "tie0") {
				cout << "\t" << netName;
				count++;
			}
		}
		cout << endl;
		cout << "Filtered Net count: " << count << endl;
	} else {
		cout << "No filtered nets found" << endl;
	}
}

/*
 * Computes the average fanout of the design
 */
vector<int> computeAverageFanout(oaDesign* design) {
	vector<int> fanoutArray;

	// Get the top block of the design
	oaBlock* block = design->getTopBlock();
	if (!block) {
		block = oaBlock::create(design);
	}

	// Iterate through all nets
	int totalNets = 0;
	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		// Increment total nets count
		totalNets++;

		// Count instance terminals & primary terminals
		int fanout = 0;
		oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
		while (oaInstTerm* instTerm = instTermIterator.getNext()) {
			fanout++;
		}
		oaIter<oaTerm> termIterator(net->getTerms());
		while (oaTerm* term = termIterator.getNext()) {
			fanout++;
		}

		// Store fanout value for this net
		fanoutArray.push_back(fanout);
	}

	return fanoutArray;
}

/*
 * Compute average fanout for filtered nets
 * Filtered nets are: VDD, VSS, blif_clk_net, blif_reset_net, tie1, tie0
 */
vector<int> computeAverageFanoutFiltered(oaDesign* design) {
	vector<int> fanoutArray;

	// Get the top block of the design
	oaBlock* block = design->getTopBlock();
	if (!block) {
		block = oaBlock::create(design);
	}

	// Iterate through all nets
	int totalNets = 0;
	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		oaString netName;
		net->getName(ns, netName);
		if (netName == "VDD" ||
            netName == "VSS" ||
            netName == "blif_clk_net" ||
            netName == "blif_reset_net" ||
            netName == "tie1" ||
            netName == "tie0") {
			continue; // Skip special nets
		}

		// Increment total nets count
		totalNets++;

		// Count instance terminals & primary terminals
		int fanout = 0;
		oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
		while (oaInstTerm* instTerm = instTermIterator.getNext()) {
			fanout++;
		}
		oaIter<oaTerm> termIterator(net->getTerms());
		while (oaTerm* term = termIterator.getNext()) {
			fanout++;
		}

		// Store fanout value for this net
		fanoutArray.push_back(fanout);
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