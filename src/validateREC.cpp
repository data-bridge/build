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
#include "validateREC.h"
#include "parse.h"
#include "Bexcept.h"


static bool isRECPlay(const string& line)
{
  vector<string> words;
  splitIntoWords(line, words);

  if (words.size() < 3 || words.size() > 6)
    return false;

  unsigned u;
  if (! str2upos(words[0], u))
    return false;
  if (u > 13)
    return false;

  for (unsigned i = 2; i < words.size(); i++)
  {
    if (words[i].length() > 3)
      return false;
  }

  // We could also check word[1] for South/East/North/West etc.

  return true;
}


static bool isRECJustMade(
  const string &lineOut, 
  const string &lineRef)
{
  unsigned lOut = lineOut.length();
  unsigned lRef = lineRef.length();

  if (lOut < 32 || lRef != lOut)
    return false;

  if (lineOut.substr(28) == "Made 0" && lineRef.substr(28) == "Won 32")
    return true;
  else
    return false;
}


static bool isRECNorthLine(
  const string &lineOut, 
  const string &lineRef)
{
  if (lineOut.length() < 30 || 
      lineRef.length() < 30 || 
      lineOut.length() != lineRef.length())
    return false;

  if (lineOut.substr(0, 3) != "IMP" || lineRef.substr(0, 3) != "IMP")
    return false;

  string pOut, pRef;
  if (! readAllWords(lineOut, 12, 23, pOut))
    return false;
  if (! readAllWords(lineRef, 12, 23, pRef))
    return false;

  const unsigned lOut = pOut.length();
  if (lOut >= pRef.length() ||
      pRef.substr(0, lOut) != pOut)
    return false;

  return true;
}


static bool isRECEWLine(
  const string &lineOut, 
  const string &lineRef)
{
  if (lineOut.length() < 25 ||
      lineRef.length() < 25 ||
      lineOut.length() > lineRef.length())
    return false;

  if (lineOut.at(12) != 'D' || lineRef.at(12) != 'D')
    return false;

  string pOut, pRef;
  if (! readAllWords(lineOut, 0, 11, pOut))
    return false;
  if (! readAllWords(lineRef, 0, 11, pRef))
    return false;

  unsigned lOut = pOut.length();
  if (pRef != pOut &&
      (lOut >= pRef.length() || pRef.substr(0, lOut) != pOut))
    return false;

  if (! readAllWords(lineOut, 24, 35, pOut))
    return false;
  if (! readAllWords(lineRef, 24, 35, pRef))
    return false;

  lOut = pOut.length();
  if (pRef != pOut &&
      (lOut >= pRef.length() || pRef.substr(0, lOut) != pOut))
    return false;

  return true;
}


static bool isRECSouthLine(
  const string &lineOut, 
  const string &lineRef)
{
  if (lineOut.length() < 25 ||
      lineRef.length() < 25 ||
      lineOut.length() != lineRef.length())
    return false;

  if (lineOut.at(0) != 'D' || lineOut.at(24) != 'D' ||
      lineRef.at(0) != 'D' || lineRef.at(24) != 'D')
    return false;

  string pOut, pRef;
  if (! readAllWords(lineOut, 12, 23, pOut))
    return false;
  if (! readAllWords(lineRef, 12, 23, pRef))
    return false;

  const unsigned lOut = pOut.length();
  if (lOut >= pRef.length() ||
      pRef.substr(0, lOut) != pOut)
    return false;

  return true;
}


bool validateREC(
  ValState& valState,
  ValProfile& prof)
{
  if (valState.dataRef.type == BRIDGE_BUFFER_EMPTY)
  {
    while (isRECPlay(valState.dataOut.line))
    {
      // Extra play line in output (not in ref!) (Pavlicek error).
      prof.log(BRIDGE_VAL_PLAY_SHORT, valState);

      if (! valState.bufferOut.next(valState.dataOut))
      {
        prof.log(BRIDGE_VAL_OUT_SHORT, valState);
        return false;
      }
    }

    if (valState.dataRef.line == valState.dataOut.line)
      return true;
    else
    {
      prof.log(BRIDGE_VAL_ERROR, valState);
      return false;
    }
  }

  if (valState.dataOut.type == BRIDGE_BUFFER_EMPTY)
  {
    while (isRECPlay(valState.dataRef.line))
    {
      // The other way round.
      prof.log(BRIDGE_VAL_PLAY_SHORT, valState);

      if (! valState.bufferRef.next(valState.dataRef))
      {
        prof.log(BRIDGE_VAL_REF_SHORT, valState);
        return false;
      }
    }

    if (valState.dataRef.line == valState.dataOut.line)
      return true;
    else
    {
      prof.log(BRIDGE_VAL_ERROR, valState);
      return false;
    }
  }


  if (isRECPlay(valState.dataOut.line) && 
      isRECPlay(valState.dataRef.line))
  {
    if (refContainsOut(valState))
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
  else if (isRECJustMade(valState.dataOut.line, valState.dataRef.line))
  {
    // "Won 32" (Pavlicek error, should be "Made 0" or so.
    prof.log(BRIDGE_VAL_REC_MADE_32, valState);

    // The next line (Score, Points) is then also different.
    if (! valState.bufferOut.next(valState.dataOut))
    {
      prof.log(BRIDGE_VAL_OUT_SHORT, valState);
      return false;
    }

    if (! valState.bufferRef.next(valState.dataRef))
    {
      prof.log(BRIDGE_VAL_REF_SHORT, valState);
      return false;
    }

    return true;
  }
  else if (isRECNorthLine(valState.dataOut.line, valState.dataRef.line))
  {
    prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
    return true;
  }
  else if (isRECEWLine(valState.dataOut.line, valState.dataRef.line))
  {
    prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
    return true;
  }
  else if (isRECSouthLine(valState.dataOut.line, valState.dataRef.line))
  {
    prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
    return true;
  }
  else
    return false;
}

