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
      stats.counts[BRIDGE_VAL_REF_SHORT]++;
      return false;
    }
    else
    {
      valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
      return true;
    }
  }

  regex re("^\\[(\\w+)\\s+\"(.*)\"\\]$");
  smatch match;
  while (1)
  {
    if (! regex_search(running.ref.line, match, re))
      return false;

    const string refField = match.str(1);

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
    else
      break;

    if (! valProgress(frstr, running.ref))
    {
      stats.counts[BRIDGE_VAL_REF_SHORT]++;
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

