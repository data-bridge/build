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
  const string& text,
  vector<string>& tokens,
  const string& delimiters);

unsigned countDelimiters(
  const string& text,
  const string& delimiters);

void splitIntoWords(
  const string& text,
  vector<string>& words);

bool getWords(
  const string& text,
  string words[],
  const int maxCount,
  unsigned& actualCount);

bool str2unsigned(
  const string& text,
  unsigned& res);

bool str2upos(
  const string& text,
  unsigned& res);

bool str2int(
  const string& text,
  int& res);

bool str2float(
  const string& text,
  float& res);

unsigned str2month(const string& text);

Player str2player(const string& text);

Vul str2vul(const string& text);

void str2lines(
  const string& sin,
  vector<string>& sout);

string chars2str(
  char * buffer,
  unsigned buflen);

bool char2player(
  const char c,
  Player& p);

Format ext2format(const string& s);

string changeExt(
  const string& fname,
  const string& newExt);

void appendFile(
  const string& fname,
  const unsigned lineno,
  const string& command,
  const string& text);

void toUpper(string& text);

string basefile(const string& path);

unsigned trimLeading(string& text);

string trimTrailing(const string& str);

bool getNextWord(
  string& text,
  string& word);

bool readNextWord(
  const string& text,
  const unsigned startPos,
  string& word);

bool readNextWord(
  const string& text,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word);

bool readAllWords(
  const string& text,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word);

bool readAllWordsOverlong(
  const string& text,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word);

bool readNextSpacedWord(
  const string& text,
  const unsigned startPos,
  string& word);

bool readLastWord(
  const string& text,
  string& word);

string guessOriginalLine(
  const string& fname,
  const unsigned count);

#endif
