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
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  ValState& valState,
  ValProfile& prof)
{
  UNUSED(fostr);
  if (running.out.line == "*")
  {
    // Could be the Pavlicek bug where play is shortened.
    while (running.ref.line != "*" && valProgress(frstr, running.ref))
    {
      if (! valState.bufferRef.next(valState.dataRef))
        THROW("bufferRef ends too soon");
      if (running.ref.line != valState.dataRef.line)
        THROW("Ref lines differ");
    }

    if (frstr.eof())
    {
      prof.log(BRIDGE_VAL_REF_SHORT, running);
      return false;
    }
    else
    {
      prof.log(BRIDGE_VAL_PLAY_SHORT, running);
      return true;
    }
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
        prof.log(BRIDGE_VAL_PLAY_SHORT, running);
        return true;
      }
      else
      {
        prof.log(BRIDGE_VAL_ERROR, running);
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

      if (lOut > lRef ||
          valState.dataRef.value.substr(0, lOut) != valState.dataOut.value)
      {
        prof.log(BRIDGE_VAL_NAMES_SHORT, running);
        return false;
      }

      return true;
    }
    else
    {
      prof.log(BRIDGE_VAL_ERROR, running);
      return false;
    }
  }

  if (valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED)
  {
    prof.log(BRIDGE_VAL_ERROR, running);
    return false;
  }

  while (1)
  {
    if (valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED)
      return false;

    const string refField = valState.dataRef.label;
    const string refValue = valState.dataRef.value;

    if (refField == "Event")
      prof.log(BRIDGE_VAL_EVENT, running);
    else if (refField == "Date")
      prof.log(BRIDGE_VAL_DATE, running);
    else if (refField == "Description")
      prof.log(BRIDGE_VAL_TITLE, running);
    else if (refField == "Stage")
      prof.log(BRIDGE_VAL_SESSION, running);
    else if (refField == "HomeTeam")
      prof.log(BRIDGE_VAL_TEAMS, running);
    else if (refField == "VisitTeam")
      prof.log(BRIDGE_VAL_TEAMS, running);
    else if (refField == "Play")
    {
      // Play may be completely absent.
      if (valState.dataRef.type == BRIDGE_BUFFER_STRUCTURED && 
          valState.dataRef.label == "Play")
        return false;

      while (1)
      {
        if (! valProgress(frstr, running.ref))
        {
          prof.log(BRIDGE_VAL_REF_SHORT, running);
          if (valState.bufferRef.next(valState.dataRef))
            THROW("bufferRef ends too late");
          return false;
        }

        if (! valState.bufferRef.next(valState.dataRef))
          THROW("bufferRef ends too soon");
        if (running.ref.line != valState.dataRef.line)
          THROW("Ref lines differ");

        if (valState.dataRef.len == 0)
        {
          prof.log(BRIDGE_VAL_ERROR, running);
          return false;
        }

        prof.log(BRIDGE_VAL_PLAY_SHORT, running);
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

    if (! valProgress(frstr, running.ref))
    {
      prof.log(BRIDGE_VAL_REF_SHORT, running);
      if (valState.bufferRef.next(valState.dataRef))
        THROW("bufferRef ends too late");
      return false;
    }

    if (! valState.bufferRef.next(valState.dataRef))
      THROW("bufferRef ends too soon");
    if (running.ref.line != valState.dataRef.line)
      THROW("Ref lines differ");

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
        prof.log(BRIDGE_VAL_NAMES_SHORT, running);
        return false;
      }

      return true;
    }

    prof.log(BRIDGE_VAL_ERROR, running);
    return false;
  }

  prof.log(BRIDGE_VAL_ERROR, running);
  return false;
}

