/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCREFSTATS_H
#define BRIDGE_FUNCREFSTATS_H

#include "RefLines.h"
#include "RefStats.h"
#include "bconst.h"

using namespace std;


void dispatchRefStats(
  const RefLines& refLines,
  RefStats& refstats,
  ostream& flog);

#endif
