// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3
//
// Simple helper:
// Print nets that contain at least one pair of instances that are swappable
// under the rule:
//   1) identical cell type
//   2) exactly identical orientation

#include "oaDesignDB.h"
#include <iostream>
#include <string>
#include <vector>

#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace oa;
using namespace std;

static oaNativeNS ns;

struct InstInfo {
	string instName;
	string cellName;
	oaDesign* master;
	oaOrient orient;
};

void printDesignNames(oaDesign* design) {
	oaString libName;
	oaString cellName;
	oaString viewName;

	design->getLibName(ns, libName);
	design->getCellName(ns, cellName);
	design->getViewName(ns, viewName);

	cout << "\tThe library name for this design is : " << libName << endl;
	cout << "\tThe cell name for this design is : " << cellName << endl;
	cout << "\tThe view name for this design is : " << viewName << endl;
}

bool sameCellAndOrient(const InstInfo& a, const InstInfo& b) {
	bool sameCell = false;
	if (a.master && b.master) {
		sameCell = (a.master == b.master);
	} else {
		sameCell = (a.cellName == b.cellName);
	}

	if (!sameCell) {
		return false;
	}

	return ((oaOrientEnum)a.orient == (oaOrientEnum)b.orient);
}

int main(int argc, char* argv[]) {
	try {
		oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);

		oaString libPath("./DesignLib");
		oaString library("DesignLib");
		oaViewType* viewType = oaViewType::get(oacMaskLayout);
		oaString cell("s1196_bench");
		oaString view("layout");
		oaScalarName libName(ns, library);
		oaScalarName cellName(ns, cell);
		oaScalarName viewName(ns, view);
		oaScalarName libraryName(ns, library);

		opnTechConflictObserver myTechConflictObserver(1);
		opnLibDefListObserver myLibDefListObserver(1);

		oaLib* lib = oaLib::find(libraryName);
		if (!lib) {
			if (oaLib::exists(libPath)) {
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
				updateLibDefsFile(libraryName, libPath);
			} else {
				cerr << "ERROR : Unable to create " << libPath << "/" << library << endl;
				return 1;
			}
		}

		cout << "The design is created and opened in 'read' mode." << endl;
		oaDesign* design =
			oaDesign::open(libraryName, cellName, viewName, viewType, 'r');

		printDesignNames(design);

		oaBlock* block = design->getTopBlock();
		if (!block) {
			cerr << "ERROR: There is no block in this design." << endl;
			design->close();
			lib->close();
			return 1;
		}

		int totalNets = 0;
		int netsWithLegalPairs = 0;
		int totalLegalPairs = 0;

		cout << endl;
		cout << "Nets with legal swap pairs (identical cell + identical orient):" << endl;

		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
			totalNets++;

			vector<InstInfo> instances;
			oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
			while (oaInstTerm* instTerm = instTermIterator.getNext()) {
				oaInst* inst = instTerm->getInst();
				if (!inst) {
					continue;
				}

				InstInfo info;
				oaString instName;
				oaString cellNameStr;
				inst->getName(ns, instName);
				inst->getCellName(ns, cellNameStr);
				info.instName = string((const char*)instName);
				info.cellName = string((const char*)cellNameStr);
				info.master = inst->getMaster();
				info.orient = inst->getOrient();
				instances.push_back(info);
			}

			vector<pair<int, int> > legalPairs;
			for (int i = 0; i < (int)instances.size(); i++) {
				for (int j = i + 1; j < (int)instances.size(); j++) {
					if (sameCellAndOrient(instances[i], instances[j])) {
						legalPairs.push_back(make_pair(i, j));
					}
				}
			}

			if (legalPairs.empty()) {
				continue;
			}

			netsWithLegalPairs++;
			totalLegalPairs += (int)legalPairs.size();

			oaString netName;
			net->getName(ns, netName);
			cout << "\nNet: " << netName << "  (pairs: " << legalPairs.size() << ")" << endl;

			for (const auto& p : legalPairs) {
				const InstInfo& a = instances[p.first];
				const InstInfo& b = instances[p.second];
				cout << "  " << a.instName << " <-> " << b.instName
					 << "  cell=" << a.cellName
					 << "  orient=" << (int)(oaOrientEnum)a.orient << endl;
			}
		}

		cout << "\nSummary:" << endl;
		cout << "  Total nets scanned: " << totalNets << endl;
		cout << "  Nets with legal swap pairs: " << netsWithLegalPairs << endl;
		cout << "  Total legal pairs found: " << totalLegalPairs << endl;

		design->close();
		lib->close();

	} catch (oaCompatibilityError& ex) {
		handleFBCError(ex);
		return 1;

	} catch (oaException& excp) {
		cout << "ERROR: " << excp.getMsg() << endl;
		return 1;
	}

	return 0;
}
