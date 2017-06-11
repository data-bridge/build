/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCTEXTSTATS_H
#define BRIDGE_FUNCTEXTSTATS_H

#include "bconst.h"

class Group;
class TextStats;

using namespace std;


void dispatchTextStats(
  const FileTask& task,
  const Group& group,
  TextStats& tstats,
  ostream& flog);

#endif
