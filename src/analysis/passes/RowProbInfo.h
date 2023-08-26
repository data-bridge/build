/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_ROWPROBINFO_H
#define BRIDGE_ROWPROBINFO_H

#include <string>

#include "SigmoidData.h"

using namespace std;

struct RowProbInfo
{
  float prob;

  bool algoFlag;
  SigmoidData sigmoidData;
  float probAdder;
};

#endif

