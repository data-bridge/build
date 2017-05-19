/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_DISPATCH_H
#define BRIDGE_DISPATCH_H

#include <string>
#include <vector>

#include "Files.h"
#include "Timers.h"
#include "ValStats.h"
#include "TextStats.h"
#include "CompStats.h"
#include "RefStats.h"
#include "bconst.h"

using namespace std;

void setTables();

void dispatch(
  const int thrNo, 
  Files& files,
  const Options& options,
  ValStats& vstats,
  TextStats& tstats,
  CompStats& cstats,
  Timers& timers,
  RefStats& refstats);

void mergeResults(
  vector<ValStats>& vstats,
  vector<TextStats>& tstats,
  vector<CompStats>& cstats,
  vector<Timers>& timer,
  vector<RefStats>& refstats,
  const Options& options);

#endif
