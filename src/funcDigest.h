/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCDIGEST_H
#define BRIDGE_FUNCDIGEST_H

#include "bconst.h"

using namespace std;


void dispatchDigest(
  const FileTask& task,
  const Options& options,
  ostream& flog);

#endif
