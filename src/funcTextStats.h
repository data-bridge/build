/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCTEXTSTATS_H
#define BRIDGE_FUNCTEXTSTATS_H

#include "Group.h"
#include "TextStats.h"
#include "bconst.h"

using namespace std;


void dispatchTextStats(
  const FileTask& task,
  Group& group,
  TextStats& tstats,
  ostream& flog);

#endif
