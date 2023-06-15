/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCTEXTSTATS_H
#define BRIDGE_FUNCTEXTSTATS_H

#include <iostream>

struct FileTask;
class Group;
class TextStats;

using namespace std;


void dispatchTextStats(
  const FileTask& task,
  const Group& group,
  TextStats& tstats,
  ostream& flog);

#endif
