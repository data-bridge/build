/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCPASSES_H
#define BRIDGE_FUNCPASSES_H

#include <vector>
#include <iostream>

class Group;
class ParamStats1D;
class ParamStats2D;
struct Options;

using namespace std;


void setPassTables();

void dispatchPasses(
  const Group& group,
  const Options& options,
  vector<ParamStats1D>& paramStats1D,
  vector<ParamStats2D>& paramStats2D,
  ostream& flog);

void passStatsContrib(
  const Group& group,
  const Options& options,
  vector<ParamStats1D>& paramStats1D);

void passPostprocess(vector<ParamStats1D>& paramStats1D);

#endif
