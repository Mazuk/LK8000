/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "DoInits.h"
#include "Logger.h"
#include "WindZigZag.h"
#include "utils/heapcheck.h"

using std::min;
using std::max;


void Heading(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  double x0, y0, mag;
  static double LastTime = 0;
  static double lastHeading = 0;

  if (DoInit[MDI_HEADING]) {
	LastTime = 0;
	lastHeading = 0;
	DoInit[MDI_HEADING]=false;
  }

  if ((Basic->Speed>0)||(Calculated->WindSpeed>0)) {

    x0 = fastsine(Basic->TrackBearing)*Basic->Speed;
    y0 = fastcosine(Basic->TrackBearing)*Basic->Speed;
    x0 += fastsine(Calculated->WindBearing)*Calculated->WindSpeed;
    y0 += fastcosine(Calculated->WindBearing)*Calculated->WindSpeed;

    Calculated->Heading = AngleLimit360(atan2(x0,y0)*RAD_TO_DEG);

    if (!Calculated->Flying) {
      // don't take wind into account when on ground
      Calculated->Heading = Basic->TrackBearing;
    }

    // calculate turn rate in wind coordinates
    if(Basic->Time > LastTime) {
      double dT = Basic->Time - LastTime;

      Calculated->TurnRateWind = AngleLimit180(Calculated->Heading
                                               - lastHeading)/dT;

      lastHeading = Calculated->Heading;
    }
    LastTime = Basic->Time;

    // calculate estimated true airspeed
    mag = isqrt4((unsigned long)(x0*x0*100+y0*y0*100))/10.0;
    Calculated->TrueAirspeedEstimated = mag;
    Calculated->IndicatedAirspeedEstimated = mag/AirDensityRatio(Calculated->NavAltitude);
    // estimate bank angle (assuming balanced turn)
    double angle = atan(DEG_TO_RAD*Calculated->TurnRateWind*
			Calculated->TrueAirspeedEstimated/9.81);

    Calculated->BankAngle = RAD_TO_DEG*angle;
    Calculated->Gload = 1.0/max(0.001,fabs(cos(angle)));

    // estimate pitch angle (assuming balanced turn)
/*
    Calculated->PitchAngle = RAD_TO_DEG*
      atan2(Calculated->GPSVario-Calculated->Vario,
           Calculated->TrueAirspeedEstimated);
*/
	// should be used as here only when no real vario available
    Calculated->PitchAngle = RAD_TO_DEG*	
      atan2(Calculated->Vario,
           Calculated->TrueAirspeedEstimated);

    // update zigzag wind
    if (((AutoWindMode & D_AUTOWIND_ZIGZAG)==D_AUTOWIND_ZIGZAG) 
        && (!ReplayLogger::IsEnabled())) {
      double zz_wind_speed;
      double zz_wind_bearing;
      int quality;
      quality = WindZigZagUpdate(Basic, Calculated, 
                                 &zz_wind_speed, 
				 &zz_wind_bearing);
      if (quality>0) {
        SetWindEstimate(zz_wind_speed, zz_wind_bearing, quality);
/* 100118 redundant!! removed. TOCHECK
        Vector v_wind;
        v_wind.x = zz_wind_speed*cos(zz_wind_bearing*3.1415926/180.0);
        v_wind.y = zz_wind_speed*sin(zz_wind_bearing*3.1415926/180.0);
        LockFlightData();
        if (windanalyser) {
	  windanalyser->slot_newEstimate(Basic, Calculated, v_wind, quality);
        }
        UnlockFlightData();
*/
      }
    }
  // else basic speed is 0 and there is no wind.. 
  } else { 
    Calculated->Heading = Basic->TrackBearing;
    Calculated->TrueAirspeedEstimated = 0; // BUGFIX 100318
    Calculated->IndicatedAirspeedEstimated = 0; // BUGFIX 100318
  }

}
