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
  ValFileStats& stats)
{
  if (running.out.line == "*")
  {
    // Could be the Pavlicek bug where play is shortened.
    while (running.ref.line != "*" && valProgress(frstr, running.ref))
    {
    }

    if (frstr.eof())
    {
      valError(stats, running, BRIDGE_VAL_REF_SHORT);
      return false;
    }
    else
    {
      valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
      return true;
    }
  }

  const unsigned lo = running.out.line.length();
  const unsigned lr = running.ref.line.length();
  if (lo == 11 && lr == 11 &&
      running.out.line.at(0) != '[' &&
      running.ref.line.at(0) != '[')
  {
    // Could be a short play line, "S4 -- -- -- --" (Pavlicek notation!).
    unsigned poso = 0;
    while (poso < lo && running.out.line.at(poso) != '-')
      poso++;
    
    if (poso > 0 && poso < lo)
    {
      if (running.ref.line.substr(0, poso) ==
          running.out.line.substr(0, poso))
      {
        valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
        return true;
      }
      else
      {
        valError(stats, running, BRIDGE_VAL_ERROR);
        return false;
      }
    }
  }

  regex re("^\\[(\\w+)\\s+\"(.*)\"\\]$");
  smatch match;
  while (1)
  {
    if (! regex_search(running.ref.line, match, re))
      return false;

    const string refField = match.str(1);
    const string refValue = match.str(2);

    if (refField == "Event")
      valError(stats, running, BRIDGE_VAL_EVENT);
    else if (refField == "Date")
      valError(stats, running, BRIDGE_VAL_DATE);
    else if (refField == "Description")
      valError(stats, running, BRIDGE_VAL_TITLE);
    else if (refField == "Stage")
      valError(stats, running, BRIDGE_VAL_SESSION);
    else if (refField == "HomeTeam")
      valError(stats, running, BRIDGE_VAL_TEAMS);
    else if (refField == "VisitTeam")
      valError(stats, running, BRIDGE_VAL_TEAMS);
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
          valError(stats, running, BRIDGE_VAL_REF_SHORT);
          return false;
        }

        if (running.ref.line.length() == 0)
        {
          valError(stats, running, BRIDGE_VAL_ERROR);
          return false;
        }

        valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
        if (running.ref.line.substr(0, 1) == "[")
          break;
      }

      return true;
    }
    else
      break;

    if (! valProgress(frstr, running.ref))
    {
      valError(stats, running, BRIDGE_VAL_REF_SHORT);
      return false;
    }

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
// cout << "refField " << refField << endl;
// cout << "ref match '" << refValue << "'" << endl;
// cout << "out match '" << matchOut.str(2) << "'" << endl;
// cout << "substr '" << refValue.substr(0, lOut) << "'" << endl;
      valError(stats, running, BRIDGE_VAL_ERROR);
      return false;
    }

    if (! valProgress(fostr, running.out))
    {
      valError(stats, running, BRIDGE_VAL_OUT_SHORT);
      return false;
    }

  }

  if (running.ref.line == running.out.line)
    return true;

  if (isPBNSite(running.out.line, running.ref.line))
  {
    valError(stats, running, BRIDGE_VAL_LOCATION);
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
      valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
      return true;
    }
  }

  return false;
}

