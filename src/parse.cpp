/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <regex>
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
  int c = 0;
  for (unsigned i = 0; i < delimiters.length(); i++)
    c += count(str.begin(), str.end(), delimiters.at(i));
  return static_cast<unsigned>(c);
}


void splitIntoWords(
  const string& line,
  vector<string>& words)
{
  // Split into words (split on \s+, effectively).
  unsigned pos = 0;
  unsigned startPos = 0;
  bool isSpace = true;
  const unsigned l = line.length();

  while (pos < l)
  {
    if (line.at(pos) == ' ')
    {
      if (! isSpace)
      {
        words.push_back(line.substr(startPos, pos-startPos));
        isSpace = true;
      }
    }
    else if (isSpace)
    {
      isSpace = false;
      startPos = pos;
    }
    pos++;
  }

  if (! isSpace)
    words.push_back(line.substr(startPos, pos-startPos));
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

  res = u;
  return true;
}


bool str2unsigned(
  const string& s,
  unsigned& res)
{
  return StringToUnsigned(s, res);
}


bool StringToNonzeroUnsigned(
  const string& s,
  unsigned& res)
{
  unsigned u;
  try
  {
    u = stoul(s, nullptr, 10);
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


bool str2upos(
  const string& s,
  unsigned& res)
{
  return StringToNonzeroUnsigned(s, res);
}


bool str2int(
  const string& s,
  int& res)
{
  int i;
  size_t pos;
  try
  {
    i = stoi(s, &pos);
    if (pos != s.size())
      return false;

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


bool str2float(
  const string& s,
  float& res)
{
  float f;
  size_t pos;
  try
  {
    f = static_cast<float>(stod(s, &pos));
    if (pos != s.size())
      return false;
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


unsigned StringToMonth(const string& m)
{
  string s = m;
  toUpper(s);

  if (s == "JANUARY")
    return 1;
  else if (s == "FEBRUARY")
    return 2;
  else if (s == "MARCH")
    return 3;
  else if (s == "APRIL")
    return 4;
  else if (s == "MAY")
    return 5;
  else if (s == "JUNE")
    return 6;
  else if (s == "JULY")
    return 7;
  else if (s == "AUGUST")
    return 8;
  else if (s == "SEPTEMBER")
    return 9;
  else if (s == "OCTOBER")
    return 10;
  else if (s == "NOVEMBER")
    return 11;
  else if (s == "DECEMBER")
    return 12;
  else
    return 0;
}


unsigned str2month(const string& m)
{
  return StringToMonth(m);
}


Player str2player(const string& text)
{
  // This function is "too permissive" as it doesn't limit the format...
  if (text == "N" || text == "North")
    return BRIDGE_NORTH;
  else if (text == "E" || text == "East")
    return BRIDGE_EAST;
  else if (text == "S" || text == "South")
    return BRIDGE_SOUTH;
  else if (text == "W" || text == "West")
    return BRIDGE_WEST;
  else
    return BRIDGE_PLAYER_SIZE;
}


Vul str2vul(const string& text)
{
  // This function is "too permissive" as it doesn't limit the format...
  // Pavlicek uses "0" -- wrong?
  if (text == "o" || text == "O" || text == "0" || 
      text == "None" || text == "Z")
    return BRIDGE_VUL_NONE;
  else if (text == "e" || text == "E" || text == "EW" || 
       text == "E" || text == "E-W")
    return BRIDGE_VUL_EAST_WEST;
  else if (text == "n" || text == "N" || text == "NS" || 
      text == "N-S" || text == "N")
    return BRIDGE_VUL_NORTH_SOUTH;
  else if (text == "b" || text == "B" || text == "All" || text == "Both")
    return BRIDGE_VUL_BOTH;
  else
    return BRIDGE_VUL_SIZE;
}


unsigned GobbleLeadingSpace(string& s)
{
  unsigned pos = 0;
  unsigned l = s.length();
  while (pos < l && s.at(pos) == ' ')
    pos++;

  if (pos > 0)
    s.erase(0, pos);
  return pos;
}


string trimTrailing(const string& str)
{
  unsigned pos = str.length();
  while (pos >= 1 && str.at(pos-1) == ' ')
    pos--;

  if (pos == 0)
    return "";
  else
    return str.substr(0, pos);
}


bool GetNextWord(
  string& s,
  string& word)
{
  // Consumes from s!
  unsigned pos = 0;
  unsigned l = s.length();
  while (pos < l && s.at(pos) != ' ')
    pos++;
  
  if (pos == 0)
    return false;
  else if (pos == l)
  {
    word = s;
    s = "";
    return true;
  }
  else
  {
    word = s.substr(0, pos);
    s.erase(0, pos+1);
    return true;
  }
}


bool getNextWord(
  string& text,
  string& word)
{
  return GetNextWord(text, word);
}


bool ReadNextWord(
  const string& s,
  const unsigned startPos,
  string& word)
{
  unsigned l = s.length();
  unsigned pos = startPos;
  while (pos < l && s.at(pos) != ' ')
    pos++;
  
  if (pos == startPos)
    return false;
  else if (pos == l)
  {
    word = s.substr(startPos, pos-startPos);
    return true;
  }
  else
  {
    word = s.substr(startPos, pos-startPos);
    return true;
  }
}


bool ReadNextWord(
  const string& s,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word)
{
  unsigned l = s.length();
  unsigned pos = startPos;
  while (pos < l && pos <= stopPosInclusive && s.at(pos) != ' ')
    pos++;
  
  if (pos == startPos)
    return false;
  else if (pos == l)
  {
    word = s.substr(startPos, pos-startPos);
    return true;
  }
  else
  {
    word = s.substr(startPos, pos-startPos);
    return true;
  }
}


bool ReadAllWords(
  const string& s,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word)
{
  unsigned l = s.length();
  if (l == 0 || l < startPos || s.at(startPos) == ' ')
    return false;

  unsigned pos = Min(l-1, stopPosInclusive);
  while (pos > startPos && s.at(pos) == ' ')
    pos--;
  
  if (s.at(pos) == ' ')
    return false;
  else
  {
    word = s.substr(startPos, pos+1-startPos);
    return true;
  }
}


bool ReadAllWordsOverlong(
  const string& s,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word)
{
  unsigned l = s.length();
  if (l == 0 || l < startPos)
    return false;

  // Skip over leading spaces.
  unsigned spos = startPos;
  while (spos < l && s.at(spos) == ' ')
    spos++;
  if (spos == l)
    return false;

  // Spill over end until end of a word is reached.
  unsigned pos = Min(l-1, stopPosInclusive);
  while (pos < l && s.at(pos) != ' ')
    pos++;

  if (pos == l)
    pos--;
  else
  {
    while (pos > spos && s.at(pos) == ' ')
      pos--;
  }
  
  word = s.substr(spos, pos+1-spos);
  return true;
}


bool ReadNextSpacedWord(
  const string& s,
  const unsigned startPos,
  string& word)
{
  stringstream t;
  unsigned l = s.length();
  unsigned pos;
  bool oneFlag = false; // Lot of ado to turn 10 into T
  for (pos = startPos; pos < l; pos++)
  {
    const char c = s.at(pos);
    if (pos > 0 && c == ' ' && s.at(pos-1) == ' ')
      break;
    else if (c == '1')
    {
      if (oneFlag)
      {
        // Wasn't 10 after all
        t << "11";
        oneFlag = false;
      }
      else
        oneFlag = true;
    }
    else if (c == ' ')
    {
      if (oneFlag)
      {
        t << "1";
        oneFlag = false;
      }
    }
    else if (oneFlag)
    {
      if (c == '0')
        t << "T";
      else
        t << "1" << c;
      oneFlag = false;
    }
    else
      t << c;
  }

  if (pos == startPos)
    return false;

  word = t.str();
  return true;
}


bool ReadLastWord(
  const string& s,
  string& word)
{
  int pos = static_cast<int>(s.length()) - 1;
  while (pos >= 0 && s.at(static_cast<unsigned>(pos)) != ' ')
    pos--;
  
  if (pos == static_cast<int>(s.length()) - 1)
    return false;
  else if (pos < 0)
  {
    word = s;
    return true;
  }
  else
  {
    word = s.substr(static_cast<unsigned>(pos)+1);
    return true;
  }
}


bool ParsePlayer(
  const char c,
  Player& p)
{
  switch(c)
  {
    case 'N':
      p = BRIDGE_NORTH;
      break;
    case 'E':
      p = BRIDGE_EAST;
      break;
    case 'S':
      p = BRIDGE_SOUTH;
      break;
    case 'W':
      p = BRIDGE_WEST;
      break;
    default:
      return false;
  }
  return true;
}


void ConvertMultilineToVector(
  const string& sin,
  vector<string>& sout)
{
  size_t l = sin.size();
  size_t p = 0;
  while (p < l)
  {
    size_t found = sin.find("\n", p);
    sout.push_back(sin.substr(p, found-p));
    if (found == string::npos)
      return;

    p = found+1;
  }
}


void str2lines(
  const string& sin,
  vector<string>& sout)
{
  ConvertMultilineToVector(sin, sout);
}


string GuessOriginalLine(
  const string& fname,
  const unsigned count)
{
  regex re("(.*)\\....$");
  smatch match;
  if (! regex_search(fname, match, re) || match.size() == 0)
    return "";
  
  string base = match.str(1);
  if (base.size() > 6)
    base = base.substr(base.size()-6, base.size());

  return base.substr(0, 3) + ".RBN " + STR(count+1) + " records " + 
    STR(count/2) + " deals";
}


void toUpper(
  string& s)
{
  for (unsigned i = 0; i < s.size(); i++)
    s.at(i) = static_cast<char>(toupper(static_cast<int>(s.at(i))));
}


Format ext2format(const string& s)
{
  string t = s;
  toUpper(t);

  if (t == "LIN")
    return BRIDGE_FORMAT_LIN;
  else if (t == "PBN")
    return BRIDGE_FORMAT_PBN;
  else if (t == "RBN")
    return BRIDGE_FORMAT_RBN;
  else if (t == "RBX")
    return BRIDGE_FORMAT_RBX;
  else if (s == "TXT") // Lower-case txt is usually something else
    return BRIDGE_FORMAT_TXT;
  else if (t == "EML")
    return BRIDGE_FORMAT_EML;
  else if (t == "REC")
    return BRIDGE_FORMAT_REC;
  else
    return BRIDGE_FORMAT_SIZE;
}

