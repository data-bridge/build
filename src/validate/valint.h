/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALINT_H
#define BRIDGE_VALINT_H

#include "../files/Buffer.h"

using namespace std;


struct ValState
{
  Buffer bufferOut;
  Buffer bufferRef;

  LineData dataOut;
  LineData dataRef;
};

#endif
