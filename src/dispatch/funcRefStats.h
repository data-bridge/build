/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCREFSTATS_H
#define BRIDGE_FUNCREFSTATS_H

#include <iostream>

class RefLines;
class RefStats;

enum Format: unsigned;

using namespace std;


void dispatchRefStats(
  const string& fname,
  const Format format,
  RefLines& refLines,
  RefStats& refstats,
  ostream& flog);

#endif
