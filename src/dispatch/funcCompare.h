/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCCOMPARE_H
#define BRIDGE_FUNCCOMPARE_H

#include <iostream>
#include <string>

struct Options;
class Group;
class CompStats;

enum Format: unsigned;

using namespace std;


void dispatchCompare(
  const string& fname,
  const Format format,
  const Options& options,
  const string& text,
  const Group& group,
  CompStats& cstats,
  ostream& flog);

#endif
