/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCDD_H
#define BRIDGE_FUNCDD_H

#include <iostream>
#include <string>

class Group;
class Files;

using namespace std;


void dispatchDD(
  Group& group,
  Files& files,
  const string& fname,
  ostream& flog);

#endif
