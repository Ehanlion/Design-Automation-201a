// plot_area_delay.cpp
// Lab 2 - Area vs Clock Period Plotting Utility
// Generates single-line and multi-line plots from synthesis data
// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 2

#include "../plotting/HTMLLinePlotter.h"
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

/**
 * Create directory if it doesn't exist
 */
bool createDirectoryIfNeeded(const string& dirPath) {
	struct stat info;
	if (stat(dirPath.c_str(), &info) != 0) {
		// Directory doesn't exist, create it
		if (mkdir(dirPath.c_str(), 0755) != 0) {
			cerr << "ERROR: Could not create directory " << dirPath << endl;
			return false;
		}
		cout << "Created directory: " << dirPath << endl;
	}
	return true;
}

/**
 * Generate single-line plot: Area vs Clock Period
 */
bool generateAreaVsDelayPlot(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Area vs Clock Period Plot" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Area vs Clock Period - Lab 2 Problem 3A";
	config.chartTitle = "Design Area vs Clock Period";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Total Area (μm²)";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Single dataset: Total Area (column 1)
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate multi-line plot: All metrics vs Clock Period
 */
bool generateMultiMetricsPlot(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Multi-Metrics Plot" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Synthesis Metrics vs Clock Period - Lab 2 Problem 3A";
	config.chartTitle = "Synthesis Metrics vs Clock Period";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Value";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Dataset 1: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Dataset 2: Instances (column 2) - Green
	DatasetConfig instancesDataset(
		2,
		"Instance Count",
		"rgba(75, 192, 192, 1)",   // Green line
		"rgba(75, 192, 192, 0.6)"  // Green points
	);
	config.datasets.push_back(instancesDataset);
	
	// Dataset 3: Slack (column 3) - Red
	DatasetConfig slackDataset(
		3,
		"Slack (ps)",
		"rgba(255, 99, 132, 1)",   // Red line
		"rgba(255, 99, 132, 0.6)"  // Red points
	);
	config.datasets.push_back(slackDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

int main() {
	cout << "\n";
	cout << "========================================" << endl;
	cout << "  Lab 2 - Plotting Utility" << endl;
	cout << "  Author: Ethan Owen (905452983)" << endl;
	cout << "========================================" << endl;
	
	// File paths
	string csvFile = "results/3A_plotting_data.csv";
	string outputDir = "plotting";
	string areaPlotFile = outputDir + "/area_vs_delay_plot.html";
	string metricsPlotFile = outputDir + "/synthesis_metrics_plot.html";
	
	// Create plotting directory if it doesn't exist
	if (!createDirectoryIfNeeded(outputDir)) {
		cerr << "ERROR: Failed to create plotting directory" << endl;
		return 1;
	}
	
	// Generate Plot 1: Single-line Area vs Clock Period
	bool success1 = generateAreaVsDelayPlot(csvFile, areaPlotFile);
	if (!success1) {
		cerr << "ERROR: Failed to generate area vs delay plot" << endl;
		return 1;
	}
	
	// Generate Plot 2: Multi-line Metrics Plot
	bool success2 = generateMultiMetricsPlot(csvFile, metricsPlotFile);
	if (!success2) {
		cerr << "ERROR: Failed to generate multi-metrics plot" << endl;
		return 1;
	}
	
	// Success message
	cout << "\n========================================" << endl;
	cout << "  Plot Generation Complete!" << endl;
	cout << "========================================" << endl;
	cout << "\nGenerated plots:" << endl;
	cout << "  1. " << areaPlotFile << endl;
	cout << "  2. " << metricsPlotFile << endl;
	cout << "\nOpen these files in a web browser to view the interactive plots." << endl;
	cout << "\n";
	
	return 0;
}
