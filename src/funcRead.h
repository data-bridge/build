/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCREAD_H
#define BRIDGE_FUNCREAD_H

#include "Group.h"
#include "bconst.h"

using namespace std;


void setReadTables();

bool dispatchReadBuffer(
  const Format format,
  const Options& options,
  Buffer& buffer,
  Group& group,
  ostream& flog);

bool dispatchReadFile(
  const string& fname,
  const Format format,
  const Options& options,
  Group& group,
  RefLines& refLines,
  ostream& flog);

#endif
