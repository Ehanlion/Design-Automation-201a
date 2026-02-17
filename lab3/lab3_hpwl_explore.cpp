// Author: Ethan Owen
// UID: 905452983
// UCLA EE 201A Lab 3 - Problem 1 HPWL Exploration Only

#include "oaDesignDB.h"
#include <algorithm>
#include <climits>
#include <iomanip>
#include <iostream>
#include <string>

#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonFunctions.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class.1/ee/ee201o/ee201ota/oa/examples/oa/common/commonTechObserver.h"

using namespace oa;
using namespace std;

static oaNativeNS ns;

struct NetHPWLDebug {
	oaInt4 minX;
	oaInt4 maxX;
	oaInt4 minY;
	oaInt4 maxY;
	oaInt4 hpwl;
	int endpointCount;
	bool hasGeometry;
};

enum class InstEndpointMode {
	Center,
	Origin
};

static inline const char* endpointModeName(InstEndpointMode mode) {
	return (mode == InstEndpointMode::Center) ? "center" : "origin";
}

// Build one bounding box for a top-level terminal endpoint from all pin figures.
// Returns false when the terminal has no geometric figures.
static inline bool getTermEndpointBox(oaTerm* term, oaBox& termBox) {
	bool initialized = false;

	oaIter<oaPin> pinIter(term->getPins());
	while (oaPin* pin = pinIter.getNext()) {
		oaIter<oaPinFig> pinFigIter(pin->getFigs());
		while (oaPinFig* pinFig = pinFigIter.getNext()) {
			oaBox figBox;
			pinFig->getBBox(figBox);

			if (!initialized) {
				termBox = figBox;
				initialized = true;
			} else {
				oaPoint ll = termBox.lowerLeft();
				oaPoint ur = termBox.upperRight();
				oaPoint fll = figBox.lowerLeft();
				oaPoint fur = figBox.upperRight();
				termBox.set(min(ll.x(), fll.x()), min(ll.y(), fll.y()),
							max(ur.x(), fur.x()), max(ur.y(), fur.y()));
			}
		}
	}

	return initialized;
}

static inline void initNetDebug(NetHPWLDebug& d) {
	d.minX = INT_MAX;
	d.maxX = INT_MIN;
	d.minY = INT_MAX;
	d.maxY = INT_MIN;
	d.hpwl = 0;
	d.endpointCount = 0;
	d.hasGeometry = false;
}

static inline void includePoint(NetHPWLDebug& d, oaInt4 x, oaInt4 y) {
	if (!d.hasGeometry) {
		d.minX = d.maxX = x;
		d.minY = d.maxY = y;
		d.hasGeometry = true;
		return;
	}

	if (x < d.minX)
		d.minX = x;
	if (x > d.maxX)
		d.maxX = x;
	if (y < d.minY)
		d.minY = y;
	if (y > d.maxY)
		d.maxY = y;
}

// Endpoint assumption for this exploration:
// 1) Every endpoint is represented as a point.
// 2) Top-level terms use the center of their merged pin-figure box.
// 3) Instance terms use either center or origin (selected by mode).
// HPWL is the half-perimeter of the smallest bbox containing all endpoint points.
static inline NetHPWLDebug computeHPWLForNetDebug(oaNet* net,
												  InstEndpointMode mode) {
	NetHPWLDebug d;
	initNetDebug(d);

	oaIter<oaTerm> termIterator(net->getTerms());
	while (oaTerm* term = termIterator.getNext()) {
		d.endpointCount++;
		oaBox termBox;
		if (!getTermEndpointBox(term, termBox)) {
			continue;
		}

		oaPoint ll = termBox.lowerLeft();
		oaPoint ur = termBox.upperRight();
		oaInt4 cx = (ll.x() + ur.x()) / 2;
		oaInt4 cy = (ll.y() + ur.y()) / 2;
		includePoint(d, cx, cy);
	}

	oaIter<oaInstTerm> instTermIterator(net->getInstTerms());
	while (oaInstTerm* instTerm = instTermIterator.getNext()) {
		d.endpointCount++;
		oaInst* instance = instTerm->getInst();
		if (mode == InstEndpointMode::Center) {
			oaBox instBox;
			instance->getBBox(instBox);
			oaInt4 cx = (instBox.lowerLeft().x() + instBox.upperRight().x()) / 2;
			oaInt4 cy = (instBox.lowerLeft().y() + instBox.upperRight().y()) / 2;
			includePoint(d, cx, cy);
		} else {
			oaPoint origin;
			instance->getOrigin(origin);
			includePoint(d, origin.x(), origin.y());
		}
	}

	if (!d.hasGeometry) {
		d.minX = d.maxX = 0;
		d.minY = d.maxY = 0;
		d.hpwl = 0;
		return d;
	}

	d.hpwl = (d.maxX - d.minX) + (d.maxY - d.minY);
	return d;
}

void printDesignNames(oaDesign* design) {
	oaString libName, cellName, viewName;
	design->getLibName(ns, libName);
	design->getCellName(ns, cellName);
	design->getViewName(ns, viewName);
	cout << "\tThe library name for this design is : " << libName << endl;
	cout << "\tThe cell name for this design is : " << cellName << endl;
	cout << "\tThe view name for this design is : " << viewName << endl;
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

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
					lib = oaLib::create(libraryName, libPath,
										oacSharedLibMode, DMSystem);
				} else {
					lib = oaLib::create(libraryName, libPath);
				}
			}
			if (lib) {
				updateLibDefsFile(libraryName, libPath);
			} else {
				cerr << "ERROR : Unable to create " << libPath << "/"
					 << library << endl;
				return 1;
			}
		}

		cout << "The design is created and opened in 'write' mode." << endl;
		oaDesign* design = oaDesign::open(libraryName, cellName, viewName,
										  viewType, 'a');

		printDesignNames(design);

		oaBlock* block = design->getTopBlock();
		if (!block) {
			block = oaBlock::create(design);
		}

		cout << endl
			 << "----- Ethan Owen: Problem 1 HPWL Exploration -----" << endl;
		cout << "Reminder -- old HPWL baseline (DBU): 5601160" << endl;
		cout << "Assumption -- endpoints are points, and HPWL is bbox half-perimeter over all endpoint points." << endl;
		cout << "Assumption -- includes all nets (power, ground, clock, floating, etc.) with no exclusions." << endl;
		cout << "Experiment -- per net, compute HPWL with instance-center and instance-origin, then keep the lower value." << endl;
		cout << endl;
		cout << "Per-net debug: NET_NAME choose hpwl xmin xmax ymin ymax center_hpwl origin_hpwl endpoints" << endl;

		long long totalCenterHPWL = 0;
		long long totalOriginHPWL = 0;
		long long totalChosenHPWL = 0;
		int netsCounted = 0;
		int netsWithoutEndpoints = 0;
		int netsWithoutGeometry = 0;
		int netsCenterBetter = 0;
		int netsOriginBetter = 0;
		int netsTied = 0;

		oaIter<oaNet> netIterator(block->getNets());
		while (oaNet* net = netIterator.getNext()) {
			oaString netName;
			net->getName(ns, netName);

			NetHPWLDebug centerD =
				computeHPWLForNetDebug(net, InstEndpointMode::Center);
			NetHPWLDebug originD =
				computeHPWLForNetDebug(net, InstEndpointMode::Origin);

			totalCenterHPWL += centerD.hpwl;
			totalOriginHPWL += originD.hpwl;

			bool pickCenter = (centerD.hpwl <= originD.hpwl);
			const NetHPWLDebug& chosenD = pickCenter ? centerD : originD;
			totalChosenHPWL += chosenD.hpwl;

			netsCounted++;
			if (centerD.endpointCount == 0) {
				netsWithoutEndpoints++;
			}
			if (!chosenD.hasGeometry) {
				netsWithoutGeometry++;
			}
			if (centerD.hpwl < originD.hpwl) {
				netsCenterBetter++;
			} else if (originD.hpwl < centerD.hpwl) {
				netsOriginBetter++;
			} else {
				netsTied++;
			}

			cout << "NET_DEBUG " << netName
				 << " choose=" << endpointModeName(
						pickCenter ? InstEndpointMode::Center
								   : InstEndpointMode::Origin)
				 << " hpwl=" << chosenD.hpwl
				 << " xmin=" << chosenD.minX
				 << " xmax=" << chosenD.maxX
				 << " ymin=" << chosenD.minY
				 << " ymax=" << chosenD.maxY
				 << " center_hpwl=" << centerD.hpwl
				 << " origin_hpwl=" << originD.hpwl
				 << " endpoints=" << chosenD.endpointCount << endl;
		}

		cout << endl;
		cout << "Problem 1 -- Nets counted in total HPWL: " << netsCounted
			 << " (no endpoints " << netsWithoutEndpoints
			 << ", no geometry " << netsWithoutGeometry << ")" << endl;
		cout << "Problem 1 -- Total HPWL center-only (DBU): " << fixed << setprecision(0)
			 << (double)totalCenterHPWL << endl;
		cout << "Problem 1 -- Total HPWL origin-only (DBU): " << fixed << setprecision(0)
			 << (double)totalOriginHPWL << endl;
		cout << "Problem 1 -- Total HPWL choose-lower-per-net (DBU): " << fixed << setprecision(0)
			 << (double)totalChosenHPWL << endl;
		cout << "Problem 1 -- Net wins: center=" << netsCenterBetter
			 << " origin=" << netsOriginBetter
			 << " tie=" << netsTied << endl;

		design->close();
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
