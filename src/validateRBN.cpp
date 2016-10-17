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
#include "validateRBN.h"
#include "parse.h"
#include "portab.h"


bool isRBNMissing(
  const ValExample& running,
  ValFileStats& stats,
  char& rf)
{
  char of;

  unsigned lOut = running.out.line.length();
  unsigned lRef = running.ref.line.length();

  if (lOut == 0 || lRef == 0)
    return false;

  of = running.out.line.at(0);
  rf = running.ref.line.at(0);

  if (of != rf)
    return false;

  switch (of)
  {
    case 'T':
      if (lOut > 2)
        return false;
      valError(stats, running, BRIDGE_VAL_TITLE);
      return true;

    case 'D':
      if (lOut > 2)
        return false;
      valError(stats, running, BRIDGE_VAL_DATE);
      return true;
        
    case 'L':
      if (lOut > 2)
        return false;
      valError(stats, running, BRIDGE_VAL_LOCATION);
      return true;
        
    case 'E':
      if (lOut > 2)
        return false;
      valError(stats, running, BRIDGE_VAL_EVENT);
      return true;
        
    case 'S':
      if (lOut > 2)
        return false;
      valError(stats, running, BRIDGE_VAL_SESSION);
      return true;

    case 'P':
      if (lOut >= lRef || 
          running.ref.line.substr(0, lOut) != running.out.line)
        return false;
      valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
      return true;

    default:
      return false;
  }

  return false;
}


bool validateRBN(
  ifstream& frstr,
  ValExample& running,
  ValFileStats& stats)
{
  char rf;
  if (isRBNMissing(running, stats, rf))
    return true;

  if (rf == 'K')
  {
    valError(stats, running, BRIDGE_VAL_TEAMS);

    if (! valProgress(frstr, running.ref))
    {
      stats.counts[BRIDGE_VAL_REF_SHORT]++;
      return false;
    }
  }
 
  return (running.out.line == running.ref.line);
}


bool splitRBXToVector(
  const string& line,
  vector<string>& list)
{
  string s = line;
  regex re("^(.)\\{([^\\}]*)\\}");
  smatch match;
  list.clear();

  while (regex_search(s, match, re))
  {
    list.push_back(match.str(1));
    list.push_back(match.str(2));
    s = regex_replace(s, re, "");
  }

  return (s == "");
}


bool validateRBX(
  ifstream& frstr,
  ValExample& running,
  ValFileStats& stats)
{
  UNUSED(frstr);

  vector<string> vOut, vRef;
  if (! splitRBXToVector(running.out.line, vOut))
  {
    valError(stats, running, BRIDGE_VAL_ERROR);
    return false;
  }

  if (! splitRBXToVector(running.ref.line, vRef))
  {
    valError(stats, running, BRIDGE_VAL_ERROR);
    return false;
  }

  unsigned j = 0;
  for (unsigned i = 0; i < vRef.size(); i += 2)
  {
    if (j >= vOut.size())
    {
      valError(stats, running, BRIDGE_VAL_ERROR);
      return false;
    }

    unsigned lOut = vOut[j+1].length();
    unsigned lRef = vRef[i+1].length();

    if (vRef[i] != vOut[j])
    {
      if (vRef[i] == "K")
      {
        if (vOut[j+1] != "")
        {
          stats.counts[BRIDGE_VAL_ERROR]++;
          return false;
        }
        else
          // j stays unchanged
          valError(stats, running, BRIDGE_VAL_TEAMS);
      }
      else
      {
        valError(stats, running, BRIDGE_VAL_ERROR);
        return false;
      }
    }
    else if (vRef[i+1] == vOut[j+1])
    {
      j += 2;
      continue;
    }
    else if (vRef[j] == "P")
    {
      if (lOut >= lRef || vRef[j+1].substr(0, lOut) != vOut[i+1])
        return false;
      valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
    }
    else if (vOut[j+1] != "")
      return false;
    else if (vRef[j] == "T")
    {
      if (lOut > 0)
        return false;
      valError(stats, running, BRIDGE_VAL_TITLE);
    }
    else if (vRef[j] == "D")
    {
      if (lOut > 0)
        return false;
      valError(stats, running, BRIDGE_VAL_DATE);
    }
    else if (vRef[j] == "L")
    {
      if (lOut > 0)
        return false;
      valError(stats, running, BRIDGE_VAL_LOCATION);
    }
    else if (vRef[j] == "E")
    {
      if (lOut > 0)
        return false;
      valError(stats, running, BRIDGE_VAL_EVENT);
    }
    else if (vRef[j] == "S")
    {
      if (lOut > 0)
        return false;
      valError(stats, running, BRIDGE_VAL_SESSION);
    }
    else
      return false;

    j += 2;
  }
  return true;
}

