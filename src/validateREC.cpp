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
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  Buffer& bufferRef,
  Buffer& bufferOut,
  ValProfile& prof)
{
  LineData bref, bout;
  if (running.ref.line == "")
  {
    while (isRECPlay(running.out.line))
    {
      // Extra play line in output (not in ref!) (Pavlicek error).
      prof.log(BRIDGE_VAL_PLAY_SHORT, running);

      if (! valProgress(fostr, running.out))
      {
        prof.log(BRIDGE_VAL_OUT_SHORT, running);
        if (bufferOut.next(bout))
          THROW("bufferOut ends too late");
        return false;
      }

      if (! bufferOut.next(bout))
        THROW("bufferOut ends too soon");
      if (bout.line != running.out.line)
        THROW("Out lines differ");
    }

    if (running.out.line == running.ref.line)
      return true;
    else
    {
      prof.log(BRIDGE_VAL_ERROR, running);
      return false;
    }
  }

  if (running.out.line == "")
  {
    while (isRECPlay(running.ref.line))
    {
      // The other way round.
      prof.log(BRIDGE_VAL_PLAY_SHORT, running);

      if (! valProgress(frstr, running.ref))
      {
        prof.log(BRIDGE_VAL_REF_SHORT, running);
        if (bufferRef.next(bref))
          THROW("bufferRef ends too late");
        return false;
      }

      if (! bufferRef.next(bref))
        THROW("bufferRef ends too soon");
      if (bref.line != running.ref.line)
        THROW("Ref lines differ");
    }

    if (running.out.line == running.ref.line)
      return true;
    else
    {
      prof.log(BRIDGE_VAL_ERROR, running);
      return false;
    }
  }


  if (isRECPlay(running.out.line) && isRECPlay(running.ref.line))
  {
    unsigned lOut = running.out.line.length();
    if (lOut >= running.ref.line.length() ||
        running.ref.line.substr(0, lOut) != running.out.line)
    {
      prof.log(BRIDGE_VAL_ERROR, running);
      return false;
    }
    else
    {
      prof.log(BRIDGE_VAL_PLAY_SHORT, running);
      return true;
    }
  }
  else if (isRECJustMade(running.out.line, running.ref.line))
  {
    // "Won 32" (Pavlicek error, should be "Made 0" or so.
    prof.log(BRIDGE_VAL_REC_MADE_32, running);

    // The next line (Score, Points) is then also different.
    if (! valProgress(fostr, running.out))
    {
      prof.log(BRIDGE_VAL_OUT_SHORT, running);
      if (bufferOut.next(bout))
        THROW("bufferOut ends too late");
      return false;
    }

    if (! bufferOut.next(bout))
      THROW("bufferOut ends too soon");
    if (bout.line != running.out.line)
      THROW("Out lines differ");

    if (! valProgress(frstr, running.ref))
    {
      prof.log(BRIDGE_VAL_REF_SHORT, running);
      if (bufferRef.next(bref))
        THROW("bufferRef ends too late");
      return false;
    }

    if (! bufferRef.next(bref))
      THROW("bufferRef ends too soon");
    if (bref.line != running.ref.line)
      THROW("Ref lines differ");

    return true;
  }
  else if (isRECNorthLine(running.out.line, running.ref.line))
  {
    prof.log(BRIDGE_VAL_NAMES_SHORT, running);
    return true;
  }
  else if (isRECEWLine(running.out.line, running.ref.line))
  {
    prof.log(BRIDGE_VAL_NAMES_SHORT, running);
    return true;
  }
  else if (isRECSouthLine(running.out.line, running.ref.line))
  {
    prof.log(BRIDGE_VAL_NAMES_SHORT, running);
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

