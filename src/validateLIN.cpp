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
#include "Bexcept.h"


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
  const ValState& valState,
  ValProfile& prof)
{
  UNUSED(running);

  vector<string> vOut(9), vRef(9);
  // if (! LINtoList(running.out.line, vOut, "vg|", 8))
  if (! LINtoList(valState.dataOut.line, vOut, "vg|", 8))
    return false;
  // if (! LINtoList(running.ref.line, vRef, "vg|", 8))
  if (! LINtoList(valState.dataRef.line, vRef, "vg|", 8))
    return false;

  if (vOut[0] != vRef[0])
    prof.log(BRIDGE_VAL_TITLE, valState);

  if (vOut[1] != vRef[1])
    prof.log(BRIDGE_VAL_SESSION, valState);

  if (vOut[2] != vRef[2])
    prof.log(BRIDGE_VAL_SCORING, valState);

  if (vOut[3] != vRef[4] || vOut[4] != vRef[4])
    prof.log(BRIDGE_VAL_SCORING, valState);

  if (vOut[5] != vRef[5] || vOut[6] != vRef[6] ||
      vOut[7] != vRef[7] || vOut[8] != vRef[8])
    prof.log(BRIDGE_VAL_TEAMS, valState);

  return true;
}


static bool isLINPlayerLine(
  const ValExample& running, 
  const ValState& valState,
  ValProfile& prof)
{
  UNUSED(running);

  vector<string> vOut(8), vRef(8);
  // if (! LINtoList(running.out.line, vOut, "pn|", 7))
  if (! LINtoList(valState.dataOut.line, vOut, "pn|", 7))
    return false;
  // if (! LINtoList(running.ref.line, vRef, "pn|", 7))
  if (! LINtoList(valState.dataRef.line, vRef, "pn|", 7))
    return false;

  for (unsigned i = 0; i < 8; i++)
  {
    if (vOut[i] == vRef[i])
      continue;

    const unsigned lOut = vOut[i].length();
    if (lOut >= vRef[i].length() ||
        vRef[i].substr(0, lOut) != vOut[i])
    {
      prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
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


static bool isLINPlaySubstr(
  const ValExample& running,
  const ValState& valState)
{
  UNUSED(running);

  // const unsigned lOut = running.out.line.length();
  // const unsigned lRef = running.ref.line.length();
  const unsigned lOut = valState.dataOut.len;
  const unsigned lRef = valState.dataRef.len;

  size_t poso = 3;
  // find!
  // while (poso < lRef && running.out.line.at(poso) != '|')
  while (poso < lRef && valState.dataOut.line.at(poso) != '|')
    poso++;

  size_t posr = 3;
  // while (posr < lRef && running.ref.line.at(posr) != '|')
  while (posr < lRef && valState.dataRef.line.at(posr) != '|')
    posr++;

  if (poso >= posr)
    return false;

  // const string os = running.out.line.substr(3, poso);
  // const string rs = running.ref.line.substr(3, poso);
  const string os = valState.dataOut.line.substr(3, poso);
  const string rs = valState.dataRef.line.substr(3, poso);

  return (os == rs);
}


bool validateLIN_RP(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  ValState& valState,
  ValProfile& prof)
{
  string expectLine;

  // if (isLINLongRefLine(running.out.line, running.ref.line, expectLine))
  if (isLINLongRefLine(valState.dataOut.line,
      valState.dataRef.line, expectLine))
  {
    if (! valProgress(fostr, running.out))
    {
      prof.log(BRIDGE_VAL_OUT_SHORT, running);
      if (valState.bufferOut.next(valState.dataOut))
        THROW("bufferOut ends too late");
      return false;
    }

    if (! valState.bufferOut.next(valState.dataOut))
      THROW("bufferOut ends too soon");
    if (running.out.line != valState.dataOut.line)
      THROW("Out lines differ");

    // if (running.out.line == expectLine)
    if (valState.dataOut.line == expectLine)
    {
      // No newline when no play (Pavlicek error).
      prof.log(BRIDGE_VAL_LIN_PLAY_NL, valState);
      return true;
    }
  }
  // else if (isLINLongOutLine(running.out.line, running.ref.line, 
      // expectLine))
  else if (isLINLongOutLine(valState.dataOut.line, valState.dataRef.line,
      expectLine))
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

    // if (running.ref.line == expectLine)
    if (valState.dataRef.line == expectLine)
    {
      // No newline when no play (Pavlicek error).
      prof.log(BRIDGE_VAL_LIN_PLAY_NL, valState);
      return true;
    }
  }
  else if (isLINHeaderLine(running, valState, prof))
    return true;
  else if (isLINPlayerLine(running, valState, prof))
    return true;
  // else if (isLINPlay(running.ref.line))
  else if (isLINPlay(valState.dataRef.line))
  {
    // if (isLINPlay(running.out.line))
    if (isLINPlay(valState.dataOut.line))
    {
      if (isLINPlaySubstr(running, valState))
      {
        prof.log(BRIDGE_VAL_ERROR, valState);
        return false;
      }
      else
      {
        prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
        return true;
      }
    }
    else
    {
      do
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
      }
      // while (isLINPlay(running.ref.line));
      while (isLINPlay(valState.dataRef.line));

      // if (running.out.line == running.ref.line)
      if (valState.dataOut.line == valState.dataRef.line)
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

  return false;
}

