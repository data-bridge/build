/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_DISPATCH_H
#define BRIDGE_DISPATCH_H

#include <string>
#include <vector>

#include "Files.h"
#include "AllStats.h"

using namespace std;

void setTables();

void dispatch(
  const int thrNo, 
  Files& files,
  const Options& options,
  AllStats& allStats);

#endif
