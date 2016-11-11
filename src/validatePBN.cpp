/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>

#include "validate.h"
#include "valint.h"
#include "validatePBN.h"
#include "parse.h"
#include "Bexcept.h"


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
    unsigned poso = valState.dataOut.line.find('-');
    
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

  if (valState.dataRef.type == BRIDGE_BUFFER_STRUCTURED &&
      valState.dataOut.type == BRIDGE_BUFFER_STRUCTURED &&
      valState.dataRef.label == valState.dataOut.label)
  {
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

  if (valState.dataOut.type != BRIDGE_BUFFER_STRUCTURED)
    THROW("Out unstructured");

  while (1)
  {
    if (valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED)
      return false;

    const string refField = valState.dataRef.label;
    const string refValue = valState.dataRef.value;

    if (refField == "Event")
      prof.log(BRIDGE_VAL_EVENT, valState);
    else if (refField == "Date")
      prof.log(BRIDGE_VAL_DATE, valState);
    else if (refField == "Description")
      prof.log(BRIDGE_VAL_TITLE, valState);
    else if (refField == "Stage")
      prof.log(BRIDGE_VAL_SESSION, valState);
    else if (refField == "HomeTeam")
      prof.log(BRIDGE_VAL_TEAMS, valState);
    else if (refField == "VisitTeam")
      prof.log(BRIDGE_VAL_TEAMS, valState);
    else
      break;

    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataOut.label != valState.dataRef.label)
      continue;

    if (valState.dataOut.value == valState.dataRef.value)
      return true;
    else if (valState.dataRef.label == "Site")
    {
      if (refContainsOutValue(valState))
      {
        prof.log(BRIDGE_VAL_LOCATION, valState);
        return true;
      }
      else
        return false;
    }
    else if (valState.dataRef.label == "Stage")
    {
      if (refContainsOutValue(valState))
      {
        prof.log(BRIDGE_VAL_SESSION, valState);
        return true;
      }
      else
        return false;
    }

    return false;
  }

  return false;
}

