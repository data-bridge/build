/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCDIGEST_H
#define BRIDGE_FUNCDIGEST_H

#include <iostream>

struct FileTask;
struct Options;

using namespace std;


void dispatchDigest(
  const FileTask& task,
  const Options& options,
  ostream& flog);

#endif
