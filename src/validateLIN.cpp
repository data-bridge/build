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
#include "validateLIN.h"
#include "parse.h"


static bool isLINLongRefLine(
  const string& lineOut, 
  const string& lineRef, 
  string& expectLine)
{
  const unsigned lOut = lineOut.length();
  const unsigned lRef = lineRef.length();

  if (lOut >= lRef)
    return false;

  const string sOut = lineOut.substr(0, 3);
  const string sRef = lineRef.substr(0, 3);

  if ((sOut != "mb|" && sOut != "pf|") || sOut != sRef)
    return false;

  if (lineRef.substr(0, lOut) == lineOut)
  {
    expectLine = lineRef.substr(lOut);
    return true;
  }

  // Can lop off "pg||" at end of lineOut and try again.
  if (lineOut.substr(lOut-4) != "pg||")
    return false;

  if (lineOut.substr(0, lOut-4) == lineRef.substr(0, lOut-4))
  {
    expectLine = lineRef.substr(lOut-4);
    return true;
  }
  else
    return false;
}


// TODO: Surely we can combine these two functions

static bool isLINLongOutLine(
  const string& lineOut, 
  const string& lineRef, 
  string& expectLine)
{
  const unsigned lOut = lineOut.length();
  const unsigned lRef = lineRef.length();

  if (lRef >= lOut)
    return false;

  const string sOut = lineOut.substr(0, 3);
  const string sRef = lineRef.substr(0, 3);

  if ((sOut != "mb|" && sOut != "pf|") || sOut != sRef)
    return false;

  if (lineOut.substr(0, lRef) == lineRef)
  {
    expectLine = lineOut.substr(lRef);
    return true;
  }

  // Can lop off "pg||" at end of lineRef and try again.
  if (lineRef.substr(lRef-4) != "pg||")
    return false;

  if (lineRef.substr(0, lRef-4) == lineOut.substr(0, lRef-4))
  {
    expectLine = lineOut.substr(lRef-4);
    return true;
  }
  else
    return false;
}


static bool LINtoList(
  const string& line,
  vector<string>& list,
  const string& tag,
  const int numFields)
{
  if (line.length() < 5 || line.substr(0, 3) != tag)
    return false;

  string piece = line.substr(3);
  piece = piece.substr(0, piece.find("|"));

  // A LIN vg line must have exactly 8 commas.
  // For a pn line it is 7.
  if (count(piece.begin(), piece.end(), ',') != numFields)
    return false;

  list.clear();
  tokenize(piece, list, ",");
  return true;
}


static bool isLINHeaderLine(
  const ValExample& running, 
  ValFileStats& stats)
{
  vector<string> vOut(9), vRef(9);
  if (! LINtoList(running.out.line, vOut, "vg|", 8))
    return false;
  if (! LINtoList(running.ref.line, vRef, "vg|", 8))
    return false;

  if (vOut[0] != vRef[0])
    valError(stats, running, BRIDGE_VAL_TITLE);

  if (vOut[1] != vRef[1])
    valError(stats, running, BRIDGE_VAL_SESSION);

  if (vOut[2] != vRef[2])
    valError(stats, running, BRIDGE_VAL_SCORING);

  if (vOut[3] != vRef[4] || vOut[4] != vRef[4])
    valError(stats, running, BRIDGE_VAL_SCORING);

  if (vOut[5] != vRef[5] || vOut[6] != vRef[6] ||
      vOut[7] != vRef[7] || vOut[8] != vRef[8])
    valError(stats, running, BRIDGE_VAL_TEAMS);

  return true;
}


static bool isLINPlayerLine(
  const ValExample& running, 
  ValFileStats& stats)
{
  vector<string> vOut(8), vRef(8);
  if (! LINtoList(running.out.line, vOut, "pn|", 7))
    return false;
  if (! LINtoList(running.ref.line, vRef, "pn|", 7))
    return false;

  for (unsigned i = 0; i < 8; i++)
  {
    if (vOut[i] == vRef[i])
      continue;

    const unsigned lOut = vOut[i].length();
    if (lOut >= vRef[i].length() ||
        vRef[i].substr(0, lOut) != vOut[i])
    {
      valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
      return false;
    }
  }
  return true;
}


static bool isLINPlay(const string& line)
{
  regex re("^pc\\|\\w+\\|pg\\|\\|$");
  smatch match;
  return regex_search(line, match, re);
}


bool validateLIN_RP(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  ValFileStats& stats)
{
  string expectLine;
  if (isLINLongRefLine(running.out.line, running.ref.line, expectLine))
  {
    if (! valProgress(fostr, running.out))
    {
      stats.counts[BRIDGE_VAL_OUT_SHORT]++;
      return false;
    }

    if (running.out.line == expectLine)
    {
      // No newline when no play (Pavlicek error).
      valError(stats, running, BRIDGE_VAL_LIN_PLAY_NL);
      return true;
    }
  }
  else if (isLINLongOutLine(running.out.line, running.ref.line, 
      expectLine))
  {
    if (! valProgress(frstr, running.ref))
    {
      stats.counts[BRIDGE_VAL_REF_SHORT]++;
      return false;
    }

    if (running.ref.line == expectLine)
    {
      // No newline when no play (Pavlicek error).
      valError(stats, running, BRIDGE_VAL_LIN_PLAY_NL);
      return true;
    }
  }
  else if (isLINHeaderLine(running, stats))
    return true;
  else if (isLINPlayerLine(running, stats))
    return true;
  else if (isLINPlay(running.ref.line) && ! isLINPlay(running.out.line))
  {
    do
    {
      if (! valProgress(frstr, running.ref))
      {
        stats.counts[BRIDGE_VAL_REF_SHORT]++;
        return false;
      }
    }
    while (isLINPlay(running.ref.line));

    if (running.out.line == running.ref.line)
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

  return false;
}

