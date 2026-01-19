//Author: Ethan Owen
//UID: 905452983
//UCLA EE 201A Lab 1

// *****************************************************************************
// Lab1.cpp
//
// The following tasks are performed by this program
//  1. Derive an oaTech observer to handle conflicts in the technology hierarchy
//  2. Derive an oaLibDefsList observer to handle warnings related to lib.defs
//  3. Open the design
//  4. Print the library name
//  5. Print the cell name
//  6. Print the view name
//  7. Create nets with the names "Hello" and "World"
//  8. Save these nets
//  9. Run the net iterator and print the existing nets in the design
//
// ****************************************************************************
// Except as specified in the OpenAccess terms of use of Cadence or Silicon
// Integration Initiative, this material may not be copied, modified,
// re-published, uploaded, executed, or distributed in any way, in any medium,
// in whole or in part, without prior written permission from Cadence.
//
//                Copyright 2002-2005 Cadence Design Systems, Inc.
//                           All Rights Reserved.
//
// To distribute any derivative work based upon this file you must first contact
// Si2 @ contracts@si2.org.
//
// *****************************************************************************
// *****************************************************************************

// Standard C++ input/output library for printing to console
#include <iostream>
// Standard C++ vector and numeric algorithms
#include <vector>
#include <numeric>
// OpenAccess Design Database API - provides access to IC design data structures
#include "oaDesignDB.h"

// Includes from a class library - helper utilities for OpenAccess
// Observer classes handle warnings/conflicts that may occur during design operations
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"


// Using OpenAccess namespace - allows us to use oaDesign, oaBlock, etc. without "oa::" prefix
using namespace oa;
// Using standard namespace - allows us to use cout, endl, etc. without "std::" prefix
using namespace std;

// OpenAccess uses a "namespace" system for naming objects. oaNativeNS is the default namespace
// This is used when getting/setting names of libraries, cells, views, nets, etc.
static oaNativeNS ns;




// ****************************************************************************
// printDesignNames()
//
// This function extracts and displays the hierarchical names of an OpenAccess design.
// In OpenAccess, designs are organized hierarchically:
//   - Library: A collection of cells (like a folder containing multiple designs)
//   - Cell: A specific design or component (like a file in that folder)
//   - View: A particular representation of that cell (e.g., "layout", "schematic", "netlist")
//
// Parameters:
//   design - Pointer to an oaDesign object that has already been opened
// ****************************************************************************
void
printDesignNames(oaDesign *design)
{
    // oaString is OpenAccess's string type - used to store names
    oaString libName;
    oaString cellName;
    oaString viewName;

    // Extract the names from the design object using the namespace (ns)
    // These methods populate the oaString variables with the actual names
    design->getLibName(ns, libName);
    design->getCellName(ns, cellName);
    design->getViewName(ns, viewName);

    // Print the extracted names to the console
    cout << "\tThe library name for this design is : " << libName << endl;
    cout << "\tThe cell name for this design is : " << cellName << endl;
    cout << "\tThe view name for this design is : " << viewName << endl;
}



// ****************************************************************************
// printNets()
//  
// This function iterates through all nets in a design and prints their names.
// A "net" in IC design represents an electrical connection (wire) between components.
// A "block" is the container that holds all the design elements (nets, instances, etc.)
// The "TopBlock" is the top-level block of the design hierarchy.
//
// Parameters:
//   design - Pointer to an oaDesign object that has already been opened
// ****************************************************************************
void
printNets(oaDesign *design)
{
    // Get the TopBlock - this is where all the nets, instances, and other design elements live
    oaBlock *block = design->getTopBlock();

    // Check if the block exists (some designs might not have a block yet)
    // if (block) {
    //     oaString netName;
    //     cout << "The following nets exist in this design." << endl;
    //     oaIter<oaNet> netIterator(block->getNets());
    //     while (oaNet *net = netIterator.getNext()) {
    //         net->getName(ns, netName);
    //         cout << "\t" << netName << endl;
    //     }
    // } else {
    //     cout << "There is no block in this design" << endl;
    // }

    if(block) {
        oaString netName;
        cout << "The following nets exist in this design." << endl;
        // Print out nets but do 3 per line
        int count = 0;
        oaIter<oaNet> netIterator(block->getNets());
        while (oaNet *net = netIterator.getNext()) {
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
    }

}




// ****************************************************************************
// main()
//
// This is the entry point of the program. It performs the following steps:
//   1. Initialize the OpenAccess API
//   2. Set up observers to handle warnings/conflicts
//   3. Find or create the design library
//   4. Open the specified design (library/cell/view)
//   5. Print design information and existing nets
//   6. Perform lab exercises (Problem 2: fanout, Problem 3: wirelength)
//   7. Clean up by closing design and library
//
// The program uses exception handling (try/catch) to gracefully handle errors
// that may occur during OpenAccess operations.
// ****************************************************************************
int
main(int argc,
     char *argv[])
{
    try {
        // Initialize OpenAccess API - must be called before any other OA operations
        // Parameters: major version, minor version, data model version (3 = supports incremental tech DBs)
        // This sets up the OpenAccess runtime environment
        oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);

        // Define the library path (where the design files are stored on disk)
        oaString libPath("./DesignLib");
        // Define the library name (logical name in OpenAccess)
        oaString library("DesignLib");
        // Get the view type - oacMaskLayout means this is a physical layout view
        // (as opposed to schematic, netlist, etc.)
        oaViewType *viewType = oaViewType::get(oacMaskLayout);
        // Define the cell name (the specific design we want to open)
        oaString cell("s1196_bench");
        // Define the view name (which view of this cell we want)
        oaString view("layout");
        
        // Create OpenAccess name objects - these are used to identify objects in the hierarchy
        // oaScalarName wraps a string in OpenAccess's naming system
        oaScalarName libName(ns, library);
        oaScalarName cellName(ns, cell);
        oaScalarName viewName(ns, view);
        oaScalarName libraryName(ns, library);
        
        // Setup observers - these handle warnings and conflicts that may occur
        // Observers are like event handlers that get called when certain things happen
        // The parameter (1) typically means "verbose mode" - print all messages
        // Tech conflict observer: handles conflicts in technology database (layer definitions, etc.)
        opnTechConflictObserver myTechConflictObserver(1);
        // LibDefList observer: handles warnings related to library definition files (lib.defs)
        opnLibDefListObserver myLibDefListObserver(1);

        // Try to find the library - this looks in lib.defs (library definition file)
        // lib.defs is a configuration file that tells OpenAccess where libraries are located
        oaLib *lib = oaLib::find(libraryName);

        // If library wasn't found in lib.defs, we need to handle it
        if (!lib) {
            // Check if the library directory actually exists on disk
            if (oaLib::exists(libPath)) {
                // Library exists on disk but wasn't registered in lib.defs
                // Open it directly by specifying the path
                lib = oaLib::open(libraryName, libPath);
            } else {
                // Library doesn't exist - we need to create it
                // Check if there's a DM (Design Manager) system environment variable
                // DM systems provide version control and collaboration features
                char *DMSystem = getenv("DMSystem");
                if (DMSystem) {
                    // Create library with DM system support (shared/versioned mode)
                    lib = oaLib::create(libraryName, libPath, oacSharedLibMode, DMSystem);
                } else {
                    // Create a simple local library without DM system
                    lib = oaLib::create(libraryName, libPath);
                }
            }
            // After finding or creating the library, update lib.defs so it's registered
            if (lib) {
                // Update lib.defs file so future runs can find this library automatically
                updateLibDefsFile(libraryName, libPath);
            } else {
                // Failed to create/open library - print error and exit
                cerr << "ERROR : Unable to create " << libPath << "/";
                cerr << library << endl;
                return(1);
            }
        }
        // Open the design - this loads the design data into memory
        // Parameters:
        //   libraryName, cellName, viewName - identify which design to open
        //   viewType - the type of view (layout, schematic, etc.)
        //   'r' - open in READ mode (use 'w' for write mode to modify the design)
        // Note: The cout message says 'write' but the code uses 'r' - this may be intentional
        cout << "The design is created and opened in 'write' mode." << endl;

        oaDesign *design = oaDesign::open(libraryName, cellName, viewName,
                                          viewType, 'r');

        // Print information about the opened design
        // This displays the library/cell/view names we're working with
        printDesignNames(design);
        // Print all the nets that exist in this design
        printNets(design);

        // Get the TopBlock - this is the container for all design elements
        // The block contains nets, instances (components), terminals (pins), etc.
        oaBlock *block = design->getTopBlock();

        // Some designs might not have a block yet (empty/new designs)
        // If no block exists, create one so we can add elements to it
        if (!block) {
            block = oaBlock::create(design);
        }

        // ========================================================================
        // EE 201A Lab 1 Problem 2: Compute Average Fanout
        // ========================================================================
        // Fanout = number of components connected to a net
        // For each net, count how many instance terminals (InstTerm) and 
        // primary terminals (Term) are connected to it
        cout << endl << "----- Firstname Lastname: Problem 2 -----" << endl;

        // TODO: Compute average fanout
        // Initialize counter for total number of nets
        int total_net = 0;
        // Create iterator to go through all nets in the block
        oaIter<oaNet> netIterator(block->getNets());
        // TODO: Create a vector/array to store fanout values for each net

        // Created fanout vector array to store fanout values for each net
        vector<int> fanout_array;
        
        // Iterate through each net in the design
        while (oaNet *net = netIterator.getNext()) {
            // Get the net name for filtering
            oaString netName;
            net->getName(ns, netName);
            
            // Filter out power, ground, and clock nets BEFORE processing
            // Based on design files: VDD (power), VSS (ground), blif_clk_net (clock)
            if (netName == "VDD" 
                || netName == "VSS" 
                || netName == "blif_clk_net") {
                continue;  // Skip special nets
            }
            
            total_net++;

            // Initialize fanout counter for this net
            int fanout = 0;

            // For each net, iterate through:
            //   1. InstTerms - terminals of instances (components) connected to this net
            //   2. Terms - primary I/O terminals (pins) connected to this net
            // Count both to get total fanout
            oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
            oaIter<oaTerm> termIterator(net->getTerms());
            
            // Count instance terminals
            while (oaInstTerm *instTerm = instTermIterator.getNext()) {
                // Increment fanout for each instance terminal
                fanout++;
            }
            
            // Count primary terminals
            while (oaTerm *term = termIterator.getNext()) {
                // Increment fanout for each primary terminal
                fanout++;
            }

            // Store fanout value for this net
            fanout_array.push_back(fanout);
        }


        // Calculate average = sum of all fanouts / number of nets
        if (fanout_array.size() > 0) {
            int totalFanout = accumulate(fanout_array.begin(), fanout_array.end(), 0);
            double avgFanout = (double)totalFanout / fanout_array.size();
            cout << "Problem 2 -- Average fanout " << avgFanout << endl;
        } else {
            cout << "Problem 2 -- Average fanout 0.0 (no nets found)" << endl;
        }

        // ========================================================================
        // EE 201A Lab 1 Problem 3: Compute Average Wirelength
        // ========================================================================
        // Wirelength = total length of wires connecting components
        // Need to access the physical geometry of nets to measure their length
        cout << endl << "----- Firstname Lastname: Problem 3 -----" << endl;

        // TODO: To get wirelength, you need to traverse the hierarchy:
        //   oaTerm (terminal/pin) --> oaPin (physical pin) --> oaPinFig (pin figure/geometry)
        //   oaPinFig inherits from oaFig, which has methods to get geometric information
        //   You'll need to iterate through nets, then through their pin figures to measure length

        // Output answers:
        cout << "Problem 2 -- Average fanout " << "*YOUR VALUE HERE*" << endl;
        cout << "Problem 3 -- Average wirelength " << "*YOUR VALUE HERE*" << endl;

        // Clean up: Close the design to free memory and release file locks
        // Always close objects when done to prevent resource leaks
        design->close();

        // Close the library as well
        lib->close();

    } catch (oaCompatibilityError &ex) {
        // Handle compatibility errors - these occur when design data format 
        // doesn't match the OpenAccess version being used
        handleFBCError(ex);
        exit(1);

    } catch (oaException &excp) {
        // Catch any other OpenAccess exceptions and print the error message
        // oaException is the base class for all OpenAccess errors
        cout << "ERROR: " << excp.getMsg() << endl;
        exit(1);
    }

    return 0;
}
