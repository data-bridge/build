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

bool isTXTHeader(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running, 
  const unsigned & headerStartTXT, 
  ValProfile& prof);


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
  const unsigned n = words.size();
  if (n < 4)
    return false;

  int i;
  if (! StringToInt(words[n-1], i))
    return false;

  for (unsigned j = 1; j < n-2; j++)
    if (StringToInt(words[j], i))
      return true;

  return false;
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

  if (lOut >= lRef || lOut < 2)
    return false;

  for (unsigned i = 0; i < lOut-2; i++)
  {
    if (wordsOut[i] != wordsRef[i])
      return false;
  }

  const unsigned d = lRef-lOut;
  for (unsigned i = lOut-2; i < lOut; i++)
  {
    if (wordsOut[i] != wordsRef[i+d])
      return false;
  }

  return true;
}


static const vector<ValError> headerErrorType =
{
  BRIDGE_VAL_TITLE, 
  BRIDGE_VAL_ERROR, // Unused
  BRIDGE_VAL_DATE, 
  BRIDGE_VAL_LOCATION, 
  BRIDGE_VAL_EVENT, 
  BRIDGE_VAL_SESSION, 
  BRIDGE_VAL_TEAMS
};


bool isTXTHeader(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running, 
  const unsigned & headerStartTXT, 
  ValProfile& prof)
  // ValFileStats& stats)
{
  // Make a list of output header lines (absolute position known).
  vector<string> listOut(7);
  listOut.clear();

  for (unsigned i = running.out.lno; i <= headerStartTXT+6; i++)
  {
    // +0 is empty by construction, +1 must be empty.
    if (i <= headerStartTXT+1)
      continue;
    listOut[i-headerStartTXT] = running.out.line;

    if (! valProgress(fostr, running.out))
    {
      // valError(stats, running, BRIDGE_VAL_OUT_SHORT);
      prof.log(BRIDGE_VAL_OUT_SHORT, running);
      return false;
    }
  }

  // Reading the rest of the out header should leave us at an empty line.
  if (running.out.line != "")
  {
    // valError(stats, running, BRIDGE_VAL_ERROR);
    prof.log(BRIDGE_VAL_ERROR, running);
    return false;
  }


  // Make a list of reference header lines (relative).
  vector<string> listRef;
  listRef.clear();

  unsigned i = running.out.lno;
  while (running.ref.line != "")
  {
    listRef.push_back(running.ref.line);
    if (! valProgress(frstr, running.ref))
    {
      // valError(stats, running, BRIDGE_VAL_REF_SHORT);
      prof.log(BRIDGE_VAL_REF_SHORT, running);
      return false;
    }

    if (i == headerStartTXT)
    {
      // Expect a newline.
      if (running.ref.line != "")
      {
        // valError(stats, running, BRIDGE_VAL_ERROR);
        prof.log(BRIDGE_VAL_ERROR, running);
        return false;
      }

      if (! valProgress(frstr, running.ref))
      {
        // valError(stats, running, BRIDGE_VAL_REF_SHORT);
        prof.log(BRIDGE_VAL_REF_SHORT, running);
        return false;
      }
      i++;
    }
    i++;
  }

  // Attempt to match them up.
  unsigned r = 0;
  for (i = 0; i <= 6; i++)
  {
    if (listOut[i] == "")
      continue;
    else if (listOut[i] != listRef[r])
    {
      // valError(stats, running, headerErrorType[i]);
      prof.log(headerErrorType[i], running);
    }
    r++;
  }
  return true;
}


bool validateTXT(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  const unsigned& headerStartTXT,
  ValProfile& prof)
  // ValFileStats& stats)
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
      // valError(stats, running, BRIDGE_VAL_OUT_SHORT);
      prof.log(BRIDGE_VAL_OUT_SHORT, running);
      return false;
    }

    if (expectPasses == 0 || isTXTPasses(running.ref.line, expectPasses))
    {
      // valError(stats, running, BRIDGE_VAL_TXT_ALL_PASS);
      prof.log(BRIDGE_VAL_TXT_ALL_PASS, running);
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
      // valError(stats, running, BRIDGE_VAL_REF_SHORT);
      prof.log(BRIDGE_VAL_REF_SHORT, running);
      return false;
    }

    if (running.ref.line != "")
      return false;

    // valError(stats, running, BRIDGE_VAL_TXT_DASHES);
    prof.log(BRIDGE_VAL_TXT_DASHES, running);
    return true;
  }

  while (isTXTPlay(running.ref.line) && ! isTXTPlay(running.out.line))
  {
    if (! valProgress(frstr, running.ref))
    {
      // valError(stats, running, BRIDGE_VAL_REF_SHORT);
      prof.log(BRIDGE_VAL_REF_SHORT, running);
      return false;
    }
    // valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
    prof.log(BRIDGE_VAL_PLAY_SHORT, running);
  }

  if (frstr.eof())
  {
    // valError(stats, running, BRIDGE_VAL_REF_SHORT);
    prof.log(BRIDGE_VAL_REF_SHORT, running);
    return false;
  }

  if (running.out.line == running.ref.line)
    return true;

  if ((running.out.line.length() == 8 ||
       running.out.line.length() == 9) &&
      running.out.line.substr(0, 6) == "Lead: " &&
      running.ref.line == "Trick   Lead    2nd    3rd    4th")
  {
    // Play is missing from output, but lead is stated.
    // valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
    prof.log(BRIDGE_VAL_PLAY_SHORT, running);
    return true;
  }

  if (isTXTResult(running.out.line) &&
      isTXTResult(running.ref.line) &&
      areTXTSimilarResults(running.out.line, running.ref.line))
  {
    // valError(stats, running, BRIDGE_VAL_TXT_RESULT);
    prof.log(BRIDGE_VAL_TXT_RESULT, running);
    return true;
  }

  if (running.out.line == "")
  {
    if (isTXTRunningScore(running.ref.line))
    {
      // valError(stats, running, BRIDGE_VAL_TEAMS);
      prof.log(BRIDGE_VAL_TEAMS, running);
      return true;
    }
    else if (running.out.lno <= headerStartTXT + 6)
    {
      if (! isTXTHeader(frstr, fostr, running, headerStartTXT, prof))
        return false;
      else if (running.out.line == running.ref.line)
        return true;
      else
      {
        // valError(stats, running, BRIDGE_VAL_TEAMS);
        prof.log(BRIDGE_VAL_TEAMS, running);
        return false;
      }
    }
  }
  else if (running.out.lno > headerStartTXT+6)
  {
    // This is tricky, as both the numbers and positions of words
    // can differ.  We combine two heuristics.
    // The first one assumes the names are in-place with no shifts.
    // The second one assumes the number of words is the same.

    unsigned lo = running.out.line.length();
    unsigned lr = running.ref.line.length();
    if (lo > lr)
      return false;

    bool diffSeen = false;
    for (unsigned i = 0; ! diffSeen && i < lo; i++)
    {
      char c = running.out.line.at(i);
      if (c != ' ' && c != running.ref.line.at(i))
        diffSeen = true;
    }

    if (! diffSeen)
    {
      // valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
      prof.log(BRIDGE_VAL_NAMES_SHORT, running);
      return true;
    }

    // Second method.
    vector<string> vOut, vRef;
    vOut.clear();
    vRef.clear();
    splitIntoWords(running.out.line, vOut);
    splitIntoWords(running.ref.line, vRef);
    if (vOut.size() != vRef.size())
      return false;

    for (unsigned i = 0; i < vOut.size(); i++)
    {
      if (vOut[i] == vRef[i])
        continue;

      const unsigned lOut = vOut[i].length();
      if (lOut >= vRef[i].length() ||
          vRef[i].substr(0, lOut) != vOut[i])
      {
        return false;
      }
    }
    // valError(stats, running, BRIDGE_VAL_NAMES_SHORT);
    prof.log(BRIDGE_VAL_NAMES_SHORT, running);
    return true;
  }

  return false;
}

