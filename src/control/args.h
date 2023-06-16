/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_ARGS_H
#define BRIDGE_ARGS_H

#include "Options.h"


void usage(const char base[]);

void printOptions(const Options& options);

void readArgs(
  int argc,
  char * argv[],
  Options& options);

#endif

