/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "utils/heapcheck.h"



void CheckTransitionFinalGlide(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  int FinalWayPoint = getFinalWaypoint();
  // update final glide mode status
  if (((ActiveWayPoint == FinalWayPoint)
       ||(ForceFinalGlide)) 
      && (ValidTaskPoint(ActiveWayPoint))) {
    
    if (Calculated->FinalGlide == 0)
      InputEvents::processGlideComputer(GCE_FLIGHTMODE_FINALGLIDE);
    Calculated->FinalGlide = 1;
  } else {
    if (Calculated->FinalGlide == 1)
      InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
    Calculated->FinalGlide = 0;
  }

}


void CheckForceFinalGlide(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // Auto Force Final Glide forces final glide mode
  // if above final glide...
    if (AutoForceFinalGlide) {
      if (!Calculated->FinalGlide) {
        if (Calculated->TaskAltitudeDifference>120) {
          ForceFinalGlide = true;
        } else {
          ForceFinalGlide = false;
        }
      } else {
        if (Calculated->TaskAltitudeDifference<-120) {
          ForceFinalGlide = false;
        } else {
          ForceFinalGlide = true;
        }
      }
    }
}


