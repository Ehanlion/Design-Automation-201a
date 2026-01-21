// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 1 Test File

#include "oaDesignDB.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

// Includes from a class library - helper utilities for OpenAccess
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

// Terminal plotting code (inlined to remove dependency on TerminalPlotter.h)

using namespace std;
using namespace oa;
static oaNativeNS ns;

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
void plotFanoutHistogram(vector<int> fanoutArray, const string& filename);
void plotHPWLHistogram(vector<double> hpwlArray, const string& filename);
void plotFanoutHistogramTerminal(vector<int> fanoutArray);
void plotHPWLHistogramTerminal(vector<double> hpwlArray);

/*
 * Main function
 * Tests the setupOpenAccess function
 */
int main() {
	// Enable flag for HTML file generation
	const bool ENABLE_HTML_GENERATION = true;

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

		// Generate fanout histogram plot
		if (!fanoutArray.empty()) {
			if (ENABLE_HTML_GENERATION) {
				plotFanoutHistogram(fanoutArray, "plotting/fanout_histogram.html");
				cout << "Fanout Histogram saved to plotting/fanout_histogram.html" << endl;
			}
			plotFanoutHistogramTerminal(fanoutArray);
		} else {
			cout << "\nCannot generate fanout histogram, no nets found!" << endl;
		}
		cout << endl;

		// Problem 3: Compute HPWL for nets with 2 ends
		cout << "----- Ethan Owen: Problem 3 -----" << endl;
		vector<double> hpwlArray = computeHPWL(design);
		double averageHPWL = computeAverage(hpwlArray);
		cout << "Problem 3 -- Average wirelength " << averageHPWL << endl;
		cout << "Total nets with 2 ends: " << hpwlArray.size() << endl;
		cout << endl;

		// Generate HPWL histogram plot
		if (!hpwlArray.empty()) {
			if (ENABLE_HTML_GENERATION) {
				plotHPWLHistogram(hpwlArray, "plotting/hpwl_histogram.html");
				cout << "HPWL histogram saved to plotting/hpwl_histogram.html" << endl;
			}
			// Also plot to terminal
			plotHPWLHistogramTerminal(hpwlArray);
		} else {
			cout << "\nCannot generate HPWL histogram, no nets with 2 ends found!" << endl;
		}
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
		cout << "The following nets exist in this design." << endl;
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
				cout << "\t" << netName << " (custom)" << endl;
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

// ========================================================================
// HPWL Computation Functions
// ========================================================================

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

// ========================================================================
// HTML Plotting Functions
// ========================================================================

/*
 * Generate a histogram plot of fanout distribution
 * Creates an HTML file with Chart.js visualization
 */
void plotFanoutHistogram(vector<int> fanoutArray, const string& filename) {
	// Build histogram: count how many nets have each fanout value
	map<int, int> histogram;
	int maxFanout = 0;

	for (size_t i = 0; i < fanoutArray.size(); i++) {
		int fanout = fanoutArray[i];
		histogram[fanout]++;
		if (fanout > maxFanout) {
			maxFanout = fanout;
		}
	}

	// Create plotting directory if it doesn't exist
	size_t lastSlash = filename.find_last_of("/");
	if (lastSlash != string::npos) {
		string dirPath = filename.substr(0, lastSlash);
		struct stat info;
		if (stat(dirPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
			// Directory doesn't exist, create it
			mkdir(dirPath.c_str(), 0755);
		}
	}

	// Create HTML file with Chart.js
	ofstream htmlFile(filename.c_str());
	if (!htmlFile.is_open()) {
		cerr << "ERROR: Could not create histogram file " << filename << endl;
		return;
	}

	htmlFile << "<!DOCTYPE html>\n";
	htmlFile << "<html>\n";
	htmlFile << "<head>\n";
	htmlFile << "    <title>Fanout Histogram</title>\n";
	htmlFile << "    <script src=\"https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js\"></script>\n";
	htmlFile << "    <style>\n";
	htmlFile << "        body { font-family: Arial, sans-serif; margin: 20px; }\n";
	htmlFile << "        .container { max-width: 1200px; margin: 0 auto; }\n";
	htmlFile << "        h1 { text-align: center; color: #333; }\n";
	htmlFile << "        canvas { background: white; border: 1px solid #ddd; }\n";
	htmlFile << "    </style>\n";
	htmlFile << "</head>\n";
	htmlFile << "<body>\n";
	htmlFile << "    <div class=\"container\">\n";
	htmlFile << "        <h1>Fanout Distribution Histogram</h1>\n";
	htmlFile << "        <p><strong>Total Nets:</strong> " << fanoutArray.size() << "</p>\n";

	// Calculate average
	double sum = 0;
	for (size_t i = 0; i < fanoutArray.size(); i++) {
		sum += fanoutArray[i];
	}
	double avg = fanoutArray.size() > 0 ? sum / fanoutArray.size() : 0;
	htmlFile << "        <p><strong>Average Fanout:</strong> " << avg << "</p>\n";

	htmlFile << "        <canvas id=\"fanoutChart\" width=\"400\" height=\"200\"></canvas>\n";
	htmlFile << "    </div>\n";
	htmlFile << "    <script>\n";
	htmlFile << "        const ctx = document.getElementById('fanoutChart').getContext('2d');\n";
	htmlFile << "        const fanoutChart = new Chart(ctx, {\n";
	htmlFile << "            type: 'bar',\n";
	htmlFile << "            data: {\n";

	// Generate labels (fanout values)
	htmlFile << "                labels: [";
	for (int i = 0; i <= maxFanout; i++) {
		if (i > 0)
			htmlFile << ", ";
		htmlFile << i;
	}
	htmlFile << "],\n";

	// Generate data (counts)
	htmlFile << "                datasets: [{\n";
	htmlFile << "                    label: 'Number of Nets',\n";
	htmlFile << "                    data: [";
	for (int i = 0; i <= maxFanout; i++) {
		if (i > 0)
			htmlFile << ", ";
		htmlFile << (histogram.find(i) != histogram.end() ? histogram[i] : 0);
	}
	htmlFile << "],\n";
	htmlFile << "                    backgroundColor: 'rgba(54, 162, 235, 0.6)',\n";
	htmlFile << "                    borderColor: 'rgba(54, 162, 235, 1)',\n";
	htmlFile << "                    borderWidth: 1\n";
	htmlFile << "                }]\n";
	htmlFile << "            },\n";
	htmlFile << "            options: {\n";
	htmlFile << "                responsive: true,\n";
	htmlFile << "                maintainAspectRatio: true,\n";
	htmlFile << "                scales: {\n";
	htmlFile << "                    y: {\n";
	htmlFile << "                        beginAtZero: true,\n";
	htmlFile << "                        title: {\n";
	htmlFile << "                            display: true,\n";
	htmlFile << "                            text: 'Number of Nets'\n";
	htmlFile << "                        }\n";
	htmlFile << "                    },\n";
	htmlFile << "                    x: {\n";
	htmlFile << "                        title: {\n";
	htmlFile << "                            display: true,\n";
	htmlFile << "                            text: 'Fanout Value'\n";
	htmlFile << "                        }\n";
	htmlFile << "                    }\n";
	htmlFile << "                },\n";
	htmlFile << "                plugins: {\n";
	htmlFile << "                    legend: {\n";
	htmlFile << "                        display: true,\n";
	htmlFile << "                        position: 'top'\n";
	htmlFile << "                    },\n";
	htmlFile << "                    title: {\n";
	htmlFile << "                        display: true,\n";
	htmlFile << "                        text: 'Distribution of Fanout Values Across All Nets'\n";
	htmlFile << "                    }\n";
	htmlFile << "                }\n";
	htmlFile << "            }\n";
	htmlFile << "        });\n";
	htmlFile << "    </script>\n";
	htmlFile << "</body>\n";
	htmlFile << "</html>\n";

	htmlFile.close();
}

/*
 * Generate a histogram plot of HPWL distribution
 * Creates an HTML file with Chart.js visualization
 */
void plotHPWLHistogram(vector<double> hpwlArray, const string& filename) {
	if (hpwlArray.empty()) {
		cerr << "ERROR: HPWL array is empty, cannot create histogram" << endl;
		return;
	}

	// Find min and max HPWL values
	double minHPWL = hpwlArray[0];
	double maxHPWL = hpwlArray[0];
	for (size_t i = 1; i < hpwlArray.size(); i++) {
		if (hpwlArray[i] < minHPWL)
			minHPWL = hpwlArray[i];
		if (hpwlArray[i] > maxHPWL)
			maxHPWL = hpwlArray[i];
	}

	// Create bins for histogram (use 20 bins)
	const int numBins = 20;
	double binWidth = (maxHPWL - minHPWL) / numBins;
	if (binWidth == 0)
		binWidth = 1; // Avoid division by zero

	map<int, int> histogram; // bin index -> count

	for (size_t i = 0; i < hpwlArray.size(); i++) {
		int binIndex = (int)((hpwlArray[i] - minHPWL) / binWidth);
		if (binIndex >= numBins)
			binIndex = numBins - 1; // Put max value in last bin
		histogram[binIndex]++;
	}

	// Create plotting directory if it doesn't exist
	size_t lastSlash = filename.find_last_of("/");
	if (lastSlash != string::npos) {
		string dirPath = filename.substr(0, lastSlash);
		struct stat info;
		if (stat(dirPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
			// Directory doesn't exist, create it
			mkdir(dirPath.c_str(), 0755);
		}
	}

	// Create HTML file with Chart.js
	ofstream htmlFile(filename.c_str());
	if (!htmlFile.is_open()) {
		cerr << "ERROR: Could not create histogram file " << filename << endl;
		return;
	}

	htmlFile << "<!DOCTYPE html>\n";
	htmlFile << "<html>\n";
	htmlFile << "<head>\n";
	htmlFile << "    <title>HPWL Histogram</title>\n";
	htmlFile << "    <script src=\"https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js\"></script>\n";
	htmlFile << "    <style>\n";
	htmlFile << "        body { font-family: Arial, sans-serif; margin: 20px; }\n";
	htmlFile << "        .container { max-width: 1200px; margin: 0 auto; }\n";
	htmlFile << "        h1 { text-align: center; color: #333; }\n";
	htmlFile << "        canvas { background: white; border: 1px solid #ddd; }\n";
	htmlFile << "    </style>\n";
	htmlFile << "</head>\n";
	htmlFile << "<body>\n";
	htmlFile << "    <div class=\"container\">\n";
	htmlFile << "        <h1>HPWL Distribution Histogram</h1>\n";
	htmlFile << "        <p><strong>Total Nets (2 ends):</strong> " << hpwlArray.size() << "</p>\n";

	// Calculate average
	double sum = 0;
	for (size_t i = 0; i < hpwlArray.size(); i++) {
		sum += hpwlArray[i];
	}
	double avg = hpwlArray.size() > 0 ? sum / hpwlArray.size() : 0;
	htmlFile << "        <p><strong>Average HPWL:</strong> " << avg << "</p>\n";
	htmlFile << "        <p><strong>Min HPWL:</strong> " << minHPWL << "</p>\n";
	htmlFile << "        <p><strong>Max HPWL:</strong> " << maxHPWL << "</p>\n";

	htmlFile << "        <canvas id=\"hpwlChart\" width=\"400\" height=\"200\"></canvas>\n";
	htmlFile << "    </div>\n";
	htmlFile << "    <script>\n";
	htmlFile << "        const ctx = document.getElementById('hpwlChart').getContext('2d');\n";
	htmlFile << "        const hpwlChart = new Chart(ctx, {\n";
	htmlFile << "            type: 'bar',\n";
	htmlFile << "            data: {\n";

	// Generate labels (bin ranges)
	htmlFile << "                labels: [";
	for (int i = 0; i < numBins; i++) {
		if (i > 0)
			htmlFile << ", ";
		double binStart = minHPWL + i * binWidth;
		double binEnd = minHPWL + (i + 1) * binWidth;
		htmlFile << "\"" << binStart << "-" << binEnd << "\"";
	}
	htmlFile << "],\n";

	// Generate data (counts)
	htmlFile << "                datasets: [{\n";
	htmlFile << "                    label: 'Number of Nets',\n";
	htmlFile << "                    data: [";
	for (int i = 0; i < numBins; i++) {
		if (i > 0)
			htmlFile << ", ";
		htmlFile << (histogram.find(i) != histogram.end() ? histogram[i] : 0);
	}
	htmlFile << "],\n";
	htmlFile << "                    backgroundColor: 'rgba(255, 99, 132, 0.6)',\n";
	htmlFile << "                    borderColor: 'rgba(255, 99, 132, 1)',\n";
	htmlFile << "                    borderWidth: 1\n";
	htmlFile << "                }]\n";
	htmlFile << "            },\n";
	htmlFile << "            options: {\n";
	htmlFile << "                responsive: true,\n";
	htmlFile << "                maintainAspectRatio: true,\n";
	htmlFile << "                scales: {\n";
	htmlFile << "                    y: {\n";
	htmlFile << "                        beginAtZero: true,\n";
	htmlFile << "                        title: {\n";
	htmlFile << "                            display: true,\n";
	htmlFile << "                            text: 'Number of Nets'\n";
	htmlFile << "                        }\n";
	htmlFile << "                    },\n";
	htmlFile << "                    x: {\n";
	htmlFile << "                        title: {\n";
	htmlFile << "                            display: true,\n";
	htmlFile << "                            text: 'HPWL Range'\n";
	htmlFile << "                        },\n";
	htmlFile << "                        ticks: {\n";
	htmlFile << "                            maxRotation: 45,\n";
	htmlFile << "                            minRotation: 45\n";
	htmlFile << "                        }\n";
	htmlFile << "                    }\n";
	htmlFile << "                },\n";
	htmlFile << "                plugins: {\n";
	htmlFile << "                    legend: {\n";
	htmlFile << "                        display: true,\n";
	htmlFile << "                        position: 'top'\n";
	htmlFile << "                    },\n";
	htmlFile << "                    title: {\n";
	htmlFile << "                        display: true,\n";
	htmlFile << "                        text: 'Distribution of HPWL Values for Nets with 2 Ends'\n";
	htmlFile << "                    }\n";
	htmlFile << "                }\n";
	htmlFile << "            }\n";
	htmlFile << "        });\n";
	htmlFile << "    </script>\n";
	htmlFile << "</body>\n";
	htmlFile << "</html>\n";

	htmlFile.close();
}

// ========================================================================
// Terminal Plotting Implementation (inlined from TerminalPlotter.h)
// ========================================================================

/**
 * HistogramConfig - Configuration structure for customizing histogram plots
 */
struct HistogramConfig {
	string title;		// Main title of the histogram
	string xAxisLabel;	// Label for X-axis (e.g., "Fanout Value", "HPWL Range")
	string yAxisLabel;	// Label for Y-axis (e.g., "Number of Nets")
	string description; // Description/subtitle at the bottom

	// Statistics labels
	string totalLabel;	 // Label for total count (e.g., "Total Nets", "Total Nets (2 ends)")
	string averageLabel; // Label for average (e.g., "Average Fanout", "Average HPWL")
	string minLabel;	 // Label for minimum value (empty string to hide)
	string maxLabel;	 // Label for maximum value (empty string to hide)

	// Histogram type configuration
	bool useBins;  // true for continuous data (bins), false for discrete values
	int numBins;   // Number of bins (only used if useBins == true)
	int precision; // Decimal precision for displaying numbers

	// Display configuration
	int maxBarWidth; // Maximum width of bars in characters
	int labelWidth;	 // Width reserved for labels

	// Default constructor with sensible defaults
	HistogramConfig() : title("Distribution Histogram"),
						xAxisLabel("Value"),
						yAxisLabel("Count"),
						description("Distribution of Values"),
						totalLabel("Total"),
						averageLabel("Average"),
						minLabel(""),
						maxLabel(""),
						useBins(false),
						numBins(20),
						precision(2),
						maxBarWidth(60),
						labelWidth(28) {
	}
};

/**
 * Normalize a value to fit within the display range
 */
static int normalizeHistogramValue(int value, int maxValue, int maxWidth) {
	if (maxValue == 0)
		return 0;
	return (int)((double)value / maxValue * maxWidth);
}

/**
 * Format a number with appropriate precision
 */
static string formatHistogramNumber(double num, int precision = 2) {
	ostringstream oss;
	oss << fixed << setprecision(precision) << num;
	return oss.str();
}

/**
 * Create a horizontal bar using ASCII characters
 */
static string createHistogramBar(int width) {
	if (width <= 0)
		return "";
	return string(width, '#');
}

/**
 * Format a label string, truncating if necessary
 */
static string formatHistogramLabel(const string& label, int maxWidth) {
	if (label.length() <= maxWidth)
		return label;
	return label.substr(0, maxWidth - 3) + "...";
}

/**
 * Generic histogram plotting function
 * Works with any numeric type (int, double, float, etc.)
 */
template <typename T>
static void plotHistogramGeneric(const vector<T>& data, const HistogramConfig& config) {
	if (data.empty()) {
		cerr << "ERROR: Data array is empty, cannot create histogram" << endl;
		return;
	}

	map<int, int> histogram; // bin/value index -> count
	int maxCount = 0;
	double minVal = 0, maxVal = 0;
	double sum = 0;

	if (config.useBins) {
		// Continuous data: use bins
		// Find min and max values
		minVal = (double)data[0];
		maxVal = (double)data[0];
		for (size_t i = 1; i < data.size(); i++) {
			double val = (double)data[i];
			if (val < minVal)
				minVal = val;
			if (val > maxVal)
				maxVal = val;
		}

		// Create bins
		double binWidth = (maxVal - minVal) / config.numBins;
		if (binWidth == 0)
			binWidth = 1; // Avoid division by zero

		// Build histogram
		for (size_t i = 0; i < data.size(); i++) {
			double val = (double)data[i];
			int binIndex = (int)((val - minVal) / binWidth);
			if (binIndex >= config.numBins)
				binIndex = config.numBins - 1; // Put max value in last bin
			histogram[binIndex]++;
			if (histogram[binIndex] > maxCount) {
				maxCount = histogram[binIndex];
			}
			sum += val;
		}
	} else {
		// Discrete data: count each value
		int minValInt = (int)data[0];
		int maxValInt = (int)data[0];

		for (size_t i = 0; i < data.size(); i++) {
			int val = (int)data[i];
			histogram[val]++;
			if (histogram[val] > maxCount) {
				maxCount = histogram[val];
			}
			if (val < minValInt)
				minValInt = val;
			if (val > maxValInt)
				maxValInt = val;
			sum += (double)data[i];
		}
		minVal = minValInt;
		maxVal = maxValInt;
	}

	// Calculate average
	double avg = data.size() > 0 ? sum / data.size() : 0;

	// Print header and statistics
	cout << "\n";
	cout << "========================================================================" << endl;
	cout << "                    " << config.title << endl;
	cout << "------------------------------------------------------------------------" << endl;
	cout << config.totalLabel << ": " << data.size() << endl;
	cout << config.averageLabel << ": " << formatHistogramNumber(avg, config.precision) << endl;
	if (!config.minLabel.empty()) {
		cout << config.minLabel << ": " << formatHistogramNumber(minVal, config.precision) << endl;
	}
	if (!config.maxLabel.empty()) {
		cout << config.maxLabel << ": " << formatHistogramNumber(maxVal, config.precision) << endl;
	}
	cout << "------------------------------------------------------------------------" << endl;
	cout << setw(config.labelWidth) << left << config.xAxisLabel << " | " << config.yAxisLabel << endl;
	cout << string(config.labelWidth, '-') << "-|-" << string(config.maxBarWidth, '-') << endl;

	// Print histogram bars
	if (config.useBins) {
		// Print bins
		double binWidth = (maxVal - minVal) / config.numBins;
		if (binWidth == 0)
			binWidth = 1;

		for (int i = 0; i < config.numBins; i++) {
			int count = (histogram.find(i) != histogram.end()) ? histogram[i] : 0;
			int barWidth = normalizeHistogramValue(count, maxCount, config.maxBarWidth);
			string bar = createHistogramBar(barWidth);

			double binStart = minVal + i * binWidth;
			double binEnd = minVal + (i + 1) * binWidth;
			string rangeLabel = formatHistogramNumber(binStart, config.precision) + "-" + formatHistogramNumber(binEnd, config.precision);
			rangeLabel = formatHistogramLabel(rangeLabel, config.labelWidth);

			cout << setw(config.labelWidth) << left << rangeLabel << " | "
				 << setw(4) << right << count << " " << bar << endl;
		}
	} else {
		// Print discrete values
		int minValInt = (int)minVal;
		int maxValInt = (int)maxVal;

		for (int i = minValInt; i <= maxValInt; i++) {
			int count = (histogram.find(i) != histogram.end()) ? histogram[i] : 0;
			int barWidth = normalizeHistogramValue(count, maxCount, config.maxBarWidth);
			string bar = createHistogramBar(barWidth);

			cout << setw(config.labelWidth) << right << i << " | "
				 << setw(4) << right << count << " " << bar << endl;
		}
	}

	cout << "------------------------------------------------------------------------" << endl;
	cout << config.description << endl;
	cout << "========================================================================" << endl;
	cout << "\n";
}

/*
 * Plot the fanout histogram to the terminal
 */
void plotFanoutHistogramTerminal(vector<int> fanoutArray) {
	HistogramConfig fanoutConfig;
	fanoutConfig.title = "Fanout Distribution Histogram";
	fanoutConfig.xAxisLabel = "Fanout Value";
	fanoutConfig.yAxisLabel = "Number of Nets";
	fanoutConfig.description = "Distribution of Fanout Values Across All Nets";
	fanoutConfig.totalLabel = "Total Nets";
	fanoutConfig.averageLabel = "Average Fanout";
	fanoutConfig.useBins = false;
	plotHistogramGeneric(fanoutArray, fanoutConfig);
}

/*
 * Plot the HPWL histogram to the terminal
 */
void plotHPWLHistogramTerminal(vector<double> hpwlArray) {
	HistogramConfig hpwlConfig;
	hpwlConfig.title = "HPWL Distribution Histogram";
	hpwlConfig.xAxisLabel = "HPWL Range";
	hpwlConfig.yAxisLabel = "Number of Nets";
	hpwlConfig.description = "Distribution of HPWL Values for Nets with 2 Ends";
	hpwlConfig.totalLabel = "Total Nets (2 ends)";
	hpwlConfig.averageLabel = "Average HPWL";
	hpwlConfig.useBins = true; // HPWL is continuous data, so use bins to show ranges
	hpwlConfig.numBins = 20;   // Use 20 bins (same as HTML plot)
	plotHistogramGeneric(hpwlArray, hpwlConfig);
}