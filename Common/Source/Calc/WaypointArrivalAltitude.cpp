/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "utils/heapcheck.h"


// This is also called by DoNearest and it is overwriting AltitudeRequired 
double CalculateWaypointArrivalAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int i) {

  double altReqd;
  double wDistance, wBearing;
  double wStartDistance=0, wStartBearing=0;
  double safetyaltitudearrival;

  safetyaltitudearrival=GetSafetyAltitude(i);


  DistanceBearing(Basic->Latitude, 
                  Basic->Longitude,
                  WayPointList[i].Latitude, 
                  WayPointList[i].Longitude,
                  &wDistance, &wBearing);

  WayPointCalc[i].Distance = wDistance;
  WayPointCalc[i].Bearing  = wBearing;

	altReqd = GlidePolar::MacCreadyAltitude ( GetMacCready(i,GMC_DEFAULT),
		wDistance, wBearing, 
		Calculated->WindSpeed, Calculated->WindBearing, 
		0, 0, true, &WayPointCalc[i].NextETE);
	// if gates are in use with a real task, and we are at start 
	// then calculate ETE for reaching the cylinder. Also working when we are 
	// in the wrong side of cylinder
	if (UseGates() && !DoOptimizeRoute()) {
		if (ActiveWayPoint==0 && i==Task[0].Index ) { 
			if (PGStartOut) {
				if (CorrectSide()) {
					// start out,  from outside cylinder
					wStartDistance=wDistance-StartRadius;
					wStartBearing=wBearing;
				} else {
					// start out,  but inside cylinder
					wStartDistance=StartRadius-wDistance;
					wStartBearing=wBearing+180;
					if (wStartBearing>360) wStartBearing-=360;
				}
			} else {
				if (CorrectSide()) {
					// start in, correct side is inside cylinder
					wStartDistance=StartRadius-wDistance;
					wStartBearing=wBearing+180;
					if (wStartBearing>360) wStartBearing-=360;
				} else {
					// start in, and we are still outside
					wStartDistance=wDistance-StartRadius;
					wStartBearing=wBearing;
				}
			}

			// we don't use GetMacCready(i,GMC_DEFAULT)
			GlidePolar::MacCreadyAltitude ( MACCREADY,
			wStartDistance, wStartBearing, 
			Calculated->WindSpeed, Calculated->WindBearing, 
			0, 0, true, &WayPointCalc[i].NextETE);
			#ifdef DEBUGTGATES
			StartupStore(_T("wStartDistance=%f wStartBearing=%f\n"),wStartDistance,wStartBearing);
			#endif
		}
	}

        // we should build a function for this since it is used also in lkcalc
	WayPointCalc[i].AltReqd[AltArrivMode]  = altReqd+safetyaltitudearrival+WayPointList[i].Altitude -Calculated->EnergyHeight; 
	WayPointCalc[i].AltArriv[AltArrivMode] = Calculated->NavAltitude + Calculated->EnergyHeight
						- altReqd 
						- WayPointList[i].Altitude 
						- safetyaltitudearrival;
/*
		WayPointCalc[i].AltArriv[ALTA_AVEFF] = Calculated->NavAltitude 
							- (wDistance / GetCurrentEfficiency(Calculated, 0)) 
							- WayPointList[i].Altitude
							-safetyaltitudearrival; 

		WayPointCalc[i].AltReqd[ALTA_AVEFF] = Calculated->NavAltitude - WayPointCalc[i].AltArriv[ALTA_AVEFF];
		WayPointCalc[i].NextETE=600.0;
*/
 
   return(WayPointCalc[i].AltArriv[AltArrivMode]); 

}
