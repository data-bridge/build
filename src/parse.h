/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_PARSE_H
#define BRIDGE_PARSE_H

#include <string>
#include <vector>

using namespace std;

void tokenize(
  const string& str,
  vector<string>& tokens,
  const string& delimiters);

unsigned countDelimiters(
  const string& str,
  const string& delimiters);

string RevStr(const string& s);

bool getWords(
  const string& str,
  string words[],
  const int maxCount,
  unsigned& actualCount);

bool StringToUnsigned(
  const string& s,
  unsigned& res);

bool StringToInt(
  const string& s,
  int& res);

bool StringToFloat(
  const string& s,
  float& res);

bool GetNextWord(
  string& s,
  string& word);

bool ReadNextWord(
  const string& s,
  const unsigned startPos,
  string& word);

bool ReadNextWord(
  const string& s,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word);

bool ReadNextSpacedWord(
  const string& s,
  const unsigned startPos,
  string& word);

bool ReadLastWord(
  const string& s,
  string& word);

bool ParsePlayer(
  const char c,
  playerType& p);

void ConvertMultilineToVector(
  const string& sin,
  vector<string>& sout);

string GuessOriginalLine(
  const string& fname,
  const unsigned count);

void toUpper(
  string& s);

#endif
