/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALIDATE_H
#define BRIDGE_VALIDATE_H

#include <string>

// #include "ValStats.h"
#include "Buffer.h"
#include "bconst.h"

using namespace std;

class ValStats;
class Buffer;


struct ValState
{
  Buffer bufferOut;
  Buffer bufferRef;

  LineData dataOut;
  LineData dataRef;
};


bool isRecordComment(
  const string& lineOut,
  const string& lineRef);

void validate(
  const string& fileOut,
  const string& fileRef,
  const Format fOrig,
  const Format fRef,
  const Options& options,
  ValStats& vstats);

bool refContainsOut(const ValState& valState);

#endif
