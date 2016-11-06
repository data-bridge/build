/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>

#include "validate.h"
#include "valint.h"
#include "validatePBN.h"
#include "parse.h"
#include "Bexcept.h"


bool isPBNSite(
  const string& lineOut,
  const string& lineRef);


bool isPBNSite(
  const string& lineOut,
  const string& lineRef)
{
  regex re("^\\[(\\w+)\\s+\"(.*)\"\\]$");
  smatch match;
  if (! regex_search(lineOut, match, re) || match.str(1) != "Site")
    return false;
  if (! regex_search(lineRef, match, re) || match.str(1) != "Site")
    return false;

  return true;
}


bool validatePBN(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  Buffer& bufferRef,
  Buffer& bufferOut,
  ValProfile& prof)
{
  LineData bref, bout;
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

  const unsigned lo = running.out.line.length();
  const unsigned lr = running.ref.line.length();

  if (lo == 11 && lr == 11 &&
      running.out.line.at(0) != '[' &&
      running.ref.line.at(0) != '[')
  {
  // if (lo != bout.len)
    // THROW("Different out length");
  // if (lr != bref.len)
    // THROW("Different out length");
    // Could be a short play line, "S4 -- -- --" (Pavlicek notation!).
    unsigned poso = 0;
    while (poso < lo && running.out.line.at(poso) != '-')
      poso++;
    
    if (poso > 0 && poso < lo)
    {
      if (running.ref.line.substr(0, poso) ==
          running.out.line.substr(0, poso))
      {
        // valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
        prof.log(BRIDGE_VAL_PLAY_SHORT, running);
        return true;
      }
      else
      {
        // valError(stats, running, BRIDGE_VAL_ERROR);
        prof.log(BRIDGE_VAL_ERROR, running);
        return false;
      }
    }
  }

  regex re("^\\[(\\w+)\\s+\"(.*)\"\\]$");
  smatch match;
  while (1)
  {
    if (! regex_search(running.ref.line, match, re))
    {
      // if (bref.type == BRIDGE_BUFFER_STRUCTURED)
        // THROW("Structured");
      return false;
    }

    const string refField = match.str(1);
    const string refValue = match.str(2);
// if (bref.type != BRIDGE_BUFFER_STRUCTURED)
  // THROW("Not structured");
// if (refField != bref.label)
  // THROW("Different ref labels");
// if (refValue != bref.value)
  // THROW("Different ref values");

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
      if (running.out.line.length() < 6 ||
          running.out.line.substr(0, 6) == "[Play ")
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

        if (running.ref.line.length() == 0)
        {
          prof.log(BRIDGE_VAL_ERROR, running);
          return false;
        }

        prof.log(BRIDGE_VAL_PLAY_SHORT, running);
        if (running.ref.line.substr(0, 1) == "[")
          break;
      }

      return true;
    }
    else
      break;

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

    // May in fact have the same field in the output, but shorter.
    smatch matchOut;
    if (! regex_search(running.out.line, matchOut, re))
      continue;

    if (matchOut.str(1) != refField)
      continue;

    const unsigned lRef = refValue.length();
    const unsigned lOut = matchOut.str(2).length();

    if (lOut > lRef ||
        refValue.substr(0, lOut) != matchOut.str(2))
    {
      prof.log(BRIDGE_VAL_ERROR, running);
      return false;
    }

    if (! valProgress(fostr, running.out))
    {
      prof.log(BRIDGE_VAL_OUT_SHORT, running);
      if (bufferOut.next(bout))
        THROW("bufferOut ends too late");
      return false;
    }

    if (! bufferOut.next(bout))
      THROW("bufferOut ends too soon");
    if (running.out.line != bout.line)
      THROW("Out lines differ");
  }

  if (running.ref.line == running.out.line)
    return true;

  if (isPBNSite(running.out.line, running.ref.line))
  {
    // valError(stats, running, BRIDGE_VAL_LOCATION);
    prof.log(BRIDGE_VAL_LOCATION, running);
    return true;
  }

  const string label = match.str(1);
  smatch matchOut;
  if (! regex_search(running.out.line, matchOut, re))
    return false;
  if (label != matchOut.str(1))
    return false;

  if (label == "West" || label == "North" ||
      label == "East" || label == "South")
  {
    const unsigned lOut = matchOut.str(2).length();
    if (lOut < match.str(2).length() &&
        match.str(2).substr(0, lOut) == matchOut.str(2))
    {
      // valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
      prof.log(BRIDGE_VAL_NAMES_SHORT, running);
      return true;
    }
  }

  return false;
}

