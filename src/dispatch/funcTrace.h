/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCTRACE_H
#define BRIDGE_FUNCTRACE_H

#include "../bconst.h"

class Group;
class Files;

using namespace std;


void dispatchTrace(
  Group& group,
  Files& files,
  const string& fname,
  ostream& flog);

#endif
