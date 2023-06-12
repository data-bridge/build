/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCPLAYERVAL_H
#define BRIDGE_FUNCPLAYERVAL_H

#include "../bconst.h"

class Group;

using namespace std;


void dispatchPlayersValidate(
  const Group& group,
  ostream& flog);

#endif
