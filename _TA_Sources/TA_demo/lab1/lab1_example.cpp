//UCLA EE 201A Lab 1

// *****************************************************************************
// *****************************************************************************
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
#include <vector>
#include <numeric>
#include "oaDesignDB.h"

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
		cout << endl << "----- Problem 2 -----" << endl;

        // compute average fanout
        int total_net = 0;
        int fanout;
        vector<int> fanout_array;
        
        oaString netName;
        oaString instName;

        oaIter<oaNet> netIterator(block->getNets());
        while (oaNet * net = netIterator.getNext()) {
            total_net++;
            fanout = 0;

            net->getName(ns, netName);
            cout << netName << ":\t";

            oaIter<oaTerm> TermIterator(net->getTerms());
            oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
            

            if( (net->getSigType()==0) && 
            (net->getInstTerms().getCount()>0) && 
            (net->getTerms().getCount()==0) ) {

                while (oaInstTerm * instTerm = instTermIterator.getNext()) {
                    fanout++;

                    instTerm->getTermName(ns, instName);
                    cout << instName << ",";                
                }
                cout << "fanout = " << fanout << endl;
                fanout_array.push_back(fanout - 1);
            }
            else {
                cout << "Not a valid net" << endl;
            }
            // ignore power, ground, and clock nets
        }

        int fanout_sum = accumulate(fanout_array.begin(), fanout_array.end(), 0);
        double fanout_avg = (fanout_sum * 1.0) / fanout_array.size();

        // Print all values in the vector
        cout << "Fanout_array values: " << endl;
        for (vector<int>::iterator it = fanout_array.begin(); it != fanout_array.end(); ++it) {
        cout << *it << " ";
        }
        cout << endl;
        
		//EE 201A Lab 1 Problem 3 starts here
		cout << endl << "----- Problem 3 -----" << endl;

        //compute average HPWL
        int HPWL = 0;
        vector<int> HPWL_array;
        oaPoint Point_1;
        oaPoint Point_2;    
        oaBox Box;

        netIterator = oaIter<oaNet> (block->getNets());
        while (oaNet * net = netIterator.getNext()) {
            
            if(net->getInstTerms().getCount()==2)
            {
                // cout << "2 instterms" << endl;
                net->getName(ns, netName);
                cout << netName << ":\t";

                oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
      
                oaInstTerm *InstTerm_1 = instTermIterator.getNext();
                oaInst *Inst_1 = InstTerm_1->getInst();
                Inst_1->getOrigin(Point_1);
                
                oaInstTerm *InstTerm_2 = instTermIterator.getNext();
                oaInst *Inst_2 = InstTerm_2->getInst();
                Inst_2->getOrigin(Point_2);

                HPWL = abs(Point_1.x() - Point_2.x()) + abs(Point_1.y() - Point_2.y());
                cout << HPWL << endl;

                HPWL_array.push_back(HPWL);
            }
            else if(net->getInstTerms().getCount()==1 && net->getTerms().getCount()==1) {
                
                cout << "1 instterms and 1 term" << endl;
                oaCollection<oaTerm, oaNet> Term_Collection = net->getTerms();
                cout << "Term obtained";
                oaIter<oaTerm> TermIterator(net->getTerms());
                cout << "Iterator created";
                oaTerm *Term = TermIterator.getNext();
                cout << "Iterator moved forward";
                oaIter<oaPin> PinIterator(Term->getPins());
                oaPin *Pin = PinIterator.getNext();
                oaIter<oaPinFig> PinFigIterator(Pin->getFigs());
                oaPinFig *PinFig = PinFigIterator.getNext();
                PinFig->getBBox(Box);
                Box.getCenter(Point_1);
                
                oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
                oaInstTerm *instTerm = instTermIterator.getNext();
                oaInst *Inst_0 = instTerm->getInst();
                Inst_0->getOrigin(Point_2);

                HPWL = abs(Point_1.x() - Point_2.x()) + abs(Point_1.y() - Point_2.y());
                cout << HPWL << endl;

                HPWL_array.push_back(HPWL);
            }
        }

        // Print all values in the vector
        cout << "HPWL_array values: " << endl;
        for (vector<int>::iterator it = HPWL_array.begin(); it != HPWL_array.end(); ++it) {
        cout << *it << " ";
        }
        cout << endl;

        int HPWL_sum = accumulate(HPWL_array.begin(), HPWL_array.end(), 0);
        double HPWL_avg = (HPWL_sum * 1.0) / HPWL_array.size();

		//Output answers:
		cout << "Problem 2 -- Average fanout " << fanout_avg << endl;
		cout << "Problem 3 -- Average wirelength " << HPWL_avg << endl;
        

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
