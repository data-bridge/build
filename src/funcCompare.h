/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCCOMPARE_H
#define BRIDGE_FUNCCOMPARE_H

#include "Group.h"
#include "CompStats.h"
#include "bconst.h"

using namespace std;


void dispatchCompare(
  const string& fname,
  const Format format,
  const Options& options,
  const string& text,
  Group& group,
  CompStats& cstats,
  ostream& flog);

#endif
