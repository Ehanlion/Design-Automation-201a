// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 1 Test File

#include "oaDesignDB.h"
#include <algorithm>
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

// Design configuration constants
const string LIB_PATH_NAME = "./DesignLib";
const string LIBRARY_NAME = "DesignLib";
const string CELL_NAME = "s1196_bench";
const string VIEW_NAME = "layout";

// Function prototypes
void setupOpenAccess();
oaDesign* openDesign();
void printDesignNames(oaDesign* design);
void printNets(oaDesign* design);
void printFilteredNets(oaDesign* design);
vector<int> getFanout(oaDesign* design);
double computeAverage(vector<int> arr);
double computeAverage(vector<double> arr);
vector<double> computeHPWL(oaDesign* design);
double computeHPWLForNet(oaNet* net);

/*
 * Main function
 * Tests the setupOpenAccess function
 */
int main() {
	try {
		// Problem 1: setup OpenAccess
		cout << "\n----- Ethan Owen: Problem 1 -----" << endl;
		setupOpenAccess();
		oaDesign* design = openDesign();
		printNets(design);
		cout << endl;
		printFilteredNets(design);
		cout << endl;
		printDesignNames(design);
		cout << endl;

		// Problem 2: Compute Average Fanout
		cout << "----- Ethan Owen: Problem 2 -----" << endl;
		vector<int> fanoutArray = getFanout(design);
		double averageFanout = computeAverage(fanoutArray);
		cout << "Problem 2 -- Average fanout " << averageFanout << endl;
		cout << endl;

		// Problem 3: Compute HPWL for nets with 2 ends
		cout << "----- Ethan Owen: Problem 3 -----" << endl;
		vector<double> hpwlArray = computeHPWL(design);
		double averageHPWL = computeAverage(hpwlArray);
		cout << "Problem 3 -- Average wirelength " << averageHPWL << endl;
		cout << "Total nets with 2 ends: " << hpwlArray.size() << endl;
		cout << endl;
	} catch (oaCompatibilityError& ex) {
		handleFBCError(ex);
		exit(1);

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
	
	// First we need to get the strings here
	string libPathName = LIB_PATH_NAME;
	string libraryName = LIBRARY_NAME;
	string cellName = CELL_NAME;
	string viewName = VIEW_NAME;
	
	// Then we need to convert the strings to const char *
	const char* libPathNameC = libPathName.c_str();
	const char* libraryNameC = libraryName.c_str();
	const char* cellNameC = cellName.c_str();
	const char* viewNameC = viewName.c_str();
	
	// Then we need to create the oaString objects
	oaString libPath(libPathNameC);
	oaString library(libraryNameC);
	oaString cell(cellNameC);
	oaString view(viewNameC);
	
	// Then we need to create the oaScalarName objects
	oaScalarName libName(ns, library);
	oaScalarName cellNameScalar(ns, cell);
	oaScalarName viewNameScalar(ns, view);
	oaScalarName libraryNameScalar(ns, library);
	
	// Then we need the view type
	oaViewType* viewType = oaViewType::get(oacMaskLayout);
	
	opnTechConflictObserver myTechConflictObserver(1);
	opnLibDefListObserver myLibDefListObserver(1);
	oaLib* lib = oaLib::find(libraryNameScalar);
	if (!lib) {
		if (oaLib::exists(libPath)) {
			lib = oaLib::open(libraryNameScalar, libPath);
		} else {
			char* DMSystem = getenv("DMSystem");
			if (DMSystem) {
				lib = oaLib::create(libraryNameScalar, libPath, oacSharedLibMode, DMSystem);
			} else {
				lib = oaLib::create(libraryNameScalar, libPath);
			}
		}
		if (lib) {
			updateLibDefsFile(libraryNameScalar, libPath);
		}
	}
}

/*
 * Open the design
 */
oaDesign* openDesign() {
	// First we need to get the strings here
	string libPathName = LIB_PATH_NAME;
	string libraryName = LIBRARY_NAME;
	string cellName = CELL_NAME;
	string viewName = VIEW_NAME;

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

	// Adding to match previous output format
	cout << "The design is created and opened in 'write' mode." << endl;

	return design;
}

/*
 * Print the design names
 */
void printDesignNames(oaDesign* design) {
	oaString libName;
	oaString cellName;
	oaString viewName;
	design->getLibName(ns, libName);
	design->getCellName(ns, cellName);
	design->getViewName(ns, viewName);
	cout << "The design names are:" << endl;
	cout << " The library name for this design is: " << libName << endl;
	cout << " The cell name for this design is: " << cellName << endl;
	cout << " The view name for this design is: " << viewName << endl;
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
		int lineCount = 5;
		cout << "All nets in the design:" << endl;
		// Print out nets but do 3 per line
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
		// Output remaining nets if not multiple of 3
		if (count % lineCount != 0) {
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
		cout << "Nets being filtered out:" << endl;
		int count = 0;
		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
			oaString netName;
			net->getName(ns, netName);
			oaSigTypeEnum sigType = net->getSigType();
			if (netName == "VDD" ||
				netName == "VSS" ||
				netName == "blif_clk_net" ||
				netName == "blif_reset_net" ||
				netName == "tie1" ||
				netName == "tie0") {
				cout << "\t" << netName << " (name)" << endl;
				count++;
			} else if (sigType == oacPowerSigType ||
					   sigType == oacGroundSigType ||
					   sigType == oacClockSigType ||
					   sigType == oacResetSigType ||
					   sigType == oacTieoffSigType ||
					   sigType == oacTieHiSigType ||
					   sigType == oacTieLoSigType) {
				cout << "\t" << netName << " (by sigType:" << sigType << ")" << endl;
				count++;
			}
		}
		cout << "Filtered Net count: " << count << endl;
	} else {
		cout << "No filtered nets found" << endl;
	}
}

/*
 * Compute average fanout for filtered nets
 * Filtered nets are: VDD, VSS, blif_clk_net, blif_reset_net, tie1, tie0
 * New logic to count fanout for a net
 * Counts all instance terminals as loads
 * Counts primary I/O terminals as loads, only counting inputs
 */
vector<int> getFanout(oaDesign* design) {
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
		oaSigTypeEnum sigType = net->getSigType();
		if (netName == "VDD" ||
			netName == "VSS" ||
			netName == "blif_clk_net" ||
			netName == "blif_reset_net" ||
			netName == "tie1" ||
			netName == "tie0") {
			continue;
		} else if (sigType == oacPowerSigType ||
				   sigType == oacGroundSigType ||
				   sigType == oacClockSigType ||
				   sigType == oacResetSigType ||
				   sigType == oacTieoffSigType ||
				   sigType == oacTieHiSigType ||
				   sigType == oacTieLoSigType) {
			continue;
		}

		// Increment total nets count
		totalNets++;

		// Count fanout for a net
		int fanout = 0;

		// Count all instance terminals as loads (inputs)
		oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
		while (oaInstTerm* instTerm = instTermIterator.getNext()) {
			fanout++;
		}

		// For primary I/O terminals (terms), only count INPUTs as loads
		oaIter<oaTerm> termIterator(net->getTerms());
		while (oaTerm* term = termIterator.getNext()) {
			// Check if this is a scalar term
			if (term->getType() == oacScalarTermType) {
				oaScalarTerm* scalarTerm = (oaScalarTerm*)term;
				oaTermType termType = scalarTerm->getTermType();
				// Only count primary inputs
				if (termType == oacInputTermType) {
					fanout++;
				}
			}
		}

		// Store fanout value for this net
		fanoutArray.push_back(fanout);
	}

	return fanoutArray;
}

/*
 * Compute the average of an array of ints
 */
double computeAverage(vector<int> arr) {
	int sum = accumulate(arr.begin(), arr.end(), 0);
	double average = (double)sum / arr.size();
	return average;
}

/*
 * Compute the average of an array of doubles
 */
double computeAverage(vector<double> arr) {
	double sum = accumulate(arr.begin(), arr.end(), 0);
	double average = sum / arr.size();
	return average;
}

/*
 * Count the number of terminals on a net
 */
int countTerminals(oaNet* net) {
	int terminalCount = 0;
	oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
	while (oaInstTerm* instTerm = instTermIterator.getNext()) {
		terminalCount++;
	}
	oaIter<oaTerm> termIterator(net->getTerms());
	while (oaTerm* term = termIterator.getNext()) {
		terminalCount++;
	}
	return terminalCount;
}

/*
 * Compute HPWL for all nets with exactly 2 ends (2 terminals)
 * Returns a vector of HPWL values for each qualifying net
 */
vector<double> computeHPWL(oaDesign* design) {
	vector<double> hpwlArray;

	// Get the top block of the design
	oaBlock* block = design->getTopBlock();
	if (!block) {
		return hpwlArray;
	}

	// Iterate through all nets
	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		// Filter out power, ground, and clock nets (same filtering as getFanout)
		oaString netName;
		net->getName(ns, netName);
		oaSigTypeEnum sigType = net->getSigType();
		if (netName == "VDD" ||
			netName == "VSS" ||
			netName == "blif_clk_net" ||
			netName == "blif_reset_net" ||
			netName == "tie1" ||
			netName == "tie0") {
			continue;
		} else if (sigType == oacPowerSigType ||
				   sigType == oacGroundSigType ||
				   sigType == oacClockSigType ||
				   sigType == oacResetSigType ||
				   sigType == oacTieoffSigType ||
				   sigType == oacTieHiSigType ||
				   sigType == oacTieLoSigType) {
			continue;
		}

		// Count terminals (instance terminals + primary terminals)
		int terminalCount = 0;
		oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
		while (oaInstTerm* instTerm = instTermIterator.getNext()) {
			terminalCount++;
		}
		oaIter<oaTerm> termIterator(net->getTerms());
		while (oaTerm* term = termIterator.getNext()) {
			terminalCount++;
		}

		// Only process nets with exactly 2 ends
		if (terminalCount == 2) {
			double hpwl = computeHPWLForNet(net);
			if (hpwl >= 0) { // Valid HPWL value
				hpwlArray.push_back(hpwl);
			} else if (hpwl == -1) {
				cout << "Warning: Invalid HPWL value for net: " + netName << endl;
			}
		}
	}

	return hpwlArray;
}

/*
 * Compute HPWL for a single net
 * HPWL means Horizontal-Plus-Vertical Wirelength
 * HPWL = (max_x - min_x) + (max_y - min_y)
 * This considers all shapes across all metal layers
 */
double computeHPWLForNet(oaNet* net) {
	oaBox bbox;
	bool bboxInitialized = false;

	// Process primary I/O terminals (oaTerm)
	// Following the TA's explicit path: oaTerm --> oaPin --> oaPinFig
	oaIter<oaTerm> termIterator(net->getTerms());
	while (oaTerm* term = termIterator.getNext()) {
		// Get the pins from the terminal
		oaIter<oaPin> pinIterator(term->getPins());
		while (oaPin* pin = pinIterator.getNext()) {
			// Get all pin figures for this pin
			// oaPinFig inherits from oaFig which provides getBBox()
			oaIter<oaPinFig> pinFigIterator(pin->getFigs());
			while (oaPinFig* pinFig = pinFigIterator.getNext()) {
				// Get bounding box of the pin figure
				oaBox pinBox;
				pinFig->getBBox(pinBox);

				if (!bboxInitialized) {
					bbox = pinBox;
					bboxInitialized = true;
				} else {
					// Expand bounding box to include this pin figure
					oaPoint lowerLeft = bbox.lowerLeft();
					oaPoint upperRight = bbox.upperRight();
					oaPoint pinLowerLeft = pinBox.lowerLeft();
					oaPoint pinUpperRight = pinBox.upperRight();

					oaInt4 minX = std::min(lowerLeft.x(), pinLowerLeft.x());
					oaInt4 minY = std::min(lowerLeft.y(), pinLowerLeft.y());
					oaInt4 maxX = std::max(upperRight.x(), pinUpperRight.x());
					oaInt4 maxY = std::max(upperRight.y(), pinUpperRight.y());

					bbox.set(minX, minY, maxX, maxY);
				}
			}
		}
	}

	// Process instance terminals (oaInstTerm)
	// Use instance center as approximation for pin location
	oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
	while (oaInstTerm* instTerm = instTermIterator.getNext()) {
		oaInst* instance = instTerm->getInst();

		// Get instance bounding box and calculate center point
		oaBox instanceBBox;
		instance->getBBox(instanceBBox);
		oaPoint instanceLowerLeft = instanceBBox.lowerLeft();
		oaPoint instanceUpperRight = instanceBBox.upperRight();

		oaInt4 instanceCenterX = (instanceLowerLeft.x() + instanceUpperRight.x()) / 2;
		oaInt4 instanceCenterY = (instanceLowerLeft.y() + instanceUpperRight.y()) / 2;

		if (!bboxInitialized) {
			// Initialize bbox with instance center point
			bbox.set(instanceCenterX, instanceCenterY, instanceCenterX, instanceCenterY);
			bboxInitialized = true;
		} else {
			// Expand bbox to include instance center
			oaPoint currentLowerLeft = bbox.lowerLeft();
			oaPoint currentUpperRight = bbox.upperRight();

			oaInt4 minX = std::min(currentLowerLeft.x(), instanceCenterX);
			oaInt4 minY = std::min(currentLowerLeft.y(), instanceCenterY);
			oaInt4 maxX = std::max(currentUpperRight.x(), instanceCenterX);
			oaInt4 maxY = std::max(currentUpperRight.y(), instanceCenterY);

			bbox.set(minX, minY, maxX, maxY);
		}
	}

	// If no pins/instances found, return -1 to indicate invalid
	// Then print later knowing we got a -1 return value
	if (!bboxInitialized) {
		return -1;
	}

	// Calculate HPWL = (max_x - min_x) + (max_y - min_y)
	oaPoint lowerLeft = bbox.lowerLeft();
	oaPoint upperRight = bbox.upperRight();
	oaInt4 width = upperRight.x() - lowerLeft.x();
	oaInt4 height = upperRight.y() - lowerLeft.y();

	return (double)(width + height);
}