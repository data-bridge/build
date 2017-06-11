/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCIMPSHEET_H
#define BRIDGE_FUNCIMPSHEET_H

#include "bconst.h"

class Group;

using namespace std;


void dispatchIMPSheet(
  const Group& group,
  ostream& flog);

#endif
