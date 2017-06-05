/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_ALLSTATS_H
#define BRIDGE_ALLSTATS_H

#include <iostream>
#include <vector>

#include "ValStats.h"
#include "TextStats.h"
#include "CompStats.h"
#include "RefStats.h"
#include "Timers.h"

#include "bconst.h"


using namespace std;

struct AllStats
{
  ValStats vstats;
  TextStats tstats;
  CompStats cstats;
  RefStats refstats;
  Timers timers;
};


void mergeResults(
  vector<AllStats>& allStatsList,
  const Options& options);

void printResults(
  const AllStats& allStats,
  const Options& options);

#endif

