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
  Buffer& bufferRef,
  Buffer& bufferOut,
  LineData& bref,
  LineData& bout,
  ValProfile& prof)
{
  UNUSED(bufferOut);
  UNUSED(fostr);
  if (running.out.line == "*")
  {
    // Could be the Pavlicek bug where play is shortened.
    while (running.ref.line != "*" && valProgress(frstr, running.ref))
    {
      if (! bufferRef.next(bref))
        THROW("bufferRef ends too soon");
      if (running.ref.line != bref.line)
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

  const unsigned lo = bout.len;
  const unsigned lr = bref.len;

  if (lo == 11 && lr == 11 &&
      bout.type != BRIDGE_BUFFER_STRUCTURED &&
      bref.type != BRIDGE_BUFFER_STRUCTURED)
  {
    // Could be a short play line, "S4 -- -- --" (Pavlicek notation!).
    unsigned poso = bout.line.find('-');
    
    if (poso > 0 && poso < lo)
    {
      if (bref.line.substr(0, poso) == bout.line.substr(0, poso))
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

  if (bref.type == BRIDGE_BUFFER_STRUCTURED &&
      bout.type == BRIDGE_BUFFER_STRUCTURED &&
      bref.label == bout.label)
  {
    if (bref.label == "West" || bref.label == "North" ||
       bref.label == "East" || bref.label == "South" ||
       bref.label == "Site" || bref.label == "Stage")
    {
      const unsigned lRef = bref.value.length();
      const unsigned lOut = bout.value.length();

      if (lOut > lRef ||
          bref.value.substr(0, lOut) != bout.value)
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

  if (bref.type != BRIDGE_BUFFER_STRUCTURED)
  {
    prof.log(BRIDGE_VAL_ERROR, running);
    return false;
  }

  while (1)
  {
    if (bref.type != BRIDGE_BUFFER_STRUCTURED)
      return false;

    const string refField = bref.label;
    const string refValue = bref.value;

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
      if (bref.type == BRIDGE_BUFFER_STRUCTURED && bref.label == "Play")
        return false;

      while (1)
      {
        if (! valProgress(frstr, running.ref))
        {
          prof.log(BRIDGE_VAL_REF_SHORT, running);
          if (bufferRef.next(bref))
            THROW("bufferRef ends too late");
          return false;
        }

        if (! bufferRef.next(bref))
          THROW("bufferRef ends too soon");
        if (running.ref.line != bref.line)
          THROW("Ref lines differ");

        if (bref.len == 0)
        {
          prof.log(BRIDGE_VAL_ERROR, running);
          return false;
        }

        prof.log(BRIDGE_VAL_PLAY_SHORT, running);
        if (bref.type == BRIDGE_BUFFER_STRUCTURED)
          break;
      }

      return true;
    }
    else
      break;

    // May in fact have the same field in the output, but shorter.
    if (bout.type != BRIDGE_BUFFER_STRUCTURED)
      continue;

    if (! valProgress(frstr, running.ref))
    {
      prof.log(BRIDGE_VAL_REF_SHORT, running);
      if (bufferRef.next(bref))
        THROW("bufferRef ends too late");
      return false;
    }

    if (! bufferRef.next(bref))
      THROW("bufferRef ends too soon");
    if (running.ref.line != bref.line)
      THROW("Ref lines differ");

    if (bout.label != bref.label)
      continue;

    if (bout.value == bref.value)
      return true;
    else if (bref.label == "Site" && bout.value == "")
      return true;
    else if (bref.label == "Stage")
    {
      const unsigned lRef = bref.value.length();
      const unsigned lOut = bout.value.length();

      if (lOut > lRef ||
          bref.value.substr(0, lOut) != bout.value)
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

