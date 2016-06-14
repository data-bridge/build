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
#include <stdio.h>
#include <stdlib.h>

#include "parse.h"

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

