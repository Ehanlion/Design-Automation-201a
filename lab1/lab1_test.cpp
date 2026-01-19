// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 1 Test File

#include "oaDesignDB.h"
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

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
void printDesignNames(oaDesign* design);
void printNets(oaDesign* design);
void printFilteredNets(oaDesign* design);
vector<int> getFanout(oaDesign* design);
double computeAverage(vector<int> arr);
void plotFanoutHistogram(vector<int> fanoutArray, const string& filename);
vector<double> computeHPWL(oaDesign* design);
double computeHPWLForNet(oaNet* net);
void plotHPWLHistogram(vector<double> hpwlArray, const string& filename);

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
		printFilteredNets(design);
		cout << endl;
		printDesignNames(design);
		cout << endl;

		// Problem 2: Compute Average Fanout
		cout << "Problem 2: Compute Average Fanout" << endl;
		vector<int> fanoutArray = getFanout(design);
		double averageFanout = computeAverage(fanoutArray);
		cout << "The average fanout is " << averageFanout << endl;
		cout << endl;
		
		// Generate histogram plot
		plotFanoutHistogram(fanoutArray, "plotting/fanout_histogram.html");
		cout << "Histogram saved to plotting/fanout_histogram.html" << endl;

		// Problem 3: Compute HPWL for nets with 2 ends
		cout << "Problem 3: Compute HPWL for nets with 2 ends" << endl;
		vector<double> hpwlArray = computeHPWL(design);
		if (!hpwlArray.empty()) {
			double sum = 0;
			for (size_t i = 0; i < hpwlArray.size(); i++) {
				sum += hpwlArray[i];
			}
			double averageHPWL = hpwlArray.size() > 0 ? sum / hpwlArray.size() : 0;
			cout << "The average HPWL is " << averageHPWL << endl;
			cout << "Total nets with 2 ends: " << hpwlArray.size() << endl;
			cout << endl;
			
			// Generate HPWL histogram plot
			plotHPWLHistogram(hpwlArray, "plotting/hpwl_histogram.html");
			cout << "HPWL histogram saved to plotting/hpwl_histogram.html" << endl;
		} else {
			cout << "No nets with 2 ends found" << endl;
		}
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
	int sum = accumulate(arr.begin(), arr.end(), 0);
	double average = (double)sum / arr.size();
	return average;
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
			if (hpwl >= 0) {  // Valid HPWL value
				hpwlArray.push_back(hpwl);
			}
		}
	}
	
	return hpwlArray;
}

/*
 * Compute HPWL for a single net
 * HPWL = (max_x - min_x) + (max_y - min_y)
 * This considers all shapes across all metal layers
 */
double computeHPWLForNet(oaNet* net) {
	oaBox bbox;
	bool bboxInitialized = false;
	
	// Iterate through all shapes on the net (across all metal layers)
	// This includes shapes on all routing layers
	oaIter<oaShape> shapeIterator(net->getShapes());
	while (oaShape* shape = shapeIterator.getNext()) {
		oaBox shapeBox;
		shape->getBBox(shapeBox);
		
		if (!bboxInitialized) {
			bbox = shapeBox;
			bboxInitialized = true;
		} else {
			// Expand bounding box to include this shape
			oaPoint lowerLeft = bbox.lowerLeft();
			oaPoint upperRight = bbox.upperRight();
			oaPoint shapeLowerLeft = shapeBox.lowerLeft();
			oaPoint shapeUpperRight = shapeBox.upperRight();
			
			oaInt4 minX = std::min(lowerLeft.x(), shapeLowerLeft.x());
			oaInt4 minY = std::min(lowerLeft.y(), shapeLowerLeft.y());
			oaInt4 maxX = std::max(upperRight.x(), shapeUpperRight.x());
			oaInt4 maxY = std::max(upperRight.y(), shapeUpperRight.y());
			
			bbox.set(minX, minY, maxX, maxY);
		}
	}
	
	// If no shapes found, return -1 to indicate invalid
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
// Plotting functions
// ========================================================================

/*
 * Generate a histogram plot of fanout distribution
 * Creates an HTML file with Chart.js visualization
 * Parameters:
 *   fanoutArray - Vector of fanout values for each net
 *   filename - Output HTML filename
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
		if (i > 0) htmlFile << ", ";
		htmlFile << i;
	}
	htmlFile << "],\n";
	
	// Generate data (counts)
	htmlFile << "                datasets: [{\n";
	htmlFile << "                    label: 'Number of Nets',\n";
	htmlFile << "                    data: [";
	for (int i = 0; i <= maxFanout; i++) {
		if (i > 0) htmlFile << ", ";
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
	
	// Also print summary to console
	cout << "Fanout Histogram Summary:" << endl;
	cout << "  Fanout Range: 0 to " << maxFanout << endl;
	cout << "  Total Nets: " << fanoutArray.size() << endl;
	for (map<int, int>::iterator it = histogram.begin(); it != histogram.end(); ++it) {
		if (it->second > 0) {
			cout << "  Fanout " << it->first << ": " << it->second << " nets" << endl;
		}
	}
}

/*
 * Generate a histogram plot of HPWL distribution
 * Creates an HTML file with Chart.js visualization
 * Parameters:
 *   hpwlArray - Vector of HPWL values for each net with 2 ends
 *   filename - Output HTML filename
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
		if (hpwlArray[i] < minHPWL) minHPWL = hpwlArray[i];
		if (hpwlArray[i] > maxHPWL) maxHPWL = hpwlArray[i];
	}
	
	// Create bins for histogram (use 20 bins)
	const int numBins = 20;
	double binWidth = (maxHPWL - minHPWL) / numBins;
	if (binWidth == 0) binWidth = 1;  // Avoid division by zero
	
	map<int, int> histogram;  // bin index -> count
	
	for (size_t i = 0; i < hpwlArray.size(); i++) {
		int binIndex = (int)((hpwlArray[i] - minHPWL) / binWidth);
		if (binIndex >= numBins) binIndex = numBins - 1;  // Put max value in last bin
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
		if (i > 0) htmlFile << ", ";
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
		if (i > 0) htmlFile << ", ";
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
	
	// Also print summary to console
	cout << "HPWL Histogram Summary:" << endl;
	cout << "  HPWL Range: " << minHPWL << " to " << maxHPWL << endl;
	cout << "  Total Nets (2 ends): " << hpwlArray.size() << endl;
	cout << "  Average HPWL: " << avg << endl;
}

