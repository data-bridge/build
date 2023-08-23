/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_DISPATCH_H
#define BRIDGE_DISPATCH_H

#include <vector>

struct Options;
class Files;
class RuleStats;
struct AllStats;

using namespace std;

void setTables(
  vector<AllStats>& allStatsList,
  const Options& options);

void dispatch(
  const size_t thrNo, 
  Files& files,
  const Options& options,
  AllStats& allStats);

#endif
