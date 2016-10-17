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
#include "validateTXT.h"
#include "parse.h"


static bool isTXTAllPass(
  const string& lineOut,
  const string& lineRef,
  unsigned& expectPasses)
{
  vector<string> wordsRef;
  splitIntoWords(lineRef, wordsRef);
  unsigned lRef = wordsRef.size();
  if (lRef > 4)
    return false;

  vector<string> wordsOut;
  splitIntoWords(lineOut, wordsOut);
  unsigned lOut = wordsOut.size();
  if (lOut < 2 || lOut+2 < lRef || lOut > lRef+1)
    return false;

  if (wordsOut[lOut-2] != "All" || wordsOut[lOut-1] != "Pass")
    return false;

  for (unsigned i = lOut-2; i < lRef; i++)
    if (wordsRef[i] != "Pass")
      return false;

  for (unsigned i = 0; i < lOut-2; i++)
  {
    if (wordsRef[i] != wordsOut[i])
      return false;
  }

  if (lOut == 2 && lRef == 4)
  {
    expectPasses = 0;
    return true;
  }

  unsigned pos = 0;
  while (pos < lineOut.length() && lineOut.at(pos) == ' ')
    pos++;
  if (pos == lineOut.length())
    return false;

  expectPasses = lOut + 1 - lRef;
  if (pos > 0 && lineOut.at(pos) == 'A')
    expectPasses++;

  return true;
}


static bool isTXTPasses(
  const string& lineOut,
  const unsigned expectPasses)
{
  vector<string> wordsOut;
  splitIntoWords(lineOut, wordsOut);
  if (wordsOut.size() != expectPasses)
    return false;

  for (unsigned i = 0; i < expectPasses; i++)
  {
    if (wordsOut[i] != "Pass")
      return false;
  }
  return true;
}


static bool isTXTPlay(const string& line)
{
  if (line.length() < 4)
    return false;

  const char c0 = line.at(0);
  const char c1 = line.at(1);
  const char c2 = line.at(2);
  const char c3 = line.at(3);

  if (c0 < '0' || c0 > '9')
    return false;

  if (c1 == '.' && c2 == ' ')
    return true;

  if (c0 == '1' && c1 >= '0' && c1 <= '3' && c2 == '.' && c3 == ' ')
    return true;

  return false;
}

static bool isTXTRunningScore(const string& line)
{
  vector<string> words;
  splitIntoWords(line, words);
  if (words.size() != 4)
    return false;

  int i;
  if (! StringToInt(words[1], i))
    return false;
  if (! StringToInt(words[3], i))
    return false;
  return true;
}


static bool isTXTResult(const string& line)
{
  if (line.length() < 4)
    return false;

  const string w = line.substr(0, 4);
  return (w == "Down" || w == "Made" || 
    (line.length() >= 6 && line.substr(0, 6) == "Passed"));
}


static bool areTXTSimilarResults(
   const string& lineOut,
   const string& lineRef)
{
  vector<string> wordsOut, wordsRef;
  splitIntoWords(lineOut, wordsOut);
  splitIntoWords(lineRef, wordsRef);

  const unsigned lOut = wordsOut.size();
  const unsigned lRef = wordsRef.size();

  if (lOut+1 != lRef || lOut < 2)
    return false;

  for (unsigned i = 0; i < lOut-2; i++)
  {
    if (wordsOut[i] != wordsRef[i])
      return false;
  }

  for (unsigned i = lOut-2; i < lOut; i++)
  {
    if (wordsOut[i] != wordsRef[i+1])
      return false;
  }

  return true;
}


bool validateTXT(
  ifstream& frstr,
  ValExample& running,
  unsigned& emptyState,
  ValFileStats& stats)
{
  // emptyState is a bit of a kludge to attempt to detect the empty
  // lines (as there is no contextual information in a TXT file).
  // It will mis-allocate if only some of the header lines are missing.

  unsigned expectPasses;
  if (isTXTAllPass(running.out.line, running.ref.line, expectPasses))
  {
    // Reference does not have "All Pass" (Pavlicek error).
    if (expectPasses > 0 && ! valProgress(frstr, running.ref))
    {
      stats.counts[BRIDGE_VAL_OUT_SHORT]++;
      return false;
    }

    if (expectPasses == 0 || isTXTPasses(running.ref.line, expectPasses))
    {
      valError(stats, running, BRIDGE_VAL_TXT_ALL_PASS);
      return true;
    }
    else
      return false;
  }
  else if (running.out.line == "" &&
      running.ref.line.length() == 41 &&
      running.ref.line.substr(0, 5) == "-----")
  {
    if (! valProgress(frstr, running.ref))
    {
      stats.counts[BRIDGE_VAL_REF_SHORT]++;
      return false;
    }

    if (running.ref.line != "")
      return false;

    valError(stats, running, BRIDGE_VAL_TXT_DASHES);
    return true;
  }

  while (isTXTPlay(running.ref.line) && ! isTXTPlay(running.out.line))
  {
    if (! valProgress(frstr, running.ref))
    {
      stats.counts[BRIDGE_VAL_REF_SHORT]++;
      return false;
    }
  }

  if (frstr.eof())
  {
    stats.counts[BRIDGE_VAL_REF_SHORT]++;
    return false;
  }

  if (running.out.line == running.ref.line)
    return true;

  if (isTXTResult(running.out.line) &&
      isTXTResult(running.ref.line) &&
      areTXTSimilarResults(running.out.line, running.ref.line))
  {
    valError(stats, running, BRIDGE_VAL_TXT_RESULT);
    return true;
  }

  if (running.out.line == "")
  {
    if (isTXTRunningScore(running.ref.line))
    {
      valError(stats, running, BRIDGE_VAL_TEAMS);
      return true;
    }
    else if (emptyState == 0)
    {
      valError(stats, running, BRIDGE_VAL_TITLE);
      emptyState++;
      return true;
    }
    else if (emptyState == 1)
    {
      valError(stats, running, BRIDGE_VAL_DATE);
      emptyState++;
      return true;
    }
    else if (emptyState == 2)
    {
      valError(stats, running, BRIDGE_VAL_LOCATION);
      emptyState++;
      return true;
    }
    else if (emptyState == 3)
    {
      valError(stats, running, BRIDGE_VAL_EVENT);
      emptyState++;
      return true;
    }
    else if (emptyState == 4)
    {
      valError(stats, running, BRIDGE_VAL_SESSION);
      emptyState++;
      return true;
    }
    else if (emptyState == 5)
    {
      valError(stats, running, BRIDGE_VAL_TEAMS);
      emptyState++;
      return true;
    }
  }

  return false;
}

