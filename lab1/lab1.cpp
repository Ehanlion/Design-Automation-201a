//Author: Firstname Lastname
//UID: XXX-XXX-XXX
//UCLA EE 201A Lab 1

// *****************************************************************************
// HelloWorld.cpp
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

#include <iostream>
#include "oaDesignDB.h"

// Includes from a class library
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"


using namespace oa;
using namespace std;

static oaNativeNS ns;




// ****************************************************************************
// printDesignNames()
//
// This function gets the library, cell and view names associated with the open
// design and prints them.
// ****************************************************************************
void
printDesignNames(oaDesign *design)
{
    oaString    libName;
    oaString    cellName;
    oaString    viewName;

    // Library, cell and view names are obtained.
    design->getLibName(ns, libName);
    design->getCellName(ns, cellName);
    design->getViewName(ns, viewName);

    // Library, cell and view names are printed.
    cout << "\tThe library name for this design is : " << libName << endl;
    cout << "\tThe cell name for this design is : " << cellName << endl;
    cout << "\tThe view name for this design is : " << viewName << endl;
}



// ****************************************************************************
// void printNets()
//  
//  This function invokes the net iterator for the design and prints the names
//  of the nets one by one.
// ****************************************************************************
void
printNets(oaDesign *design)
{
    // Get the TopBlock of the current design
    oaBlock *block = design->getTopBlock();

    if (block) {
        oaString        netName;

        cout << "The following nets exist in this design." << endl;

        // Iterate over all nets in the design
        oaIter<oaNet>   netIterator(block->getNets());
        while (oaNet * net = netIterator.getNext()) {
            net->getName(ns, netName);
            cout << "\t" << netName << endl;
        }
    } else {
        cout << "There is no block in this design" << endl;
    }
}






// ****************************************************************************
// main()
//
// This is the top level function that opens the design, prints library, cell,
// and view names, creates nets, and iterates the design to print the net 
// names.
// ****************************************************************************
int
main(int    argc,
     char   *argv[])
{
    try {
        // Initialize OA with data model 3, since incremental technology
        // databases are supported by this application.
        oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);

        oaString                libPath("./DesignLib");
        oaString                library("DesignLib");
	    oaViewType      	*viewType = oaViewType::get(oacMaskLayout);
        oaString        	cell("s1196_bench");
       	oaString        	view("layout"); 
	    oaScalarName            libName(ns,
                                        library);
        oaScalarName            cellName(ns,
                                         cell);
        oaScalarName            viewName(ns,
                                         view);
	    oaScalarName    	libraryName(ns,library);
        // Setup an instance of the oaTech conflict observer.
        opnTechConflictObserver myTechConflictObserver(1);

        // Setup an instance of the oaLibDefList observer.
        opnLibDefListObserver   myLibDefListObserver(1);

        // Read in the lib.defs file.
		oaLib *lib = oaLib::find(libraryName);

        if (!lib) {
            if (oaLib::exists(libPath)) {
                // Library does exist at this path but was not in lib.defs
                lib = oaLib::open(libraryName, libPath);
            } else {
            char *DMSystem=getenv("DMSystem");
            if(DMSystem){
                    lib = oaLib::create(libraryName, libPath, oacSharedLibMode, DMSystem);
                } else {
                    lib = oaLib::create(libraryName, libPath);
                }
            }
            if (lib) {
                // We need to update the user's lib.def file since we either
                // found or created the library without a lib.defs reference.
                updateLibDefsFile(libraryName, libPath);
            } else {
                // Print error mesage 
                cerr << "ERROR : Unable to create " << libPath << "/";
                cerr << library << endl;
                return(1);
            }
        }
		// Create the design with the specified viewType,
        // Opening it for a 'write' operation.
        cout << "The design is created and opened in 'write' mode." << endl;

        oaDesign    *design = oaDesign::open(libraryName, cellName, viewName,
                                             viewType, 'r');

        // The library, cell, and view names are printed.
        printDesignNames(design);
		printNets(design);

		// Get the TopBlock for this design.
        oaBlock *block = design->getTopBlock();
	
		// If no TopBlock exist yet then create one.
        if (!block) {
            block = oaBlock::create(design);
        }

		//EE 201A Lab 1 Problem 2 starts here
		cout << endl << "----- Firstname Lastname: Problem 2 -----" << endl;

        // compute average fanout
        int total_net = 0;
        oaIter<oaNet> netIterator(block->getNets());
        // fanout array

        while (oaNet * net = netIterator.getNext()) {
            total_net++;
            fanout = 0;

            net 
            oaIter<oaInstTerm> instTermIterator(....)
            oaIter<oaTerm> termIterator(....)
            while (oaInstTerm * instTerm = instTermIterator.getNext()) {

            }
            fanout_array.push_back(fanout)
            // ignore power, ground, and clock nets
        }
        // what about instterms?
        cout << "Problem 2 -- Average fanout " << (double)sum(fanout_array) / len(fanout_array) << endl;

		//EE 201A Lab 1 Problem 3 starts here
		cout << endl << "----- Firstname Lastname: Problem 3 -----" << endl;

        // oaTerm --> oaPin --> oaPinFig (that inherits from oaFig, that includes a useful method you can use.)

		//Output answers:
		cout << "Problem 2 -- Average fanout " << "*YOUR VALUE HERE*" << endl;
		cout << "Problem 3 -- Average wirelength " << "*YOUR VALUE HERE*" << endl;

        // The design is closed.   
        design->close();

        // The library is closed.   
        lib->close();

    } catch (oaCompatibilityError &ex) {
        handleFBCError(ex);
        exit(1);

    } catch (oaException &excp) {
        cout << "ERROR: " << excp.getMsg() << endl;
        exit(1);
    }

    return 0;
}
