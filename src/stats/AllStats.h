/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_ALLSTATS_H
#define BRIDGE_ALLSTATS_H

#include <vector>

#include "ValStats.h"
#include "TextStats.h"
#include "CompStats.h"
#include "RefStats.h"
#include "DuplStats.h"
#include "Timers.h"

using namespace std;

struct Options;

struct AllStats
{
  ValStats vstats;
  TextStats tstats;
  CompStats cstats;
  RefStats refstats;
  DuplStats duplstats;
  Timers timers;

  void operator += (const AllStats& as2)
  {
    vstats += as2.vstats;
    tstats += as2.tstats;
    cstats += as2.cstats;
    refstats += as2.refstats;
    duplstats += as2.duplstats;
    timers += as2.timers;
  };
};


void mergeResults(
  vector<AllStats>& allStatsList,
  const Options& options);

void printResults(
  const AllStats& allStats,
  const Options& options);

#endif

