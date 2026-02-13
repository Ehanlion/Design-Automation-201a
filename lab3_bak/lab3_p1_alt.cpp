// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3
//
// Alternate Part 1-only implementation:
// - Counts HPWL on signal nets (power/ground filtered out).
// - Uses transformed master-pin centroids for instance terminals.
// - Uses top-level term pin centroids for I/O terminals.

#include "oaDesignDB.h"
#include <iomanip>
#include <iostream>
#include <string>

#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace oa;
using namespace std;

static oaNativeNS ns;

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

bool isNetIncludedForPart1(oaNet* net) {
	oaSigTypeEnum sigType = net->getSigType();
	if (sigType == oacPowerSigType || sigType == oacGroundSigType) {
		return false;
	}

	oaString netName;
	net->getName(ns, netName);
	if (netName == "VDD" || netName == "VSS") {
		return false;
	}

	return true;
}

bool getCentroidPointFromTerm(oaTerm* term, oaPoint& centroid) {
	long long sumX = 0;
	long long sumY = 0;
	int count = 0;

	oaIter<oaPin> pinIterator(term->getPins());
	while (oaPin* pin = pinIterator.getNext()) {
		oaIter<oaPinFig> pinFigIterator(pin->getFigs());
		while (oaPinFig* pinFig = pinFigIterator.getNext()) {
			oaBox pinBox;
			pinFig->getBBox(pinBox);
			oaPoint ll = pinBox.lowerLeft();
			oaPoint ur = pinBox.upperRight();
			sumX += (ll.x() + ur.x()) / 2;
			sumY += (ll.y() + ur.y()) / 2;
			count++;
		}
	}

	if (count == 0) {
		return false;
	}

	centroid.set((oaCoord)(sumX / count), (oaCoord)(sumY / count));
	return true;
}

bool getInstTermPoint(oaInstTerm* instTerm, oaPoint& instPoint) {
	oaInst* inst = instTerm->getInst();
	if (!inst) {
		return false;
	}

	oaPoint masterCentroid;
	bool haveMasterPinCentroid = false;

	oaTerm* term = instTerm->getTerm();
	if (term) {
		haveMasterPinCentroid = getCentroidPointFromTerm(term, masterCentroid);
	}

	if (!haveMasterPinCentroid) {
		// Fallback if master-pin geometry is unavailable.
		inst->getOrigin(instPoint);
		return true;
	}

	oaTransform xform;
	inst->getTransform(xform);
	instPoint = masterCentroid;
	instPoint.transform(xform);
	return true;
}

double computeHPWLForNet_PinCentroid(oaNet* net) {
	bool initialized = false;
	oaInt4 minX = 0;
	oaInt4 minY = 0;
	oaInt4 maxX = 0;
	oaInt4 maxY = 0;

	auto expandBBox = [&](oaInt4 x, oaInt4 y) {
		if (!initialized) {
			minX = maxX = x;
			minY = maxY = y;
			initialized = true;
			return;
		}
		if (x < minX) minX = x;
		if (x > maxX) maxX = x;
		if (y < minY) minY = y;
		if (y > maxY) maxY = y;
	};

	oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
	while (oaInstTerm* instTerm = instTermIterator.getNext()) {
		oaPoint p;
		if (getInstTermPoint(instTerm, p)) {
			expandBBox(p.x(), p.y());
		}
	}

	oaIter<oaTerm> termIterator(net->getTerms());
	while (oaTerm* term = termIterator.getNext()) {
		oaPoint p;
		if (getCentroidPointFromTerm(term, p)) {
			expandBBox(p.x(), p.y());
		}
	}

	if (!initialized) {
		return -1.0;
	}

	return (double)((maxX - minX) + (maxY - minY));
}

double computeTotalHPWL_Part1Alt(oaDesign* design, int& netsCounted, int& skippedNets) {
	oaBlock* block = design->getTopBlock();
	if (!block) {
		cout << "ERROR: No block found in the design." << endl;
		netsCounted = 0;
		skippedNets = 0;
		return 0.0;
	}

	double totalHPWL = 0.0;
	netsCounted = 0;
	skippedNets = 0;

	oaIter<oaNet> netIterator(block->getNets());
	while (oaNet* net = netIterator.getNext()) {
		if (!isNetIncludedForPart1(net)) {
			continue;
		}

		double hpwl = computeHPWLForNet_PinCentroid(net);
		if (hpwl >= 0.0) {
			totalHPWL += hpwl;
			netsCounted++;
		} else {
			skippedNets++;
		}
	}

	return totalHPWL;
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

		cout << endl << "----- Ethan Owen: Problem 1 (Alt) -----" << endl;

		int netsCounted = 0;
		int skippedNets = 0;
		double totalHPWL = computeTotalHPWL_Part1Alt(design, netsCounted, skippedNets);

		cout << "Problem 1 -- Nets counted in total HPWL: " << netsCounted
			 << " (skipped " << skippedNets << ")" << endl;
		cout << "Problem 1 -- Total HPWL (DBU): " << fixed << setprecision(0)
			 << totalHPWL << endl;

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
