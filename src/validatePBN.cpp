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
  while (1)
  {
    regex re("^\\[(\\w+)\\s+\"(.*)\"\\]$");
    smatch match;
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

  return false;
}

