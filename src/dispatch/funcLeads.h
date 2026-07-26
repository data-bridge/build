/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCLEADS_H
#define BRIDGE_FUNCLEADS_H

#include <iostream>
#include <string>

class Group;
class Files;

using namespace std;


void dispatchLeads(
  Group& group,
  Files& files,
  const string& fname,
  ostream& flog);

#endif
