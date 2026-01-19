#ifndef TERMINAL_PLOTTER_H
#define TERMINAL_PLOTTER_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <sstream>

using namespace std;

/**
 * HistogramConfig - Configuration structure for customizing histogram plots
 */
struct HistogramConfig {
    string title;                    // Main title of the histogram
    string xAxisLabel;               // Label for X-axis (e.g., "Fanout Value", "HPWL Range")
    string yAxisLabel;               // Label for Y-axis (e.g., "Number of Nets")
    string description;              // Description/subtitle at the bottom
    
    // Statistics labels
    string totalLabel;               // Label for total count (e.g., "Total Nets", "Total Nets (2 ends)")
    string averageLabel;             // Label for average (e.g., "Average Fanout", "Average HPWL")
    string minLabel;                 // Label for minimum value (empty string to hide)
    string maxLabel;                 // Label for maximum value (empty string to hide)
    
    // Histogram type configuration
    bool useBins;                    // true for continuous data (bins), false for discrete values
    int numBins;                     // Number of bins (only used if useBins == true)
    int precision;                   // Decimal precision for displaying numbers
    
    // Display configuration
    int maxBarWidth;                 // Maximum width of bars in characters
    int labelWidth;                  // Width reserved for labels
    
    // Default constructor with sensible defaults
    HistogramConfig() :
        title("Distribution Histogram"),
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
        labelWidth(28)
    {}
};

/**
 * TerminalPlotter - A class for rendering histograms to the terminal using ASCII characters
 * Replicates the functionality of the HTML plotting files but outputs to console
 */
class TerminalPlotter {
private:
    static const int DEFAULT_MAX_BAR_WIDTH = 60;
    static const int DEFAULT_LABEL_WIDTH = 28;

    /**
     * Normalize a value to fit within the display range
     */
    static int normalizeValue(int value, int maxValue, int maxWidth) {
        if (maxValue == 0) return 0;
        return (int)((double)value / maxValue * maxWidth);
    }

    /**
     * Format a number with appropriate precision
     */
    static string formatNumber(double num, int precision = 2) {
        ostringstream oss;
        oss << fixed << setprecision(precision) << num;
        return oss.str();
    }

    /**
     * Create a horizontal bar using ASCII characters
     */
    static string createBar(int width) {
        if (width <= 0) return "";
        return string(width, '#');
    }

    /**
     * Format a label string, truncating if necessary
     */
    static string formatLabel(const string& label, int maxWidth) {
        if (label.length() <= maxWidth) return label;
        return label.substr(0, maxWidth - 3) + "...";
    }

public:
    /**
     * Generic histogram plotting function
     * Works with any numeric type (int, double, float, etc.)
     * @param data Vector of numeric values to plot
     * @param config Configuration structure for customizing the plot
     */
    template<typename T>
    static void plotHistogram(const vector<T>& data, const HistogramConfig& config) {
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
                if (val < minVal) minVal = val;
                if (val > maxVal) maxVal = val;
            }

            // Create bins
            double binWidth = (maxVal - minVal) / config.numBins;
            if (binWidth == 0) binWidth = 1; // Avoid division by zero

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
                if (val < minValInt) minValInt = val;
                if (val > maxValInt) maxValInt = val;
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
        cout << config.averageLabel << ": " << formatNumber(avg, config.precision) << endl;
        if (!config.minLabel.empty()) {
            cout << config.minLabel << ": " << formatNumber(minVal, config.precision) << endl;
        }
        if (!config.maxLabel.empty()) {
            cout << config.maxLabel << ": " << formatNumber(maxVal, config.precision) << endl;
        }
        cout << "------------------------------------------------------------------------" << endl;
        cout << setw(config.labelWidth) << left << config.xAxisLabel << " | " << config.yAxisLabel << endl;
        cout << string(config.labelWidth, '-') << "-|-" << string(config.maxBarWidth, '-') << endl;

        // Print histogram bars
        if (config.useBins) {
            // Print bins
            double binWidth = (maxVal - minVal) / config.numBins;
            if (binWidth == 0) binWidth = 1;
            
            for (int i = 0; i < config.numBins; i++) {
                int count = (histogram.find(i) != histogram.end()) ? histogram[i] : 0;
                int barWidth = normalizeValue(count, maxCount, config.maxBarWidth);
                string bar = createBar(barWidth);
                
                double binStart = minVal + i * binWidth;
                double binEnd = minVal + (i + 1) * binWidth;
                string rangeLabel = formatNumber(binStart, config.precision) + "-" + formatNumber(binEnd, config.precision);
                rangeLabel = formatLabel(rangeLabel, config.labelWidth);
                
                cout << setw(config.labelWidth) << left << rangeLabel << " | " 
                     << setw(4) << right << count << " " << bar << endl;
            }
        } else {
            // Print discrete values
            int minValInt = (int)minVal;
            int maxValInt = (int)maxVal;
            
            for (int i = minValInt; i <= maxValInt; i++) {
                int count = (histogram.find(i) != histogram.end()) ? histogram[i] : 0;
                int barWidth = normalizeValue(count, maxCount, config.maxBarWidth);
                string bar = createBar(barWidth);
                
                cout << setw(config.labelWidth) << right << i << " | " 
                     << setw(4) << right << count << " " << bar << endl;
            }
        }

        cout << "------------------------------------------------------------------------" << endl;
        cout << config.description << endl;
        cout << "========================================================================" << endl;
        cout << "\n";
    }
};

#endif // TERMINAL_PLOTTER_H
