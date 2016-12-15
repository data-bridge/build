/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include "validateTXT.h"
#include "parse.h"


static bool isTXTAllPass(
  const ValState& valState,
  unsigned& expectPasses)
{
  vector<string> wordsRef;
  splitIntoWords(valState.dataRef.line, wordsRef);
  unsigned lRef = static_cast<unsigned>(wordsRef.size());
  if (lRef > 4)
    return false;

  vector<string> wordsOut;
  splitIntoWords(valState.dataOut.line, wordsOut);
  unsigned lOut = static_cast<unsigned>(wordsOut.size());
  if (lOut < 2 || lOut+2 < lRef || lOut > lRef+1)
    return false;

  if (wordsOut[lOut-2] != "All" || wordsOut[lOut-1] != "Pass")
    return false;

  for (unsigned i = lOut-2; i < lRef; i++)
    if (wordsRef[i] != "Pass")
      return false;

  for (unsigned i = 0; i < lOut-2; i++)
    if (wordsRef[i] != wordsOut[i])
      return false;

  if (lOut == 2 && lRef == 4)
  {
    expectPasses = 0;
    return true;
  }

  unsigned pos = 0;
  while (pos < valState.dataOut.len && 
      valState.dataOut.line.at(pos) == ' ')
    pos++;
  if (pos == valState.dataOut.len)
    return false;

  expectPasses = lOut + 1 - lRef;
  if (pos > 0 && valState.dataOut.line.at(pos) == 'A')
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
    if (wordsOut[i] != "Pass")
      return false;

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
  const unsigned n = static_cast<unsigned>(words.size());
  if (n < 4)
    return false;

  int i;
  if (! str2int(words[n-1], i))
    return false;

  for (unsigned j = 1; j < n-2; j++)
    if (str2int(words[j], i))
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

  const unsigned lOut = static_cast<unsigned>(wordsOut.size());
  const unsigned lRef = static_cast<unsigned>(wordsRef.size());

  if (lOut >= lRef || lOut < 2)
    return false;

  for (unsigned i = 0; i < lOut-2; i++)
    if (wordsOut[i] != wordsRef[i])
      return false;

  const unsigned d = lRef-lOut;
  for (unsigned i = lOut-2; i < lOut; i++)
    if (wordsOut[i] != wordsRef[i+d])
      return false;

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


static bool isTXTHeader(
  ValState& valState,
  const unsigned & headerStartOut, 
  ValProfile& prof)
{
  // The absolute position is known in the output.
  // In the reference, there are two variations.

  vector<string> listOut(7), listRef(7);
  for (unsigned i = 0; i < 7; i++)
  {
    listOut[i] = "";
    listRef[i] = "";
  }

  // Read the rest of the header, stopping at empty line before board.
  const unsigned offsetOut = valState.dataOut.no - headerStartOut;
  for (unsigned i = valState.dataOut.no; i <= headerStartOut+6; i++)
  {
    listOut[i-headerStartOut] = valState.dataOut.line;
    if (! valState.bufferOut.next(valState.dataOut))
      return false;
  }

  // Reading the rest of the out header should leave us at an empty line.
  if (valState.dataOut.type != BRIDGE_BUFFER_EMPTY)
    return false;

  const unsigned headerStartRef = valState.bufferRef.previousHeaderStart();

  // Do the same for the reference.
  for (unsigned i = valState.dataRef.no; i <= headerStartRef+6; i++)
  {
    if (i == headerStartRef+6 && 
        valState.dataRef.type == BRIDGE_BUFFER_EMPTY)
    {
      // This happens in practice when the location is missing.
      listRef.erase(listRef.begin()+6);
      listRef.insert(listRef.begin()+3, "");
    }
    else
    {
      listRef[i-headerStartRef] = valState.dataRef.line;
      if (! valState.bufferRef.next(valState.dataRef))
        return false;
    }
  }

  // Reading the rest of the ref header should leave us at an empty line.
  if (valState.dataRef.type != BRIDGE_BUFFER_EMPTY)
    return false;

  for (unsigned i = offsetOut; i < 7; i++)
  {
    if (listOut[i] != listRef[i])
      prof.log(headerErrorType[i], valState);
  }
  return true;
}


bool validateTXT(
  ValState& valState,
  ValProfile& prof)
{
  unsigned expectPasses;
  if (isTXTAllPass(valState, expectPasses))
  {
    // Reference does not have "All Pass" (Pavlicek error).
    if (expectPasses > 0 && ! valState.bufferRef.next(valState.dataRef))
      return false;

    if (expectPasses == 0 || 
        isTXTPasses(valState.dataRef.line, expectPasses))
    {
      prof.log(BRIDGE_VAL_TXT_ALL_PASS, valState); return true;
    }
    else
      return false;
  }

  if (valState.dataOut.type == BRIDGE_BUFFER_EMPTY &&
      valState.dataRef.type == BRIDGE_BUFFER_DASHES)
  {
    // Skip over dash line.
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.type != BRIDGE_BUFFER_EMPTY)
      return false;

    prof.log(BRIDGE_VAL_TXT_DASHES, valState);
    return true;
  }

  if (! isTXTPlay(valState.dataOut.line))
  {
    while (isTXTPlay(valState.dataRef.line))
    {
      if (! valState.bufferRef.next(valState.dataRef))
        return false;

      prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
    }
  }

  if (valState.dataOut.line == valState.dataRef.line)
    return true;

  if ((valState.dataOut.len == 8 ||
      valState.dataOut.len == 9) &&
      valState.dataOut.line.substr(0, 6) == "Lead: " &&
      valState.dataRef.line == "Trick   Lead    2nd    3rd    4th")
  {
    prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
    return true;
  }

  if (isTXTResult(valState.dataOut.line) &&
      isTXTResult(valState.dataRef.line) &&
      areTXTSimilarResults(valState.dataOut.line, 
        valState.dataRef.line))
  {
    prof.log(BRIDGE_VAL_TXT_RESULT, valState);
    return true;
  }

  const unsigned headerStartOut = valState.bufferOut.previousHeaderStart();

  if (valState.dataOut.type == BRIDGE_BUFFER_EMPTY)
  {
    if (isTXTRunningScore(valState.dataRef.line))
    {
      prof.log(BRIDGE_VAL_TEAMS, valState);
      return true;
    }
    else if (valState.dataOut.no <= headerStartOut+6)
    {
      return isTXTHeader(valState, headerStartOut, prof);
    }
  }

  if (valState.dataOut.no > headerStartOut+6)
  {
    // This is tricky, as both the numbers and positions of words
    // can differ.  We combine two heuristics.
    // The first one assumes the names are in-place with no shifts.
    // The second one assumes the number of words is the same.

    unsigned lo = valState.dataOut.len;
    unsigned lr = valState.dataRef.len;
    if (lo > lr)
      return false;

    bool diffSeen = false;
    for (unsigned i = 0; ! diffSeen && i < lo; i++)
    {
      char c = valState.dataOut.line.at(i);
      if (c != ' ' && c != valState.dataRef.line.at(i))
        diffSeen = true;
    }

    if (! diffSeen)
    {
      prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
      return true;
    }

    // Second method.
    vector<string> vOut, vRef;
    vOut.clear();
    vRef.clear();
    splitIntoWords(valState.dataOut.line, vOut);
    splitIntoWords(valState.dataRef.line, vRef);
    if (vOut.size() != vRef.size())
      return false;

    for (unsigned i = 0; i < vOut.size(); i++)
    {
      if (vOut[i] == vRef[i])
        continue;

      if (! firstContainsSecond(vRef[i], vOut[i]))
        return false;
    }
    prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
    return true;
  }

  return false;
}

