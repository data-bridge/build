/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_PARSE_H
#define BRIDGE_PARSE_H

#include <string>
#include <vector>
#include "bconst.h"

using namespace std;

void tokenize(
  const string& str,
  vector<string>& tokens,
  const string& delimiters);

unsigned countDelimiters(
  const string& str,
  const string& delimiters);

void splitIntoWords(
  const string& line,
  vector<string>& words);

string RevStr(const string& s);

bool getWords(
  const string& str,
  string words[],
  const int maxCount,
  unsigned& actualCount);

bool StringToNonzeroUnsigned(
  const string& s,
  unsigned& res);

bool str2upos(
  const string& s,
  unsigned& res);

bool StringToUnsigned(
  const string& s,
  unsigned& res);

bool str2unsigned(
  const string& s,
  unsigned& res);

bool str2int(
  const string& s,
  int& res);

bool str2float(
  const string& s,
  float& res);

unsigned StringToMonth(const string& m);

unsigned str2month(const string& m);

Player str2player(const string& text);

Vul str2vul(const string& text);

unsigned GobbleLeadingSpace(string& s);

string trimTrailing(const string& str);

bool GetNextWord(
  string& s,
  string& word);

bool getNextWord(
  string& text,
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

bool ReadAllWords(
  const string& s,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word);

bool ReadAllWordsOverlong(
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

void str2lines(
  const string& sin,
  vector<string>& sout);

string GuessOriginalLine(
  const string& fname,
  const unsigned count);

void toUpper(
  string& s);

Format ext2format(const string& s);

#endif
