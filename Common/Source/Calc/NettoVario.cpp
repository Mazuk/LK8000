/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "McReady.h"
#include "utils/heapcheck.h"

using std::min;
using std::max;

void NettoVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  double n;
  // get load factor
  if (Basic->AccelerationAvailable) {
	n = fabs(Basic->Gload);
  } else {
	n = fabs(Calculated->Gload);
  }

  // calculate sink rate of glider for calculating netto vario

  bool replay_disabled = !ReplayLogger::IsEnabled();

  double glider_sink_rate;    
  if (Basic->AirspeedAvailable && replay_disabled) {
    glider_sink_rate= GlidePolar::SinkRate(max((double)GlidePolar::Vminsink, Basic->IndicatedAirspeed), n);
  } else {
    glider_sink_rate= GlidePolar::SinkRate(max((double)GlidePolar::Vminsink, Calculated->IndicatedAirspeedEstimated), n);
  }
  Calculated->GliderSinkRate = glider_sink_rate;

  if (Basic->NettoVarioAvailable && replay_disabled) {
	Calculated->NettoVario = Basic->NettoVario;
  } else {
	if (Basic->VarioAvailable && replay_disabled) {
		Calculated->NettoVario = Basic->Vario - glider_sink_rate;
	} else {
		Calculated->NettoVario = Calculated->Vario - glider_sink_rate;
	}
  }
}
