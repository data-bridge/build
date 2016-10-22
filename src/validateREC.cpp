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


static bool isRECPlay(const string& line)
{
  vector<string> words;
  splitIntoWords(line, words);

  if (words.size() < 3 || words.size() > 6)
    return false;

  unsigned u;
  if (! StringToNonzeroUnsigned(words[0], u))
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
  if (! ReadAllWords(lineOut, 12, 23, pOut))
    return false;
  if (! ReadAllWords(lineRef, 12, 23, pRef))
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
  if (! ReadAllWords(lineOut, 0, 11, pOut))
    return false;
  if (! ReadAllWords(lineRef, 0, 11, pRef))
    return false;

  unsigned lOut = pOut.length();
  if (pRef != pOut &&
      (lOut >= pRef.length() || pRef.substr(0, lOut) != pOut))
    return false;

  if (! ReadAllWords(lineOut, 24, 35, pOut))
    return false;
  if (! ReadAllWords(lineRef, 24, 35, pRef))
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
  if (! ReadAllWords(lineOut, 12, 23, pOut))
    return false;
  if (! ReadAllWords(lineRef, 12, 23, pRef))
    return false;

  const unsigned lOut = pOut.length();
  if (lOut >= pRef.length() ||
      pRef.substr(0, lOut) != pOut)
    return false;

  return true;
}


bool validateREC(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  ValFileStats& stats)
{
  if (running.ref.line == "")
  {
    while (isRECPlay(running.out.line))
    {
      // Extra play line in output (not in ref!) (Pavlicek error).
      valError(stats, running, BRIDGE_VAL_PLAY_SHORT);

      if (! valProgress(fostr, running.out))
      {
        valError(stats, running, BRIDGE_VAL_OUT_SHORT);
        return false;
      }
    }

    if (running.out.line == running.ref.line)
      return true;
    else
    {
      valError(stats, running, BRIDGE_VAL_ERROR);
      return false;
    }
  }


  if (isRECJustMade(running.out.line, running.ref.line))
  {
    // "Won 32" (Pavlicek error, should be "Made 0" or so.
    valError(stats, running, BRIDGE_VAL_REC_MADE_32);

    // The next line (Score, Points) is then also different.
    if (! valProgress(fostr, running.out))
    {
      valError(stats, running, BRIDGE_VAL_OUT_SHORT);
      return false;
    }

    if (! valProgress(frstr, running.ref))
    {
      valError(stats, running, BRIDGE_VAL_REF_SHORT);
      return false;
    }
    return true;
  }
  else if (isRECNorthLine(running.out.line, running.ref.line))
  {
    valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
    return true;
  }
  else if (isRECEWLine(running.out.line, running.ref.line))
  {
    valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
    return true;
  }
  else if (isRECSouthLine(running.out.line, running.ref.line))
  {
    valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
    return true;
  }
  // else if (isRECPlayerLine(running.out.line, running.ref.line))
  // {
    // valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
    // return false;
  // }
  else
    return false;
}

