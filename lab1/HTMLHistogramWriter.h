// HTMLHistogramWriter.h
// Generic HTML histogram generator using Chart.js
// Author: Ethan Owen
// UID: 905452983

#ifndef HTML_HISTOGRAM_WRITER_H
#define HTML_HISTOGRAM_WRITER_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

/**
 * Configuration structure for customizing HTML histogram appearance
 */
struct HTMLHistogramConfig {
	// Titles and labels
	std::string pageTitle;		 // HTML page title
	std::string chartTitle;		 // Main chart title
	std::string xAxisLabel;		 // X-axis label
	std::string yAxisLabel;		 // Y-axis label
	std::string datasetLabel;	 // Dataset label in legend
	
	// Statistics labels (set to empty string to hide)
	std::string totalLabel;		 // Label for total count
	std::string averageLabel;	 // Label for average
	std::string minLabel;		 // Label for minimum value
	std::string maxLabel;		 // Label for maximum value
	
	// Chart styling
	std::string barColor;		 // Bar fill color (rgba format)
	std::string borderColor;	 // Bar border color (rgba format)
	
	// Histogram configuration
	bool useBins;				 // true = continuous data with bins, false = discrete values
	int numBins;				 // Number of bins (only used if useBins == true)
	int precision;				 // Decimal places for displaying numbers
	
	// Chart.js options
	int tickRotation;			 // Rotation angle for x-axis tick labels (0-90)
	
	// Default constructor with sensible defaults
	HTMLHistogramConfig() 
		: pageTitle("Histogram")
		, chartTitle("Distribution Histogram")
		, xAxisLabel("Value")
		, yAxisLabel("Count")
		, datasetLabel("Frequency")
		, totalLabel("Total")
		, averageLabel("Average")
		, minLabel("")
		, maxLabel("")
		, barColor("rgba(54, 162, 235, 0.6)")
		, borderColor("rgba(54, 162, 235, 1)")
		, useBins(false)
		, numBins(20)
		, precision(2)
		, tickRotation(0) {
	}
};

/**
 * Generic HTML Histogram Writer Class
 * Generates interactive HTML histograms using Chart.js
 * 
 * Usage:
 *   HTMLHistogramConfig config;
 *   config.chartTitle = "My Data";
 *   HTMLHistogramWriter<double> writer(config);
 *   writer.write("output.html", myDataVector);
 */
template <typename T>
class HTMLHistogramWriter {
private:
	HTMLHistogramConfig config;
	
	/**
	 * Create directory if it doesn't exist
	 */
	bool ensureDirectoryExists(const std::string& filepath) {
		size_t lastSlash = filepath.find_last_of("/");
		if (lastSlash != std::string::npos) {
			std::string dirPath = filepath.substr(0, lastSlash);
			struct stat info;
			if (stat(dirPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
				// Directory doesn't exist, create it
				if (mkdir(dirPath.c_str(), 0755) != 0) {
					return false;
				}
			}
		}
		return true;
	}
	
	/**
	 * Calculate statistics from data
	 */
	void calculateStatistics(const std::vector<T>& data, double& minVal, double& maxVal, double& avgVal) {
		if (data.empty()) {
			minVal = maxVal = avgVal = 0;
			return;
		}
		
		minVal = (double)data[0];
		maxVal = (double)data[0];
		double sum = 0;
		
		for (size_t i = 0; i < data.size(); i++) {
			double val = (double)data[i];
			if (val < minVal) minVal = val;
			if (val > maxVal) maxVal = val;
			sum += val;
		}
		
		avgVal = sum / data.size();
	}
	
	/**
	 * Build histogram data structure
	 */
	void buildHistogram(const std::vector<T>& data, std::map<int, int>& histogram, 
						double& minVal, double& maxVal, double& binWidth) {
		if (config.useBins) {
			// Continuous data: use bins
			binWidth = (maxVal - minVal) / config.numBins;
			if (binWidth == 0) binWidth = 1; // Avoid division by zero
			
			for (size_t i = 0; i < data.size(); i++) {
				double val = (double)data[i];
				int binIndex = (int)((val - minVal) / binWidth);
				if (binIndex >= config.numBins) {
					binIndex = config.numBins - 1; // Put max value in last bin
				}
				histogram[binIndex]++;
			}
		} else {
			// Discrete data: count each unique value
			for (size_t i = 0; i < data.size(); i++) {
				int val = (int)data[i];
				histogram[val]++;
			}
		}
	}
	
	/**
	 * Write HTML header and styles
	 */
	void writeHeader(std::ofstream& htmlFile) {
		htmlFile << "<!DOCTYPE html>\n";
		htmlFile << "<html>\n";
		htmlFile << "<head>\n";
		htmlFile << "    <title>" << config.pageTitle << "</title>\n";
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
		htmlFile << "        <h1>" << config.chartTitle << "</h1>\n";
	}
	
	/**
	 * Write statistics section
	 */
	void writeStatistics(std::ofstream& htmlFile, size_t dataSize, 
						 double avg, double minVal, double maxVal) {
		if (!config.totalLabel.empty()) {
			htmlFile << "        <p><strong>" << config.totalLabel << ":</strong> " << dataSize << "</p>\n";
		}
		if (!config.averageLabel.empty()) {
			htmlFile << "        <p><strong>" << config.averageLabel << ":</strong> " << avg << "</p>\n";
		}
		if (!config.minLabel.empty()) {
			htmlFile << "        <p><strong>" << config.minLabel << ":</strong> " << minVal << "</p>\n";
		}
		if (!config.maxLabel.empty()) {
			htmlFile << "        <p><strong>" << config.maxLabel << ":</strong> " << maxVal << "</p>\n";
		}
	}
	
	/**
	 * Write chart canvas and script opening
	 */
	void writeChartOpen(std::ofstream& htmlFile) {
		htmlFile << "        <canvas id=\"histogramChart\" width=\"400\" height=\"200\"></canvas>\n";
		htmlFile << "    </div>\n";
		htmlFile << "    <script>\n";
		htmlFile << "        const ctx = document.getElementById('histogramChart').getContext('2d');\n";
		htmlFile << "        const chart = new Chart(ctx, {\n";
		htmlFile << "            type: 'bar',\n";
		htmlFile << "            data: {\n";
	}
	
	/**
	 * Write discrete value labels and data
	 */
	void writeDiscreteData(std::ofstream& htmlFile, const std::map<int, int>& histogram, 
						   double minVal, double maxVal) {
		int minValInt = (int)minVal;
		int maxValInt = (int)maxVal;
		
		// Generate labels
		htmlFile << "                labels: [";
		for (int i = minValInt; i <= maxValInt; i++) {
			if (i > minValInt) htmlFile << ", ";
			htmlFile << i;
		}
		htmlFile << "],\n";
		
		// Generate data
		htmlFile << "                datasets: [{\n";
		htmlFile << "                    label: '" << config.datasetLabel << "',\n";
		htmlFile << "                    data: [";
		for (int i = minValInt; i <= maxValInt; i++) {
			if (i > minValInt) htmlFile << ", ";
			auto it = histogram.find(i);
			htmlFile << (it != histogram.end() ? it->second : 0);
		}
		htmlFile << "],\n";
	}
	
	/**
	 * Write binned range labels and data
	 */
	void writeBinnedData(std::ofstream& htmlFile, const std::map<int, int>& histogram, 
						 double minVal, double binWidth) {
		// Generate labels (bin ranges)
		htmlFile << "                labels: [";
		for (int i = 0; i < config.numBins; i++) {
			if (i > 0) htmlFile << ", ";
			double binStart = minVal + i * binWidth;
			double binEnd = minVal + (i + 1) * binWidth;
			htmlFile << "\"" << binStart << "-" << binEnd << "\"";
		}
		htmlFile << "],\n";
		
		// Generate data (counts)
		htmlFile << "                datasets: [{\n";
		htmlFile << "                    label: '" << config.datasetLabel << "',\n";
		htmlFile << "                    data: [";
		for (int i = 0; i < config.numBins; i++) {
			if (i > 0) htmlFile << ", ";
			auto it = histogram.find(i);
			htmlFile << (it != histogram.end() ? it->second : 0);
		}
		htmlFile << "],\n";
	}
	
	/**
	 * Write chart styling and options
	 */
	void writeChartOptions(std::ofstream& htmlFile) {
		htmlFile << "                    backgroundColor: '" << config.barColor << "',\n";
		htmlFile << "                    borderColor: '" << config.borderColor << "',\n";
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
		htmlFile << "                            text: '" << config.yAxisLabel << "'\n";
		htmlFile << "                        }\n";
		htmlFile << "                    },\n";
		htmlFile << "                    x: {\n";
		htmlFile << "                        title: {\n";
		htmlFile << "                            display: true,\n";
		htmlFile << "                            text: '" << config.xAxisLabel << "'\n";
		htmlFile << "                        }";
		
		// Add tick rotation if configured
		if (config.tickRotation > 0) {
			htmlFile << ",\n";
			htmlFile << "                        ticks: {\n";
			htmlFile << "                            maxRotation: " << config.tickRotation << ",\n";
			htmlFile << "                            minRotation: " << config.tickRotation << "\n";
			htmlFile << "                        }\n";
		} else {
			htmlFile << "\n";
		}
		
		htmlFile << "                    }\n";
		htmlFile << "                },\n";
		htmlFile << "                plugins: {\n";
		htmlFile << "                    legend: {\n";
		htmlFile << "                        display: true,\n";
		htmlFile << "                        position: 'top'\n";
		htmlFile << "                    },\n";
		htmlFile << "                    title: {\n";
		htmlFile << "                        display: true,\n";
		htmlFile << "                        text: '" << config.chartTitle << "'\n";
		htmlFile << "                    }\n";
		htmlFile << "                }\n";
		htmlFile << "            }\n";
		htmlFile << "        });\n";
		htmlFile << "    </script>\n";
		htmlFile << "</body>\n";
		htmlFile << "</html>\n";
	}
	
public:
	/**
	 * Constructor
	 */
	HTMLHistogramWriter(const HTMLHistogramConfig& cfg) : config(cfg) {}
	
	/**
	 * Write histogram to HTML file
	 * @param filename Output file path
	 * @param data Vector of data to plot
	 * @return true if successful, false on error
	 */
	bool write(const std::string& filename, const std::vector<T>& data) {
		// Validate input
		if (data.empty()) {
			std::cerr << "ERROR: Data array is empty, cannot create histogram" << std::endl;
			return false;
		}
		
		// Ensure output directory exists
		if (!ensureDirectoryExists(filename)) {
			std::cerr << "ERROR: Could not create directory for " << filename << std::endl;
			return false;
		}
		
		// Calculate statistics
		double minVal, maxVal, avgVal;
		calculateStatistics(data, minVal, maxVal, avgVal);
		
		// Build histogram
		std::map<int, int> histogram;
		double binWidth = 0;
		buildHistogram(data, histogram, minVal, maxVal, binWidth);
		
		// Open output file
		std::ofstream htmlFile(filename.c_str());
		if (!htmlFile.is_open()) {
			std::cerr << "ERROR: Could not create histogram file " << filename << std::endl;
			return false;
		}
		
		// Write HTML components
		writeHeader(htmlFile);
		writeStatistics(htmlFile, data.size(), avgVal, minVal, maxVal);
		writeChartOpen(htmlFile);
		
		// Write data (discrete or binned)
		if (config.useBins) {
			writeBinnedData(htmlFile, histogram, minVal, binWidth);
		} else {
			writeDiscreteData(htmlFile, histogram, minVal, maxVal);
		}
		
		// Write chart options and close
		writeChartOptions(htmlFile);
		
		htmlFile.close();
		return true;
	}
};

/**
 * Convenience function to create a fanout histogram configuration
 */
inline HTMLHistogramConfig createFanoutConfig() {
	HTMLHistogramConfig config;
	config.pageTitle = "Fanout Histogram";
	config.chartTitle = "Fanout Distribution Histogram";
	config.xAxisLabel = "Fanout Value";
	config.yAxisLabel = "Number of Nets";
	config.datasetLabel = "Number of Nets";
	config.totalLabel = "Total Nets";
	config.averageLabel = "Average Fanout";
	config.barColor = "rgba(54, 162, 235, 0.6)";
	config.borderColor = "rgba(54, 162, 235, 1)";
	config.useBins = false;
	return config;
}

/**
 * Convenience function to create an HPWL histogram configuration
 */
inline HTMLHistogramConfig createHPWLConfig() {
	HTMLHistogramConfig config;
	config.pageTitle = "HPWL Histogram";
	config.chartTitle = "HPWL Distribution Histogram";
	config.xAxisLabel = "HPWL Range";
	config.yAxisLabel = "Number of Nets";
	config.datasetLabel = "Number of Nets";
	config.totalLabel = "Total Nets (2 ends)";
	config.averageLabel = "Average HPWL";
	config.minLabel = "Min HPWL";
	config.maxLabel = "Max HPWL";
	config.barColor = "rgba(255, 99, 132, 0.6)";
	config.borderColor = "rgba(255, 99, 132, 1)";
	config.useBins = true;
	config.numBins = 20;
	config.tickRotation = 45;
	return config;
}

#endif // HTML_HISTOGRAM_WRITER_H
