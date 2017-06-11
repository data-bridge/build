/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCWRITE_H
#define BRIDGE_FUNCWRITE_H

#include "bconst.h"

class Group;

using namespace std;


void setWriteTables();

void dispatchWrite(
  const string& fname,
  const Format format,
  const BoardOrder order,
  const Group& group,
  string& text,
  ostream& flog);

#endif
