/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include "validate.h"
#include "valint.h"
#include "validateREC.h"
#include "ValProfile.h"

#include "../util/parse.h"

#define PLOG(x) prof.log(x, valState.dataOut, valState.dataRef)


static bool isRECPlay(const string& line)
{
  vector<string> words;
  splitIntoWords(line, words);

  if (words.size() < 3 || words.size() > 6)
    return false;

  unsigned u;
  if (! str2upos(words[0], u) || u > 13)
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
  unsigned lOut = static_cast<unsigned>(lineOut.length());
  unsigned lRef = static_cast<unsigned>(lineRef.length());

  if (lOut < 32 || lRef != lOut)
    return false;

  if (lineOut.substr(28) == "Made 0" && lineRef.substr(28) == "Won 32")
    return true;
  else
    return false;
}


static bool isRECPlayer(
  const string& lineOut, 
  const string& lineRef,
  const unsigned startPos,
  const unsigned endPosIncl)
{
  string pOut, pRef;
  if (! readAllWords(lineOut, startPos, endPosIncl, pOut))
    return false;
  if (! readAllWords(lineRef, startPos, endPosIncl, pRef))
    return false;

  return (pRef == pOut || firstContainsSecond(pRef, pOut));
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

  return isRECPlayer(lineOut, lineRef, 12, 23);
}


static bool isRECEWLine(
  const string &lineOut, 
  const string &lineRef)
{
  if (lineOut.at(12) != 'D' || lineRef.at(12) != 'D')
    return false;

  if (! isRECPlayer(lineOut, lineRef, 0, 11))
    return false;
  return isRECPlayer(lineOut, lineRef, 24, 35);
}


static bool isRECSouthLine(
  const string &lineOut, 
  const string &lineRef)
{
  if (lineOut.length() != lineRef.length())
    return false;

  if (lineOut.at(0) != 'D' || lineOut.at(24) != 'D' ||
      lineRef.at(0) != 'D' || lineRef.at(24) != 'D')
    return false;

  return isRECPlayer(lineOut, lineRef, 12, 23);
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
      PLOG(BRIDGE_VAL_PLAY_SHORT);

      if (! valState.bufferOut.next(valState.dataOut))
        return false;
    }
    return (valState.dataRef.line == valState.dataOut.line);
  }

  if (valState.dataOut.type == BRIDGE_BUFFER_EMPTY)
  {
    while (isRECPlay(valState.dataRef.line))
    {
      // The other way round.
      PLOG(BRIDGE_VAL_PLAY_SHORT);

      if (! valState.bufferRef.next(valState.dataRef))
        return false;
    }

    return (valState.dataRef.line == valState.dataOut.line);
  }


  if (isRECPlay(valState.dataOut.line) && 
      isRECPlay(valState.dataRef.line))
  {
    if (refContainsOut(valState.dataOut, valState.dataRef))
    {
      PLOG(BRIDGE_VAL_PLAY_SHORT);
      return true;
    }
    else
      return false;
  }

  if (isRECJustMade(valState.dataOut.line, valState.dataRef.line))
  {
    // "Won 32" (Pavlicek error, should be "Made 0" or so.
    PLOG(BRIDGE_VAL_REC_MADE_32);

    // The next line (Score, Points) is then also different.
    if (! valState.bufferOut.next(valState.dataOut) ||
        ! valState.bufferRef.next(valState.dataRef))
      return false;
    else
      return true;
  }

  if (valState.dataOut.len < 25 ||
      valState.dataRef.len < 25 ||
      valState.dataOut.len > valState.dataRef.len)
    return false;

  if (isRECNorthLine(valState.dataOut.line, valState.dataRef.line))
  {
    PLOG(BRIDGE_VAL_NAMES_SHORT);
    return true;
  }

  if (isRECEWLine(valState.dataOut.line, valState.dataRef.line))
  {
    PLOG(BRIDGE_VAL_NAMES_SHORT);
    return true;
  }

  if (isRECSouthLine(valState.dataOut.line, valState.dataRef.line))
  {
    PLOG(BRIDGE_VAL_NAMES_SHORT);
    return true;
  }

  return false;
}

