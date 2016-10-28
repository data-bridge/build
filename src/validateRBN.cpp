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

#include "ValProfile.h"
#include "validate.h"
#include "valint.h"
#include "validateRBN.h"
#include "parse.h"
#include "portab.h"


bool isRBNMissing(
  ifstream& frstr,
  ValExample& running,
  ValProfile& prof,
  char& rf);

bool areRBNNames(
  const string& lineRef,
  const string& lineOut);

bool splitRBXToVector(
  const string& line,
  vector<string>& list);


bool isRBNMissing(
  ifstream& frstr,
  ValExample& running,
  ValProfile& prof,
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
  {
    if (rf == 'P' && of == 'R')
    {
      if (! valProgress(frstr, running.ref))
      {
        // valError(stats, running, BRIDGE_VAL_REF_SHORT);
        prof.log(BRIDGE_VAL_REF_SHORT, running);
        return false;
      }

      rf = running.ref.line.at(0);
      if (rf != of)
      {
        // valError(stats, running, BRIDGE_VAL_ERROR);
        prof.log(BRIDGE_VAL_ERROR, running);
        return false;
      }

      return (running.ref.line == running.out.line);
    }

    return false;
  }

  switch (of)
  {
    case 'T':
      if (lOut > 2)
        return false;
      // valError(stats, running, BRIDGE_VAL_TITLE);
      prof.log(BRIDGE_VAL_TITLE, running);
      return true;

    case 'D':
      if (lOut > 2)
        return false;
      // valError(stats, running, BRIDGE_VAL_DATE);
      prof.log(BRIDGE_VAL_DATE, running);
      return true;
        
    case 'L':
      if (lOut > 2)
        return false;
      // valError(stats, running, BRIDGE_VAL_LOCATION);
      prof.log(BRIDGE_VAL_LOCATION, running);
      return true;
        
    case 'E':
      if (lOut > 2)
        return false;
      // valError(stats, running, BRIDGE_VAL_EVENT);
      prof.log(BRIDGE_VAL_EVENT, running);
      return true;
        
    case 'S':
      if (lOut >= lRef || 
          running.ref.line.substr(0, lOut) != running.out.line)
      // if (lOut > 2)
        // return false;
      // valError(stats, running, BRIDGE_VAL_SESSION);
      prof.log(BRIDGE_VAL_SESSION, running);
      return true;

    case 'P':
      if (lOut >= lRef || 
          running.ref.line.substr(0, lOut) != running.out.line)
        return false;
      // valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
      prof.log(BRIDGE_VAL_PLAY_SHORT, running);
      return true;

    default:
      return false;
  }

}


bool areRBNNames(
  const string& lineRef,
  const string& lineOut)
{
  vector<string> listRef, listOut;
  listRef.clear();
  listOut.clear();
  tokenize(lineRef, listRef, "+:");
  tokenize(lineOut, listOut, "+:");

  if (listRef.size() != listOut.size())
    return false;

  for (unsigned i = 0; i < listRef.size(); i++)
  {
    if (listRef[i] == listOut[i])
      continue;
    unsigned lOut = listOut[i].length();
    if (lOut >= listRef[i].length() ||
        listRef[i].substr(0, lOut) != listOut[i])
      return false;
  }
  return true;
}


bool validateRBN(
  ifstream& frstr,
  ValExample& running,
  ValProfile& prof)
  // ValFileStats& stats)
{
  char rf = ' ';
  if (isRBNMissing(frstr, running, prof, rf))
    return true;

  if (rf == 'K')
  {
    // valError(stats, running, BRIDGE_VAL_TEAMS);
    prof.log(BRIDGE_VAL_TEAMS, running);

    if (! valProgress(frstr, running.ref))
    {
      // valError(stats, running, BRIDGE_VAL_REF_SHORT);
      prof.log(BRIDGE_VAL_REF_SHORT, running);
      return false;
    }
  }

  if (running.ref.line.length() > 0 && running.out.line.length() > 0)
  {
    if (running.ref.line.at(0) == 'N' && running.out.line.at(0) == 'N')
    {
      if (areRBNNames(running.ref.line, running.out.line))
        return true;
      else
      {
        // valError(stats, running, BRIDGE_VAL_ERROR);
        prof.log(BRIDGE_VAL_ERROR, running);
        return false;
      }
    }
  }
 
  if (running.out.line == running.ref.line)
    return true;
  else
  {
    // valError(stats, running, BRIDGE_VAL_ERROR);
    prof.log(BRIDGE_VAL_ERROR, running);
    return false;
  }
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
    s = regex_replace(s, re, string(""));
  }

  return (s == "");
}


bool validateRBX(
  ifstream& frstr,
  ValExample& running,
  ValProfile& prof)
  // ValFileStats& stats)
{
  UNUSED(frstr);

  vector<string> vOut, vRef;
  if (! splitRBXToVector(running.out.line, vOut))
  {
    // valError(stats, running, BRIDGE_VAL_ERROR);
    prof.log(BRIDGE_VAL_ERROR, running);
    return false;
  }

  if (! splitRBXToVector(running.ref.line, vRef))
  {
    // valError(stats, running, BRIDGE_VAL_ERROR);
    prof.log(BRIDGE_VAL_ERROR, running);
    return false;
  }

  unsigned j = 0;
  for (unsigned i = 0; i < vRef.size(); i += 2)
  {
    if (j >= vOut.size())
    {
      // valError(stats, running, BRIDGE_VAL_ERROR);
      prof.log(BRIDGE_VAL_ERROR, running);
      return false;
    }

    unsigned lOut = vOut[j+1].length();
    unsigned lRef = vRef[i+1].length();

    if (vRef[i] != vOut[j])
    {
      if (vRef[i] == "K")
      {
        // j stays unchanged
        // valError(stats, running, BRIDGE_VAL_TEAMS);
        prof.log(BRIDGE_VAL_TEAMS, running);
        continue;
      }
      else if (vRef[i] == "P" && vOut[j] == "R" && vRef[i+2] == "R")
      {
        // Play may be missing completely (e.g., coming from EML).
        // valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
        prof.log(BRIDGE_VAL_PLAY_SHORT, running);
        continue;
      }
      else
      {
        // valError(stats, running, BRIDGE_VAL_ERROR);
        prof.log(BRIDGE_VAL_ERROR, running);
        return false;
      }
    }
    else if (vRef[i+1] == vOut[j+1])
    {
      j += 2;
      continue;
    }
    else if (vRef[i] == "P")
    {
      if (lOut >= lRef || vRef[i+1].substr(0, lOut) != vOut[j+1])
        return false;
      // valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
      prof.log(BRIDGE_VAL_PLAY_SHORT, running);
    }
    else if (vRef[i] == "%")
    {
      // Record numbers have not been checked yet.
      string llOut = "% " + vOut[j+1];
      string llRef = "% " + vRef[i+1];
      if (isRecordComment(llOut, llRef))
        // valError(stats, running, BRIDGE_VAL_RECORD_NUMBER);
        prof.log(BRIDGE_VAL_RECORD_NUMBER, running);
      else
      {
        // valError(stats, running, BRIDGE_VAL_ERROR);
        prof.log(BRIDGE_VAL_ERROR, running);
        return false;
      }
    }
    else if (vRef[i] == "N")
      return areRBNNames(vRef[i+1], vOut[j+1]);
    else if (vRef[i] == "S")
    {
      if (lOut >= lRef || 
          vRef[i+1].substr(0, lOut) != vOut[j+1])
      // if (lOut > 0)
        return false;
      // valError(stats, running, BRIDGE_VAL_SESSION);
      prof.log(BRIDGE_VAL_SESSION, running);
    }
    else if (vOut[j+1] != "")
      return false;
    else if (vRef[i] == "T")
    {
      if (lOut > 0)
        return false;
      // valError(stats, running, BRIDGE_VAL_TITLE);
      prof.log(BRIDGE_VAL_TITLE, running);
    }
    else if (vRef[i] == "D")
    {
      if (lOut > 0)
        return false;
      // valError(stats, running, BRIDGE_VAL_DATE);
      prof.log(BRIDGE_VAL_DATE, running);
    }
    else if (vRef[i] == "L")
    {
      if (lOut > 0)
        return false;
      // valError(stats, running, BRIDGE_VAL_LOCATION);
      prof.log(BRIDGE_VAL_LOCATION, running);
    }
    else if (vRef[i] == "E")
    {
      if (lOut > 0)
        return false;
      // valError(stats, running, BRIDGE_VAL_EVENT);
      prof.log(BRIDGE_VAL_EVENT, running);
    }
    else
      return false;

    j += 2;
  }
  return true;
}

