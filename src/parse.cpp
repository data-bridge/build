/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <regex>
#include <stdio.h>
#include <stdlib.h>

#include "parse.h"
#include "portab.h"

using namespace std;



// tokenize splits a string into tokens separated by delimiter.
// http://stackoverflow.com/questions/236129/split-a-string-in-c

void tokenize(
  const string& str, 
  vector<string>& tokens,
  const string& delimiters)
{
  string::size_type pos, lastPos = 0;

  while (true)
  {
    pos = str.find_first_of(delimiters, lastPos);
    if (pos == std::string::npos)
    {
      pos = str.length();
      tokens.push_back(string(str.data()+lastPos, 
        static_cast<string::size_type>(pos - lastPos)));
      break;
    }
    else
    {
      tokens.push_back(string(str.data()+lastPos,
        static_cast<string::size_type>(pos - lastPos)));
    }
    lastPos = pos + 1;
   }
}


unsigned countDelimiters(
  const string& str,
  const string& delimiters)
{
  unsigned c = 0;
  for (unsigned i = 0; i < delimiters.length(); i++)
    c += count(str.begin(), str.end(), delimiters.at(i));
  return c;
}


string RevStr(const string& s)
{
  string t = s;
  reverse(t.begin(), t.end());
  return t;
}


// getWords splits on one or more whitespaces

bool getWords(
  const string& str,
  string words[],
  const int maxCount,
  unsigned& actualCount)
{
  regex whitespace("(\\S+)");
  auto wordsBegin = sregex_iterator(str.begin(), str.end(), whitespace);
  auto wordsEnd = sregex_iterator();

  if (distance(wordsBegin, wordsEnd) > maxCount)
    return false;

  actualCount = 0;
  for (sregex_iterator it = wordsBegin; it != wordsEnd; it++)
    words[actualCount++] = it->str();

  return true;
}


bool StringToUnsigned(
  const string& s,
  unsigned& res)
{
  unsigned u;
  try
  {
    u = stoul(s, nullptr, 0);
  }
  catch (const invalid_argument& ia)
  {
    UNUSED(ia);
    return false;
  }
  catch (const out_of_range& ia)
  {
    UNUSED(ia);
    return false;
  }

  if (u < 1)
    return false;

  res = u;
  return true;
}


bool StringToInt(
  const string& s,
  int& res)
{
  int i;
  try
  {
    i = stoi(s, nullptr, 0);
  }
  catch (const invalid_argument& ia)
  {
    UNUSED(ia);
    return false;
  }
  catch (const out_of_range& ia)
  {
    UNUSED(ia);
    return false;
  }

  res = i;
  return true;
}


bool StringToFloat(
  const string& s,
  float& res)
{
  float f;
  try
  {
    f = stof(s, nullptr);
  }
  catch (const invalid_argument& ia)
  {
    UNUSED(ia);
    return false;
  }
  catch (const out_of_range& ia)
  {
    UNUSED(ia);
    return false;
  }

  res = f;
  return true;
}

