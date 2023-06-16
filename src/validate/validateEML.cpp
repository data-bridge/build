/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include "validate.h"
#include "valint.h"
#include "ValProfile.h"
#include "validateEML.h"

#define PLOG(x) prof.log(x, valState.dataOut, valState.dataRef)


bool validateEML(
  ValState& valState,
  ValProfile& prof)
{
  if (valState.dataOut.len < 42)
    return false;

  // A bit thin -- could look more carefully.

  if (refContainsOut(valState.dataOut, valState.dataRef))
  {
    PLOG(BRIDGE_VAL_PLAY_SHORT);
    return true;
  }
  

  if (valState.dataRef.len > 75)
  {
    // The reference play may be shifted in by 3.
    string outShort = valState.dataOut.line;
    outShort.erase(38, 3);
    if (outShort == valState.dataRef.line.substr(0, valState.dataOut.len-3))
    {
      PLOG(BRIDGE_VAL_PLAY_SHORT);
      return true;
    }
  }

  return false;
}

