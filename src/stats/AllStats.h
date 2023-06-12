/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_ALLSTATS_H
#define BRIDGE_ALLSTATS_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#include <vector>
#pragma warning(pop)

#include "TextStats.h"
#include "CompStats.h"
#include "RefStats.h"
#include "DuplStats.h"
#include "Timers.h"

#include "../bconst.h"


using namespace std;

struct AllStats
{
  ValStats vstats;
  TextStats tstats;
  CompStats cstats;
  RefStats refstats;
  DuplStats duplstats;
  Timers timers;
};


void mergeResults(
  vector<AllStats>& allStatsList,
  const Options& options);

void printResults(
  const AllStats& allStats,
  const Options& options);

#endif

