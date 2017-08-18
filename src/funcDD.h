/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCDD_H
#define BRIDGE_FUNCDD_H

#include "bconst.h"

class Group;
class Files;

using namespace std;


void dispatchDD(
  const DDInfoType infoNo,
  const Group& group,
  Files& files,
  const string& fname,
  const Options& options,
  ostream& flog);

#endif
