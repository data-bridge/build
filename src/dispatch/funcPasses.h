/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCPASSES_H
#define BRIDGE_FUNCPASSES_H

#include <iostream>

class Group;
class ParamStats1D;
class ParamStats2D;
struct Options;

using namespace std;


void dispatchPasses(
  const Group& group,
  const Options& options,
  ParamStats1D& paramStats1D,
  ParamStats2D& paramStats2D,
  ostream& flog);

#endif
