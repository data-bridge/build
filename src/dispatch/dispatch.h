/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_DISPATCH_H
#define BRIDGE_DISPATCH_H

struct Options;
class Files;
struct AllStats;

using namespace std;

void setTables();

void dispatch(
  const size_t thrNo, 
  Files& files,
  const Options& options,
  AllStats& allStats);

#endif
