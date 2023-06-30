/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCPASSES_H
#define BRIDGE_FUNCPASSES_H

#include <iostream>

class Group;

using namespace std;


void dispatchPasses(
  const Group& group,
  ostream& flog);

#endif
