/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCCLEAR_H
#define BRIDGE_FUNCCLEAR_H

#include <iostream>
#include <string>

class Group;
class Files;

using namespace std;


void dispatchClear(
  Group& group,
  Files& files,
  const string& fname,
  ostream& flog);

#endif
