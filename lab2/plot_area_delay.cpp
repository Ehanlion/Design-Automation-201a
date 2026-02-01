// plot_area_delay.cpp
// Lab 2 - Period vs Area/Instances/Slack Plotting Utility
// Generates plots: Period vs Area + Instances, Period vs Slack, Period vs Area and Slack
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
 * Generate plot: Period vs Area + Instances
 */
bool generateAreaInstancesVsPeriodPlot(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Period vs Area + Instances Plot" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Period vs Area + Instances - Lab 2 Problem 3A";
	config.chartTitle = "Period vs Area + Instances";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Value";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Dataset 1: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Dataset 2: Instances (column 2) - Green
	DatasetConfig instancesDataset(
		2,  // column index for "instances"
		"Instance Count",
		"rgba(75, 192, 192, 1)",   // Green line
		"rgba(75, 192, 192, 0.6)"  // Green points
	);
	config.datasets.push_back(instancesDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate plot: Period vs Area only
 */
bool generateAreaVsPeriodPlot(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Period vs Area Plot" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Period vs Area - Lab 2 Problem 3A";
	config.chartTitle = "Period vs Area";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Total Area (μm²)";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Single dataset: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate plot: Period vs Slack
 */
bool generateSlackVsPeriodPlot(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Period vs Slack Plot" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Period vs Slack - Lab 2 Problem 3A";
	config.chartTitle = "Period vs Slack";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Slack (ps)";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Single dataset: Slack (column 3) - Red
	DatasetConfig slackDataset(
		3,  // column index for "slack"
		"Slack (ps)",
		"rgba(255, 99, 132, 1)",   // Red line
		"rgba(255, 99, 132, 0.6)"  // Red points
	);
	config.datasets.push_back(slackDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate plot: Period vs Area and Slack
 */
bool generateAreaSlackVsPeriodPlot(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Period vs Area and Slack Plot" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Period vs Area and Slack - Lab 2 Problem 3A";
	config.chartTitle = "Period vs Area and Slack";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Value";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Dataset 1: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Dataset 2: Slack (column 3) - Red
	DatasetConfig slackDataset(
		3,  // column index for "slack"
		"Slack (ps)",
		"rgba(255, 99, 132, 1)",   // Red line
		"rgba(255, 99, 132, 0.6)"  // Red points
	);
	config.datasets.push_back(slackDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate filtered plot: Period vs Area only (with updated title)
 */
bool generateAreaVsPeriodPlotFiltered(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Period vs Area Plot (Filtered)" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Period vs Area (Filtered: slack >= 0 and < 200) - Lab 2 Problem 3A";
	config.chartTitle = "Period vs Area";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Total Area (μm²)";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Single dataset: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate synthesis metrics plot: Period vs Area + Instances + Slack
 */
bool generateSynthesisMetricsPlot(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Synthesis Metrics Plot" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Synthesis Metrics - Lab 2 Problem 3A";
	config.chartTitle = "Synthesis Metrics";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Value";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Dataset 1: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Dataset 2: Instances (column 2) - Green
	DatasetConfig instancesDataset(
		2,  // column index for "instances"
		"Instance Count",
		"rgba(75, 192, 192, 1)",   // Green line
		"rgba(75, 192, 192, 0.6)"  // Green points
	);
	config.datasets.push_back(instancesDataset);
	
	// Dataset 3: Slack (column 3) - Red
	DatasetConfig slackDataset(
		3,  // column index for "slack"
		"Slack (ps)",
		"rgba(255, 99, 132, 1)",   // Red line
		"rgba(255, 99, 132, 0.6)"  // Red points
	);
	config.datasets.push_back(slackDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate filtered synthesis metrics plot: Period vs Area + Instances + Slack
 */
bool generateSynthesisMetricsPlotFiltered(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Synthesis Metrics Plot (Filtered)" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Synthesis Metrics (Filtered: slack >= 0 and < 200) - Lab 2 Problem 3A";
	config.chartTitle = "Synthesis Metrics (Filtered: slack >= 0 and < 200)";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Value";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Dataset 1: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Dataset 2: Instances (column 2) - Green
	DatasetConfig instancesDataset(
		2,  // column index for "instances"
		"Instance Count",
		"rgba(75, 192, 192, 1)",   // Green line
		"rgba(75, 192, 192, 0.6)"  // Green points
	);
	config.datasets.push_back(instancesDataset);
	
	// Dataset 3: Slack (column 3) - Red
	DatasetConfig slackDataset(
		3,  // column index for "slack"
		"Slack (ps)",
		"rgba(255, 99, 132, 1)",   // Red line
		"rgba(255, 99, 132, 0.6)"  // Red points
	);
	config.datasets.push_back(slackDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate filtered plot: Period vs Area + Instances (with updated title)
 */
bool generateAreaInstancesVsPeriodPlotFiltered(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Period vs Area + Instances Plot (Filtered)" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Period vs Area + Instances (Filtered: slack >= 0 and < 200) - Lab 2 Problem 3A";
	config.chartTitle = "Period vs Area + Instances (Filtered: slack >= 0 and < 200)";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Value";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Dataset 1: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Dataset 2: Instances (column 2) - Green
	DatasetConfig instancesDataset(
		2,  // column index for "instances"
		"Instance Count",
		"rgba(75, 192, 192, 1)",   // Green line
		"rgba(75, 192, 192, 0.6)"  // Green points
	);
	config.datasets.push_back(instancesDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate filtered plot: Period vs Slack (with updated title)
 */
bool generateSlackVsPeriodPlotFiltered(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Period vs Slack Plot (Filtered)" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Period vs Slack (Filtered: slack >= 0 and < 200) - Lab 2 Problem 3A";
	config.chartTitle = "Period vs Slack (Filtered: slack >= 0 and < 200)";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Slack (ps)";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Single dataset: Slack (column 3) - Red
	DatasetConfig slackDataset(
		3,  // column index for "slack"
		"Slack (ps)",
		"rgba(255, 99, 132, 1)",   // Red line
		"rgba(255, 99, 132, 0.6)"  // Red points
	);
	config.datasets.push_back(slackDataset);
	
	// Create plotter and generate HTML
	HTMLLinePlotter plotter;
	return plotter.plotFromCSV(csvFile, outputFile, config);
}

/**
 * Generate filtered plot: Period vs Area and Slack (with updated title)
 */
bool generateAreaSlackVsPeriodPlotFiltered(const string& csvFile, const string& outputFile) {
	cout << "\n========================================" << endl;
	cout << "  Generating Period vs Area and Slack Plot (Filtered)" << endl;
	cout << "========================================" << endl;
	
	// Configure plot
	CSVLinePlotConfig config;
	config.pageTitle = "Period vs Area and Slack (Filtered: slack >= 0 and < 200) - Lab 2 Problem 3A";
	config.chartTitle = "Period vs Area and Slack (Filtered: slack >= 0 and < 200)";
	config.xAxisLabel = "Clock Period (ps)";
	config.yAxisLabel = "Value";
	config.xColumnIndex = 0;  // period column
	config.skipHeader = true;
	
	// Dataset 1: Total Area (column 1) - Blue
	DatasetConfig areaDataset(
		1,  // column index for "total area"
		"Total Area (μm²)",
		"rgba(54, 162, 235, 1)",   // Blue line
		"rgba(54, 162, 235, 0.6)"  // Blue points
	);
	config.datasets.push_back(areaDataset);
	
	// Dataset 2: Slack (column 3) - Red
	DatasetConfig slackDataset(
		3,  // column index for "slack"
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
	string csvFileFiltered = "results/3A_plotting_data_filtered.csv";
	string outputDir = "plotting";
	string areaInstancesPlotFile = outputDir + "/period_vs_area_instances_plot.html";
	string slackPlotFile = outputDir + "/period_vs_slack_plot.html";
	string areaSlackPlotFile = outputDir + "/period_vs_area_slack_plot.html";
	string areaPlotFile = outputDir + "/period_vs_area_plot.html";
	string synthesisMetricsPlotFile = outputDir + "/synthesis_metrics_plot.html";
	string areaInstancesPlotFileFiltered = outputDir + "/period_vs_area_instances_plot_filtered.html";
	string slackPlotFileFiltered = outputDir + "/period_vs_slack_plot_filtered.html";
	string areaSlackPlotFileFiltered = outputDir + "/period_vs_area_slack_plot_filtered.html";
	string areaPlotFileFiltered = outputDir + "/period_vs_area_plot_filtered.html";
	string synthesisMetricsPlotFileFiltered = outputDir + "/synthesis_metrics_plot_filtered.html";
	
	// Create plotting directory if it doesn't exist
	if (!createDirectoryIfNeeded(outputDir)) {
		cerr << "ERROR: Failed to create plotting directory" << endl;
		return 1;
	}
	
	// Generate Plot 1: Period vs Area + Instances (original)
	bool success1 = generateAreaInstancesVsPeriodPlot(csvFile, areaInstancesPlotFile);
	if (!success1) {
		cerr << "ERROR: Failed to generate period vs area + instances plot" << endl;
		return 1;
	}
	
	// Generate Plot 2: Period vs Slack (original)
	bool success2 = generateSlackVsPeriodPlot(csvFile, slackPlotFile);
	if (!success2) {
		cerr << "ERROR: Failed to generate period vs slack plot" << endl;
		return 1;
	}
	
	// Generate Plot 3: Period vs Area and Slack (original)
	bool success3 = generateAreaSlackVsPeriodPlot(csvFile, areaSlackPlotFile);
	if (!success3) {
		cerr << "ERROR: Failed to generate period vs area and slack plot" << endl;
		return 1;
	}
	
	// Generate Plot 4: Period vs Area only (original)
	bool success4 = generateAreaVsPeriodPlot(csvFile, areaPlotFile);
	if (!success4) {
		cerr << "ERROR: Failed to generate period vs area plot" << endl;
		return 1;
	}
	
	// Generate Plot 5: Synthesis Metrics (original)
	bool success5 = generateSynthesisMetricsPlot(csvFile, synthesisMetricsPlotFile);
	if (!success5) {
		cerr << "ERROR: Failed to generate synthesis metrics plot" << endl;
		return 1;
	}
	
	// Generate Plot 1 Filtered: Period vs Area + Instances (filtered: slack >= 0 and < 200)
	bool success1f = generateAreaInstancesVsPeriodPlotFiltered(csvFileFiltered, areaInstancesPlotFileFiltered);
	if (!success1f) {
		cerr << "ERROR: Failed to generate filtered period vs area + instances plot" << endl;
		return 1;
	}
	
	// Generate Plot 2 Filtered: Period vs Slack (filtered: slack >= 0 and < 200)
	bool success2f = generateSlackVsPeriodPlotFiltered(csvFileFiltered, slackPlotFileFiltered);
	if (!success2f) {
		cerr << "ERROR: Failed to generate filtered period vs slack plot" << endl;
		return 1;
	}
	
	// Generate Plot 3 Filtered: Period vs Area and Slack (filtered: slack >= 0 and < 200)
	bool success3f = generateAreaSlackVsPeriodPlotFiltered(csvFileFiltered, areaSlackPlotFileFiltered);
	if (!success3f) {
		cerr << "ERROR: Failed to generate filtered period vs area and slack plot" << endl;
		return 1;
	}
	
	// Generate Plot 4 Filtered: Period vs Area only (filtered: slack >= 0 and < 200)
	bool success4f = generateAreaVsPeriodPlotFiltered(csvFileFiltered, areaPlotFileFiltered);
	if (!success4f) {
		cerr << "ERROR: Failed to generate filtered period vs area plot" << endl;
		return 1;
	}
	
	// Generate Plot 5 Filtered: Synthesis Metrics (filtered: slack >= 0 and < 200)
	bool success5f = generateSynthesisMetricsPlotFiltered(csvFileFiltered, synthesisMetricsPlotFileFiltered);
	if (!success5f) {
		cerr << "ERROR: Failed to generate filtered synthesis metrics plot" << endl;
		return 1;
	}
	
	// Success message
	cout << "\n========================================" << endl;
	cout << "  Plot Generation Complete!" << endl;
	cout << "========================================" << endl;
	cout << "\nGenerated plots (original):" << endl;
	cout << "  1. " << areaInstancesPlotFile << endl;
	cout << "  2. " << slackPlotFile << endl;
	cout << "  3. " << areaSlackPlotFile << endl;
	cout << "  4. " << areaPlotFile << endl;
	cout << "  5. " << synthesisMetricsPlotFile << endl;
	cout << "\nGenerated plots (filtered: slack >= 0 and < 200):" << endl;
	cout << "  6. " << areaInstancesPlotFileFiltered << endl;
	cout << "  7. " << slackPlotFileFiltered << endl;
	cout << "  8. " << areaSlackPlotFileFiltered << endl;
	cout << "  9. " << areaPlotFileFiltered << endl;
	cout << " 10. " << synthesisMetricsPlotFileFiltered << endl;
	cout << "\nOpen these files in a web browser to view the interactive plots." << endl;
	cout << "\n";
	
	return 0;
}
