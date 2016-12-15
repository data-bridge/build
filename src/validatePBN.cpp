/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "validatePBN.h"


bool validatePBN(
  ValState& valState,
  ValProfile& prof)
{
  if (valState.dataOut.line == "*")
  {
    // Could be the Pavlicek bug where play is shortened.
    while (valState.dataRef.line != "*" && 
        valState.bufferRef.next(valState.dataRef))
    {
    }

    prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
    return true;
  }

  const unsigned lo = valState.dataOut.len;
  const unsigned lr = valState.dataRef.len;

  if (lo == 11 && lr == 11 &&
      valState.dataOut.type != BRIDGE_BUFFER_STRUCTURED &&
      valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED)
  {
    // Could be a short play line, "S4 -- -- --" (Pavlicek notation!).
    unsigned poso = static_cast<unsigned>
      (valState.dataOut.line.find('-'));
    
    if (poso > 0 && poso < lo)
    {
      if (valState.dataRef.line.substr(0, poso) == 
          valState.dataOut.line.substr(0, poso))
      {
        prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
        return true;
      }
      else
        return false;
    }
  }

  if (valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED ||
      valState.dataOut.type != BRIDGE_BUFFER_STRUCTURED)
    return false;

  while (1)
  {
    if (valState.dataRef.label == valState.dataOut.label)
    {
      if (valState.dataOut.value == valState.dataRef.value)
        return true;

      if ((valState.dataRef.label == "West" || 
           valState.dataRef.label == "North" ||
           valState.dataRef.label == "East" || 
           valState.dataRef.label == "South") &&
           refContainsOutValue(valState))
      {
        prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
        return true;
      }
      else if (valState.dataRef.label == "Site" &&
        refContainsOutValue(valState))
      {
        prof.log(BRIDGE_VAL_LOCATION, valState);
        return true;
      }
      else if (valState.dataRef.label == "Stage" &&
        refContainsOutValue(valState))
      {
        prof.log(BRIDGE_VAL_SESSION, valState);
        return true;
      }
      else
        return false;
    }

    ValError ve;
    if (valState.dataRef.label == "Event")
      ve = BRIDGE_VAL_EVENT;
    else if (valState.dataRef.label == "Date")
      ve = BRIDGE_VAL_DATE;
    else if (valState.dataRef.label == "Description")
      ve = BRIDGE_VAL_TITLE;
    else if (valState.dataRef.label == "Stage")
      ve = BRIDGE_VAL_SESSION;
    else if (valState.dataRef.label == "HomeTeam")
      ve = BRIDGE_VAL_TEAMS;
    else if (valState.dataRef.label == "VisitTeam")
      ve = BRIDGE_VAL_TEAMS;
    else
      break;

    if (! valState.bufferRef.next(valState.dataRef) ||
        valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED)
      return false;

    prof.log(ve, valState);
  }

  return false;
}

