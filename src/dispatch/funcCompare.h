/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCCOMPARE_H
#define BRIDGE_FUNCCOMPARE_H

#include "../bconst.h"

class Group;
class CompStats;

using namespace std;


void dispatchCompare(
  const string& fname,
  const Format format,
  const Options& options,
  const string& text,
  const Group& group,
  CompStats& cstats,
  ostream& flog);

#endif
