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
      {
        prof.log(BRIDGE_VAL_ERROR, valState);
        return false;
      }
    }
  }

  if (valState.dataRef.type == BRIDGE_BUFFER_STRUCTURED &&
      valState.dataOut.type == BRIDGE_BUFFER_STRUCTURED &&
      valState.dataRef.label == valState.dataOut.label)
  {
    if (valState.dataRef.label == "West" || 
        valState.dataRef.label == "North" ||
        valState.dataRef.label == "East" || 
        valState.dataRef.label == "South" ||
        valState.dataRef.label == "Site" || 
        valState.dataRef.label == "Stage")
    {
      const unsigned lRef = valState.dataRef.value.length();
      const unsigned lOut = valState.dataOut.value.length();
bool nn = refContainsOutValue(valState);

      if (lOut > lRef ||
          valState.dataRef.value.substr(0, lOut) != valState.dataOut.value)
      {
if (nn)
  THROW("Conflict 1");
        prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
        return false;
      }
if (! nn)
  THROW("Conflict 2");

      return true;
    }
    else
    {
      prof.log(BRIDGE_VAL_ERROR, valState);
      return false;
    }
  }

  if (valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED)
  {
    prof.log(BRIDGE_VAL_ERROR, valState);
    return false;
  }

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
    else if (refField == "Play")
    {
      // Play may be completely absent.
      if (valState.dataRef.type == BRIDGE_BUFFER_STRUCTURED && 
          valState.dataRef.label == "Play")
        return false;

      while (1)
      {
        if (! valState.bufferRef.next(valState.dataRef))
        {
          prof.log(BRIDGE_VAL_REF_SHORT, valState);
          return false;
        }

        if (valState.dataRef.len == 0)
        {
          prof.log(BRIDGE_VAL_ERROR, valState);
          return false;
        }

        prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
        if (valState.dataRef.type == BRIDGE_BUFFER_STRUCTURED)
          break;
      }

      return true;
    }
    else
      break;

    // May in fact have the same field in the output, but shorter.
    if (valState.dataOut.type != BRIDGE_BUFFER_STRUCTURED)
      continue;

    if (! valState.bufferRef.next(valState.dataRef))
    {
      prof.log(BRIDGE_VAL_REF_SHORT, valState);
      return false;
    }

    if (valState.dataOut.label != valState.dataRef.label)
      continue;

    if (valState.dataOut.value == valState.dataRef.value)
      return true;
    else if (valState.dataRef.label == "Site" && 
        valState.dataOut.value == "")
      return true;
    else if (valState.dataRef.label == "Stage")
    {
      const unsigned lRef = valState.dataRef.value.length();
      const unsigned lOut = valState.dataOut.value.length();

      if (lOut > lRef ||
          valState.dataRef.value.substr(0, lOut) != valState.dataOut.value)
      {
        prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
        return false;
      }

      return true;
    }

    prof.log(BRIDGE_VAL_ERROR, valState);
    return false;
  }

  prof.log(BRIDGE_VAL_ERROR, valState);
  return false;
}

