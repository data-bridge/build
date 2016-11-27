/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_DDSIF_H
#define BRIDGE_DDSIF_H

#include "dll.h"


struct RunningDD
{
  unsigned tricksDecl;
  unsigned tricksDef;
  bool declLeadFlag;
  deal dl;
};

unsigned tricksDD(
  RunningDD& running);

#endif
