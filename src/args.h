/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_ARGS_H
#define BRIDGE_ARGS_H

#include "bconst.h"


void usage(
 const char base[]);

void printOptions();

void readArgs(
  int argc,
  char * argv[],
  OptionsType& options);

#endif

