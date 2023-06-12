/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCDUPL_H
#define BRIDGE_FUNCDUPL_H

#include "../bconst.h"

class Group;
class RefLines;
class DuplStats;

using namespace std;


void dispatchDupl(
  const Group& group,
  const RefLines& reflines,
  DuplStats& duplstats,
  ostream& flog);

#endif
