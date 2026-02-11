// Author: Firstname Lastname
// UID: XXX-XXX-XXX
// UCLA EE 201A Lab 3

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

#include "oaDesignDB.h"
#include <bits/stdc++.h>
#include <iostream>
#include <sys/time.h>

#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace oa;
using namespace std;

static oaNativeNS ns;

// ****************************************************************************
// printDesignNames()
//
// This function gets the library, cell and view names associated with the open
// design and prints them.
// ****************************************************************************
void printDesignNames(oaDesign* design) {
	oaString libName;
	oaString cellName;
	oaString viewName;

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
void printNets(oaDesign* design) {
	// Get the TopBlock of the current design
	oaBlock* block = design->getTopBlock();

	if (block) {
		oaString netName;

		cout << "The following nets exist in this design." << endl;

		// Iterate over all nets in the design
		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
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
int main(int argc,
		 char* argv[]) {
	try {
		// Initialize OA with data model 3, since incremental technology
		// databases are supported by this application.
		oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);

		oaString libPath("./DesignLib");
		oaString library("DesignLib");
		oaViewType* viewType = oaViewType::get(oacMaskLayout);
		oaString cell("s1196_bench");
		oaString view("layout");
		oaScalarName libName(ns,
							 library);
		oaScalarName cellName(ns,
							  cell);
		oaScalarName viewName(ns,
							  view);
		oaScalarName libraryName(ns, library);
		// Setup an instance of the oaTech conflict observer.
		opnTechConflictObserver myTechConflictObserver(1);

		// Setup an instance of the oaLibDefList observer.
		opnLibDefListObserver myLibDefListObserver(1);

		// Read in the lib.defs file.
		oaLib* lib = oaLib::find(libraryName);

		if (!lib) {
			if (oaLib::exists(libPath)) {
				// Library does exist at this path but was not in lib.defs
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
				// We need to update the user's lib.def file since we either
				// found or created the library without a lib.defs reference.
				updateLibDefsFile(libraryName, libPath);
			} else {
				// Print error mesage
				cerr << "ERROR : Unable to create " << libPath << "/";
				cerr << library << endl;
				return (1);
			}
		}
		// Create the design with the specified viewType,
		// Opening it for a 'write' operation.
		cout << "The design is created and opened in 'write' mode." << endl;

		oaDesign* design = oaDesign::open(libraryName, cellName, viewName,
										  viewType, 'r');

		// The library, cell, and view names are printed.
		printDesignNames(design);
		printNets(design);

		// Get the TopBlock for this design.
		oaBlock* block = design->getTopBlock();

		// If no TopBlock exist yet then create one.
		if (!block) {
			block = oaBlock::create(design);
		}

		// EE 201A Lab 3 Problem 1 starts here
		cout << endl
			 << "----- Firstname Lastname: Problem 1 -----" << endl;

		// EE 201A Lab 3 Problem 2 starts here
		cout << endl
			 << "----- Firstname Lastname: Problem 2 -----" << endl;
		struct timeval start, end;
		gettimeofday(&start, NULL);

		// ..... YOUR CODE HERE

		gettimeofday(&end, NULL);
		double time_taken;
		time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec -
									start.tv_usec)) *
					 1e-6;

		// Output answers
		cout << "Problem 1 -- Total wirelength of original design: " << "*YOUR VALUE HERE*" << endl;
		cout << "Problem 2 -- Total wirelength AFTER my incremental placement algorithm:  " << "*YOUR VALUE HERE*" << endl;
		cout << "Problem 2 -- Total number of swaps used:  " << "*YOUR VALUE HERE*" << endl;
		cout << "Problem 2 -- Time taken:  " << fixed
			 << time_taken << setprecision(6);
		cout << " sec" << endl;

		// The design is closed.
		design->close();

		// The library is closed.
		lib->close();

	} catch (oaCompatibilityError& ex) {
		handleFBCError(ex);
		exit(1);

	} catch (oaException& excp) {
		cout << "ERROR: " << excp.getMsg() << endl;
		exit(1);
	}

	return 0;
}
