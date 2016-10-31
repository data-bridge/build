/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_DISPATCH_H
#define BRIDGE_DISPATCH_H

#include <string>
#include <vector>

#include "Files.h"
#include "Timers.h"
#include "ValStats.h"
#include "bconst.h"

using namespace std;

void setTables();

void dispatch(
  const int thrNo, 
  Files& files,
  const OptionsType& options,
  ValStats& vstats,
  Timers& timers);

void mergeResults(
  vector<ValStats>& vstats,
  vector<Timers>& timer,
  const OptionsType& options);

#endif
