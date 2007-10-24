/******************************************************************************

GHAAS Water Balance/Transport Model V3.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2007, University of New Hampshire

MDDischLevel2.c

balazs.fekete@unh.edu

*******************************************************************************/

#include <stdio.h>
#include <cm.h>
#include <MF.h>
#include <MD.h>

// Inputs
static int _MDInDischLevel3ID       = MFUnset;
static int _MDInIrrUptakeExternalID = MFUnset;
// Outputs
static int _MDOutIrrUptakeRiverID   = MFUnset;
static int _MDOutIrrUptakeExcessID  = MFUnset;
static int _MDOutDischLevel2ID      = MFUnset;

static void _MDDischLevel2 (int itemID) {
// Inputs
	float discharge;       // Discharge [m3/s]
	float irrUptakeExt;    // External irrigational water uptake [mm/dt]
// Outputs
	float irrUptakeRiver;  // Irrigational water uptake from river [mm/dt]
	float irrUptakeExcess; // Irrigational water uptake from unsustainable source [mm/dt]
	float discharge_mm;

	discharge = MFVarGetFloat (_MDInDischLevel3ID, itemID, 0.0);

	if (_MDInIrrUptakeExternalID != MFUnset) {
		irrUptakeExt = MFVarGetFloat (_MDInIrrUptakeExternalID, itemID, 0.0);
		if (_MDOutIrrUptakeRiverID != MFUnset) {
			discharge_mm = discharge * 1000.0 * MFModelGet_dt () / MFModelGetArea (itemID);
			if (discharge_mm > irrUptakeExt) {
				irrUptakeRiver  = irrUptakeExt;
				irrUptakeExcess = 0.0;
				discharge_mm    = discharge_mm - irrUptakeRiver;
			}
			else {
				irrUptakeRiver  = discharge_mm;
				irrUptakeExcess = irrUptakeExt - discharge_mm;
				discharge_mm    = 0.0;
			}
			MFVarSetFloat (_MDOutIrrUptakeRiverID,  itemID, irrUptakeRiver);
		}
		else irrUptakeExcess = irrUptakeExt;
		MFVarSetFloat (_MDOutIrrUptakeExcessID, itemID, irrUptakeExcess);
		discharge = discharge_mm * MFModelGetArea (itemID) / (1000.0 * MFModelGet_dt ());
	}
	MFVarSetFloat (_MDOutDischLevel2ID,  itemID, discharge);
}

enum { MDnone, MDcalculate };

int MDDischLevel2Def() {
	int optID = MFUnset, ret;
	const char *optStr, *optName = "IrrUptakeRiver";
	const char *options [] = { MDNoneStr, MDCalculateStr, (char *) NULL };

	if (_MDOutDischLevel2ID != MFUnset) return (_MDOutDischLevel2ID);

	MFDefEntering ("Discharge Level 2");
	if (((_MDInDischLevel3ID  = MDDischLevel3Def ()) == CMfailed) ||
	    ((_MDOutDischLevel2ID = MFVarGetID ("__DischLevel2",  "m/3", MFOutput, MFState, false)) == CMfailed))
	    return (CMfailed);
	if ((ret = MDIrrGrossDemandDef ()) != MFUnset) {
		if (ret == CMfailed) return (CMfailed);
		if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
		switch (optID) {
			case MDcalculate:
				if  ((_MDOutIrrUptakeRiverID   = MDIrrUptakeRiverDef ()) == CMfailed) return (CMfailed);
			case MDnone:
				if (((_MDInIrrUptakeExternalID = MFVarGetID (MDVarIrrUptakeExternal, "mm",  MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
				    ((_MDOutIrrUptakeExcessID  = MFVarGetID (MDVarIrrUptakeExcess,   "mm",  MFOutput, MFFlux,  MFBoundary)) == CMfailed))
					return (CMfailed);
				break;
			default: MFOptionMessage (optName, optStr, options); return (CMfailed);
		}
	}
	if (MFModelAddFunction(_MDDischLevel2) == CMfailed) return (CMfailed);
	MFDefLeaving ("Discharge Level 2");
	return (_MDOutDischLevel2ID);
}
