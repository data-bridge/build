/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCPLAYERVAL_H
#define BRIDGE_FUNCPLAYERVAL_H

#include <iostream>

class Group;

using namespace std;


void dispatchPlayersValidate(
  const Group& group,
  ostream& flog);

#endif
