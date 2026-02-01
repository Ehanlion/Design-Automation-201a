# EE201A Plotting Utilities

This directory contains reusable plotting utilities for all EE201A labs.

## Available Plotters

### 1. HTMLHistogramWriter.h
Generic histogram plotter for discrete and continuous data.

**Features:**
- Discrete value histograms
- Binned continuous data histograms
- Configurable colors, labels, and bins
- Chart.js-based interactive plots

**Usage:**
```cpp
#include "../plotting/HTMLHistogramWriter.h"

HTMLHistogramConfig config;
config.chartTitle = "My Histogram";
config.xAxisLabel = "Value";
config.yAxisLabel = "Count";

HTMLHistogramWriter<int> writer(config);
writer.write("output.html", dataVector);
```

### 2. TerminalPlotter.h
ASCII-based terminal histogram plotter.

**Features:**
- Terminal-based visualization
- Same configuration as HTMLHistogramWriter
- No external dependencies

**Usage:**
```cpp
#include "../plotting/TerminalPlotter.h"

HistogramConfig config;
config.title = "My Histogram";
TerminalPlotter::plotHistogram(dataVector, config);
```

### 3. HTMLLinePlotter.h (NEW)
Generic CSV line plotter with multi-line support.

**Features:**
- Single and multi-line plots
- CSV file parsing
- Configurable colors per dataset
- Interactive Chart.js plots
- Automatic directory creation

**Usage Example 1: Single Line Plot**
```cpp
#include "../plotting/HTMLLinePlotter.h"

CSVLinePlotConfig config;
config.chartTitle = "Area vs Clock Period";
config.xAxisLabel = "Clock Period (ps)";
config.yAxisLabel = "Area (μm²)";
config.xColumnIndex = 0;

// Add single dataset
DatasetConfig area(1, "Total Area", "rgba(54, 162, 235, 1)", "rgba(54, 162, 235, 0.6)");
config.datasets.push_back(area);

HTMLLinePlotter plotter;
plotter.plotFromCSV("data.csv", "output.html", config);
```

**Usage Example 2: Multi-Line Plot**
```cpp
CSVLinePlotConfig config;
config.chartTitle = "Multiple Metrics";
config.xAxisLabel = "X Axis";
config.yAxisLabel = "Y Axis";
config.xColumnIndex = 0;

// Add multiple datasets
config.datasets.push_back(DatasetConfig(1, "Metric 1", "rgba(54, 162, 235, 1)", "rgba(54, 162, 235, 0.6)"));
config.datasets.push_back(DatasetConfig(2, "Metric 2", "rgba(75, 192, 192, 1)", "rgba(75, 192, 192, 0.6)"));
config.datasets.push_back(DatasetConfig(3, "Metric 3", "rgba(255, 99, 132, 1)", "rgba(255, 99, 132, 0.6)"));

HTMLLinePlotter plotter;
plotter.plotFromCSV("data.csv", "output.html", config);
```

## Color Palette Reference

Pre-defined colors for consistent plotting:

```cpp
// Blue
"rgba(54, 162, 235, 1)"   // Line
"rgba(54, 162, 235, 0.6)" // Point

// Green
"rgba(75, 192, 192, 1)"   // Line
"rgba(75, 192, 192, 0.6)" // Point

// Red
"rgba(255, 99, 132, 1)"   // Line
"rgba(255, 99, 132, 0.6)" // Point

// Orange
"rgba(255, 159, 64, 1)"   // Line
"rgba(255, 159, 64, 0.6)" // Point

// Purple
"rgba(153, 102, 255, 1)"  // Line
"rgba(153, 102, 255, 0.6)"// Point
```

## CSV Format

For HTMLLinePlotter, CSV files should follow this format:

```
column1,column2,column3,column4
100,1194.872,551,-280
150,1212.428,586,-247
200,1323.084,634,-164
...
```

- First row: headers (skipped if `skipHeader = true`)
- Subsequent rows: data values
- Comma-separated values
- Column indices start at 0

## Lab Examples

### Lab 1
Uses HTMLHistogramWriter and TerminalPlotter for fanout and HPWL distributions.

### Lab 2
Uses HTMLLinePlotter for synthesis analysis:
- `plot_area_delay.cpp` - Area vs delay visualization
- `scripts/plotAreaDelay.sh` - Convenience script

## Dependencies

- C++11 compiler
- Chart.js 3.9.1 (loaded from CDN in generated HTML)
- No additional libraries required

## Author

Ethan Owen (UID: 905452983)
UCLA EE 201A - VLSI Design Automation
