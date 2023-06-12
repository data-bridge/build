/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCVALIDATE_H
#define BRIDGE_FUNCVALIDATE_H

class ValStats;

#include "../bconst.h"

using namespace std;


void dispatchValidate(
  const FileTask& task,
  const FileOutputTask& otask,
  const Options& options,
  const string& text,
  ValStats& vstats,
  ostream& flog);

#endif
