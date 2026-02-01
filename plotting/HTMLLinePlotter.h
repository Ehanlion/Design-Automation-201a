// HTMLLinePlotter.h
// Generic HTML line plot generator using Chart.js for CSV data
// Supports single and multi-line plots
// Author: Ethan Owen
// UID: 905452983

#ifndef HTML_LINE_PLOTTER_H
#define HTML_LINE_PLOTTER_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * Configuration for a single dataset (one line on the plot)
 */
struct DatasetConfig {
	int columnIndex;          // Which CSV column to plot
	std::string label;        // Dataset label for legend
	std::string lineColor;    // Line color (rgba format)
	std::string pointColor;   // Point color (rgba format)
	
	// Default constructor
	DatasetConfig()
		: columnIndex(1)
		, label("Data")
		, lineColor("rgba(54, 162, 235, 1)")
		, pointColor("rgba(54, 162, 235, 0.6)") {
	}
	
	// Parameterized constructor
	DatasetConfig(int col, const std::string& lbl, const std::string& lColor, const std::string& pColor)
		: columnIndex(col)
		, label(lbl)
		, lineColor(lColor)
		, pointColor(pColor) {
	}
};

/**
 * Configuration structure for CSV line plot
 */
struct CSVLinePlotConfig {
	// Titles and labels
	std::string pageTitle;        // HTML page title
	std::string chartTitle;       // Main chart title
	std::string xAxisLabel;       // X-axis label
	std::string yAxisLabel;       // Y-axis label
	
	// Data configuration
	int xColumnIndex;             // Which CSV column for X-axis
	std::vector<DatasetConfig> datasets;  // Multiple Y-axis datasets
	bool skipHeader;              // Skip first line if true
	
	// Chart styling
	int precision;                // Decimal places for displaying numbers
	
	// Default constructor with sensible defaults
	CSVLinePlotConfig()
		: pageTitle("Line Plot")
		, chartTitle("Data Plot")
		, xAxisLabel("X")
		, yAxisLabel("Y")
		, xColumnIndex(0)
		, skipHeader(true)
		, precision(2) {
	}
};

/**
 * Generic HTML Line Plotter Class
 * Generates interactive HTML line plots from CSV files using Chart.js
 * Supports both single-line and multi-line plots
 * 
 * Usage:
 *   CSVLinePlotConfig config;
 *   config.xColumnIndex = 0;
 *   config.datasets.push_back(DatasetConfig(1, "Area", "rgba(54, 162, 235, 1)", "rgba(54, 162, 235, 0.6)"));
 *   HTMLLinePlotter plotter;
 *   plotter.plotFromCSV("data.csv", "output.html", config);
 */
class HTMLLinePlotter {
private:
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
	 * Split a string by delimiter
	 */
	std::vector<std::string> split(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);
		std::string token;
		while (std::getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}
	
	/**
	 * Trim whitespace from string
	 */
	std::string trim(const std::string& str) {
		size_t first = str.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return "";
		size_t last = str.find_last_not_of(" \t\r\n");
		return str.substr(first, last - first + 1);
	}
	
	/**
	 * Parse CSV file and extract data for plotting
	 */
	bool parseCSV(const std::string& csvFile, const CSVLinePlotConfig& config,
				  std::vector<double>& xData, 
				  std::vector<std::vector<double> >& yDatasets) {
		std::ifstream file(csvFile.c_str());
		if (!file.is_open()) {
			std::cerr << "ERROR: Could not open CSV file " << csvFile << std::endl;
			return false;
		}
		
		// Initialize y datasets
		yDatasets.resize(config.datasets.size());
		
		std::string line;
		bool firstLine = true;
		int lineNum = 0;
		
		while (std::getline(file, line)) {
			lineNum++;
			
			// Skip header if configured
			if (firstLine && config.skipHeader) {
				firstLine = false;
				continue;
			}
			firstLine = false;
			
			// Skip empty lines
			if (trim(line).empty()) {
				continue;
			}
			
			// Split by comma
			std::vector<std::string> tokens = split(line, ',');
			
			// Validate we have enough columns
			if ((int)tokens.size() <= config.xColumnIndex) {
				std::cerr << "WARNING: Line " << lineNum << " missing X column (index " 
						  << config.xColumnIndex << ")" << std::endl;
				continue;
			}
			
			// Parse X value
			try {
				double xVal = atof(trim(tokens[config.xColumnIndex]).c_str());
				xData.push_back(xVal);
			} catch (...) {
				std::cerr << "WARNING: Line " << lineNum << " invalid X value" << std::endl;
				continue;
			}
			
			// Parse Y values for each dataset
			for (size_t i = 0; i < config.datasets.size(); i++) {
				int yColIndex = config.datasets[i].columnIndex;
				
				if ((int)tokens.size() <= yColIndex) {
					std::cerr << "WARNING: Line " << lineNum << " missing Y column (index " 
							  << yColIndex << ")" << std::endl;
					yDatasets[i].push_back(0.0);  // Default to 0
					continue;
				}
				
				try {
					double yVal = atof(trim(tokens[yColIndex]).c_str());
					yDatasets[i].push_back(yVal);
				} catch (...) {
					std::cerr << "WARNING: Line " << lineNum << " invalid Y value for dataset " 
							  << i << std::endl;
					yDatasets[i].push_back(0.0);  // Default to 0
				}
			}
		}
		
		file.close();
		
		// Validate we got some data
		if (xData.empty()) {
			std::cerr << "ERROR: No valid data found in CSV file" << std::endl;
			return false;
		}
		
		// Validate all datasets have same size
		for (size_t i = 0; i < yDatasets.size(); i++) {
			if (yDatasets[i].size() != xData.size()) {
				std::cerr << "ERROR: Dataset " << i << " has different size than X data" << std::endl;
				return false;
			}
		}
		
		return true;
	}
	
	/**
	 * Write HTML header and styles
	 */
	void writeHeader(std::ofstream& htmlFile, const CSVLinePlotConfig& config) {
		htmlFile << "<!DOCTYPE html>\n";
		htmlFile << "<html>\n";
		htmlFile << "<head>\n";
		htmlFile << "    <title>" << config.pageTitle << "</title>\n";
		htmlFile << "    <script src=\"https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js\"></script>\n";
		htmlFile << "    <style>\n";
		htmlFile << "        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n";
		htmlFile << "        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n";
		htmlFile << "        h1 { text-align: center; color: #333; margin-bottom: 30px; }\n";
		htmlFile << "        canvas { background: white; }\n";
		htmlFile << "        .info { margin-top: 20px; padding: 15px; background: #f9f9f9; border-radius: 4px; }\n";
		htmlFile << "        .info p { margin: 5px 0; color: #666; }\n";
		htmlFile << "    </style>\n";
		htmlFile << "</head>\n";
		htmlFile << "<body>\n";
		htmlFile << "    <div class=\"container\">\n";
		htmlFile << "        <h1>" << config.chartTitle << "</h1>\n";
	}
	
	/**
	 * Write chart canvas and script
	 */
	void writeChart(std::ofstream& htmlFile, const CSVLinePlotConfig& config,
					const std::vector<double>& xData,
					const std::vector<std::vector<double> >& yDatasets) {
		htmlFile << "        <canvas id=\"lineChart\" width=\"400\" height=\"200\"></canvas>\n";
		htmlFile << "        <div class=\"info\">\n";
		htmlFile << "            <p><strong>Data Points:</strong> " << xData.size() << "</p>\n";
		htmlFile << "            <p><strong>Datasets:</strong> " << config.datasets.size() << "</p>\n";
		htmlFile << "        </div>\n";
		htmlFile << "    </div>\n";
		htmlFile << "    <script>\n";
		htmlFile << "        const ctx = document.getElementById('lineChart').getContext('2d');\n";
		htmlFile << "        const chart = new Chart(ctx, {\n";
		htmlFile << "            type: 'line',\n";
		htmlFile << "            data: {\n";
		
		// Write X-axis labels
		htmlFile << "                labels: [";
		for (size_t i = 0; i < xData.size(); i++) {
			if (i > 0) htmlFile << ", ";
			htmlFile << xData[i];
		}
		htmlFile << "],\n";
		
		// Write datasets
		htmlFile << "                datasets: [\n";
		for (size_t d = 0; d < config.datasets.size(); d++) {
			const DatasetConfig& ds = config.datasets[d];
			
			htmlFile << "                    {\n";
			htmlFile << "                        label: '" << ds.label << "',\n";
			htmlFile << "                        data: [";
			
			for (size_t i = 0; i < yDatasets[d].size(); i++) {
				if (i > 0) htmlFile << ", ";
				htmlFile << yDatasets[d][i];
			}
			
			htmlFile << "],\n";
			htmlFile << "                        borderColor: '" << ds.lineColor << "',\n";
			htmlFile << "                        backgroundColor: '" << ds.pointColor << "',\n";
			htmlFile << "                        borderWidth: 2,\n";
			htmlFile << "                        pointRadius: 4,\n";
			htmlFile << "                        pointHoverRadius: 6,\n";
			htmlFile << "                        tension: 0.1,\n";
			htmlFile << "                        fill: false\n";
			htmlFile << "                    }";
			
			if (d < config.datasets.size() - 1) {
				htmlFile << ",";
			}
			htmlFile << "\n";
		}
		htmlFile << "                ]\n";
		htmlFile << "            },\n";
		
		// Write options
		htmlFile << "            options: {\n";
		htmlFile << "                responsive: true,\n";
		htmlFile << "                maintainAspectRatio: true,\n";
		htmlFile << "                interaction: {\n";
		htmlFile << "                    mode: 'index',\n";
		htmlFile << "                    intersect: false\n";
		htmlFile << "                },\n";
		htmlFile << "                plugins: {\n";
		htmlFile << "                    legend: {\n";
		htmlFile << "                        display: true,\n";
		htmlFile << "                        position: 'top'\n";
		htmlFile << "                    },\n";
		htmlFile << "                    title: {\n";
		htmlFile << "                        display: true,\n";
		htmlFile << "                        text: '" << config.chartTitle << "',\n";
		htmlFile << "                        font: { size: 16 }\n";
		htmlFile << "                    },\n";
		htmlFile << "                    tooltip: {\n";
		htmlFile << "                        enabled: true,\n";
		htmlFile << "                        mode: 'index',\n";
		htmlFile << "                        intersect: false\n";
		htmlFile << "                    }\n";
		htmlFile << "                },\n";
		htmlFile << "                scales: {\n";
		htmlFile << "                    x: {\n";
		htmlFile << "                        display: true,\n";
		htmlFile << "                        title: {\n";
		htmlFile << "                            display: true,\n";
		htmlFile << "                            text: '" << config.xAxisLabel << "',\n";
		htmlFile << "                            font: { size: 14 }\n";
		htmlFile << "                        },\n";
		htmlFile << "                        grid: {\n";
		htmlFile << "                            display: true,\n";
		htmlFile << "                            color: 'rgba(0, 0, 0, 0.1)'\n";
		htmlFile << "                        }\n";
		htmlFile << "                    },\n";
		htmlFile << "                    y: {\n";
		htmlFile << "                        display: true,\n";
		htmlFile << "                        title: {\n";
		htmlFile << "                            display: true,\n";
		htmlFile << "                            text: '" << config.yAxisLabel << "',\n";
		htmlFile << "                            font: { size: 14 }\n";
		htmlFile << "                        },\n";
		htmlFile << "                        grid: {\n";
		htmlFile << "                            display: true,\n";
		htmlFile << "                            color: 'rgba(0, 0, 0, 0.1)'\n";
		htmlFile << "                        }\n";
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
	 * Plot data from CSV file
	 * @param csvFile Input CSV file path
	 * @param htmlOutput Output HTML file path
	 * @param config Plot configuration
	 * @return true if successful, false on error
	 */
	bool plotFromCSV(const std::string& csvFile, const std::string& htmlOutput, 
					 const CSVLinePlotConfig& config) {
		// Validate config
		if (config.datasets.empty()) {
			std::cerr << "ERROR: No datasets configured" << std::endl;
			return false;
		}
		
		// Parse CSV file
		std::vector<double> xData;
		std::vector<std::vector<double> > yDatasets;
		
		std::cout << "Parsing CSV file: " << csvFile << std::endl;
		if (!parseCSV(csvFile, config, xData, yDatasets)) {
			return false;
		}
		
		std::cout << "Found " << xData.size() << " data points" << std::endl;
		std::cout << "Found " << yDatasets.size() << " datasets" << std::endl;
		
		// Ensure output directory exists
		if (!ensureDirectoryExists(htmlOutput)) {
			std::cerr << "ERROR: Could not create directory for " << htmlOutput << std::endl;
			return false;
		}
		
		// Open output file
		std::ofstream htmlFile(htmlOutput.c_str());
		if (!htmlFile.is_open()) {
			std::cerr << "ERROR: Could not create HTML file " << htmlOutput << std::endl;
			return false;
		}
		
		// Write HTML
		writeHeader(htmlFile, config);
		writeChart(htmlFile, config, xData, yDatasets);
		
		htmlFile.close();
		
		std::cout << "Successfully created plot: " << htmlOutput << std::endl;
		return true;
	}
};

#endif // HTML_LINE_PLOTTER_H
