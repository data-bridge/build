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

#endif
