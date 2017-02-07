/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <fstream>
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
  const string& text, 
  vector<string>& tokens,
  const string& delimiters)
{
  string::size_type pos, lastPos = 0;

  while (true)
  {
    pos = text.find_first_of(delimiters, lastPos);
    if (pos == std::string::npos)
    {
      pos = text.length();
      tokens.push_back(string(text.data()+lastPos, 
        static_cast<string::size_type>(pos - lastPos)));
      break;
    }
    else
    {
      tokens.push_back(string(text.data()+lastPos,
        static_cast<string::size_type>(pos - lastPos)));
    }
    lastPos = pos + 1;
   }
}


unsigned countDelimiters(
  const string& text,
  const string& delimiters)
{
  int c = 0;
  for (unsigned i = 0; i < delimiters.length(); i++)
    c += static_cast<int>
      (count(text.begin(), text.end(), delimiters.at(i)));
  return static_cast<unsigned>(c);
}


void splitIntoWords(
  const string& text,
  vector<string>& words)
{
  // Split into words (split on \s+, effectively).
  unsigned pos = 0;
  unsigned startPos = 0;
  bool isSpace = true;
  const unsigned l = static_cast<unsigned>(text.length());

  while (pos < l)
  {
    if (text.at(pos) == ' ')
    {
      if (! isSpace)
      {
        words.push_back(text.substr(startPos, pos-startPos));
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
    words.push_back(text.substr(startPos, pos-startPos));
}


// getWords splits on one or more whitespaces

bool getWords(
  const string& text,
  string words[],
  const int maxCount,
  unsigned& actualCount)
{
  regex whitespace("(\\S+)");
  auto wordsBegin = sregex_iterator(text.begin(), text.end(), whitespace);
  auto wordsEnd = sregex_iterator();

  if (distance(wordsBegin, wordsEnd) > maxCount)
    return false;

  actualCount = 0;
  for (sregex_iterator it = wordsBegin; it != wordsEnd; it++)
    words[actualCount++] = it->str();

  return true;
}


bool str2unsigned(
  const string& text,
  unsigned& res)
{
  unsigned u;
  try
  {
    u = stoul(text, nullptr, 0);
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


bool str2upos(
  const string& text,
  unsigned& res)
{
  unsigned u;
  try
  {
    u = stoul(text, nullptr, 10);
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


bool str2int(
  const string& text,
  int& res)
{
  int i;
  size_t pos;
  try
  {
    i = stoi(text, &pos);
    if (pos != text.size())
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
  const string& text,
  float& res)
{
  float f;
  size_t pos;
  try
  {
    f = static_cast<float>(stod(text, &pos));
    if (pos != text.size())
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


unsigned str2month(const string& text)
{
  string st = text;
  toUpper(st);

  if (st == "JANUARY")
    return 1;
  else if (st == "FEBRUARY")
    return 2;
  else if (st == "MARCH")
    return 3;
  else if (st == "APRIL")
    return 4;
  else if (st == "MAY")
    return 5;
  else if (st == "JUNE")
    return 6;
  else if (st == "JULY")
    return 7;
  else if (st == "AUGUST")
    return 8;
  else if (st == "SEPTEMBER")
    return 9;
  else if (st == "OCTOBER")
    return 10;
  else if (st == "NOVEMBER")
    return 11;
  else if (st == "DECEMBER")
    return 12;
  else
    return 0;
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
      text == "None" || text == "NONE" || text == "Z")
    return BRIDGE_VUL_NONE;
  else if (text == "e" || text == "E" || text == "EW" || 
       text == "E" || text == "E-W")
    return BRIDGE_VUL_EAST_WEST;
  else if (text == "n" || text == "N" || text == "NS" || 
      text == "N-S" || text == "N")
    return BRIDGE_VUL_NORTH_SOUTH;
  else if (text == "b" || text == "B" || text == "All" || 
      text == "Both" || text == "BOTH")
    return BRIDGE_VUL_BOTH;
  else
    return BRIDGE_VUL_SIZE;
}


bool str2denom(
  const string& text,
  Denom& denom)
{
  if (text == "N" || text == "NT")
    denom = BRIDGE_NOTRUMP;
  else if (text == "S")
    denom = BRIDGE_SPADES;
  else if (text == "H")
    denom = BRIDGE_HEARTS;
  else if (text == "D")
    denom = BRIDGE_DIAMONDS;
  else if (text == "C")
    denom = BRIDGE_CLUBS;
  else
    return false;

  return true;
}


void str2lines(
  const string& text,
  vector<string>& lines)
{
  size_t l = text.size();
  size_t p = 0;
  while (p < l)
  {
    size_t found = text.find("\n", p);
    lines.push_back(text.substr(p, found-p));
    if (found == string::npos)
      return;

    p = found+1;
  }
}


string chars2str(char * buffer, unsigned buflen)
{
  return string(buffer, buflen);
}


bool char2player(
  const char c,
  Player& player)
{
  switch(c)
  {
    case 'N':
      player = BRIDGE_NORTH;
      break;
    case 'E':
      player = BRIDGE_EAST;
      break;
    case 'S':
      player = BRIDGE_SOUTH;
      break;
    case 'W':
      player = BRIDGE_WEST;
      break;
    default:
      return false;
  }
  return true;
}


Format ext2format(const string& text)
{
  string st = text;
  toUpper(st);

  if (st == "LIN")
    return BRIDGE_FORMAT_LIN;
  else if (st == "PBN")
    return BRIDGE_FORMAT_PBN;
  else if (st == "RBN")
    return BRIDGE_FORMAT_RBN;
  else if (st == "RBX")
    return BRIDGE_FORMAT_RBX;
  else if (st == "TXT") // Lower-case txt is usually something else
    return BRIDGE_FORMAT_TXT;
  else if (st == "EML")
    return BRIDGE_FORMAT_EML;
  else if (st == "REC")
    return BRIDGE_FORMAT_REC;
  else
    return BRIDGE_FORMAT_SIZE;
}


string changeExt(
  const string& fname,
  const string& newExt)
{
  regex re("\\.\\w+$");
  return regex_replace(fname, re, newExt);
}


void appendFile(
  const string& fname,
  const unsigned lineno,
  const string& command,
  const string& text)
{
  const char * fn = fname.c_str();
  ifstream f(fn);
  ofstream fout;
  if (! f.good())
  {
    // New file.
    fout.open(fn);
    fout << lineno << " " << command << " \"" << text << "\"\n";
    return;
  }

  vector<string> buf;
  buf.clear();
  string tmp;
  while (getline(f, tmp))
    buf.push_back(tmp);
  f.close();

  fout.open(fn);
  regex re("^(\\d+)\\s+(\\w+)\\s+\"(.*)\"\\s*$");
  smatch match;
  const string newline = STR(lineno) + " " + 
    command + " \"" + text + "\"\n";

  bool seen = false;
  for (auto& line: buf)
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    if (! regex_search(line, match, re) || match.size() < 3)
      fout << line << "\n";
    else
    {
      unsigned lno;
      if (! str2unsigned(match.str(1), lno))
        fout << line << "\n";
      else if (lno < lineno)
        fout << line << "\n";
      else if (lno == lineno)
      {
        if (command == match.str(2))
        {
          // Replace.
          fout << newline;
          seen = true;
        }
        else
          fout << line << "\n";
      }
      else if (! seen)
      {
        // Insert.
        fout << newline;
        seen = true;
      }
      else
      {
        fout << line << "\n";
      }
    }
  }

  if (! seen)
    fout << newline;
  fout.close();
}


string basefile(const string& path)
{
  size_t pos = path.find_last_of("/\\");
  if (pos == string::npos)
    return path;
  else
    return path.substr(pos+1);
}


void toUpper(
  string& text)
{
  for (unsigned i = 0; i < text.size(); i++)
    text.at(i) = static_cast<char>(toupper(static_cast<int>(text.at(i))));
}


unsigned trimLeading(
  string& text,
  const char c)
{
  const unsigned l = static_cast<unsigned>(text.length());
  unsigned pos = 0;
  while (pos < l && text.at(pos) == c)
    pos++;

  if (pos > 0)
    text.erase(0, pos);
  return pos;
}


string trimTrailing(
  const string& str,
  const char c)
{
  unsigned pos = static_cast<unsigned>(str.length());
  while (pos >= 1 && str.at(pos-1) == c)
    pos--;

  if (pos == 0)
    return "";
  else
    return str.substr(0, pos);
}


bool getNextWord(
  string& text,
  string& word)
{
  // Consumes from s!
  const unsigned l = static_cast<unsigned>(text.length());
  unsigned pos = 0;
  while (pos < l && text.at(pos) != ' ')
    pos++;
  
  if (pos == 0)
    return false;
  else if (pos == l)
  {
    word = text;
    text = "";
    return true;
  }
  else
  {
    word = text.substr(0, pos);
    text.erase(0, pos+1);
    return true;
  }
}


bool readNextWord(
  const string& text,
  const unsigned startPos,
  string& word)
{
  unsigned l = static_cast<unsigned>(text.length());
  unsigned pos = startPos;
  while (pos < l && text.at(pos) != ' ')
    pos++;
  
  if (pos == startPos)
    return false;
  else
  {
    word = text.substr(startPos, pos-startPos);
    return true;
  }
}


bool readNextWord(
  const string& text,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word)
{
  const unsigned l = static_cast<unsigned>(text.length());
  unsigned pos = startPos;
  while (pos < l && pos <= stopPosInclusive && text.at(pos) != ' ')
    pos++;
  
  if (pos == startPos)
    return false;
  else
  {
    word = text.substr(startPos, pos-startPos);
    return true;
  }
}


bool readAllWords(
  const string& text,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word)
{
  const unsigned l = static_cast<unsigned>(text.length());
  if (l == 0 || l < startPos || text.at(startPos) == ' ')
    return false;

  unsigned pos = Min(l-1, stopPosInclusive);
  while (pos > startPos && text.at(pos) == ' ')
    pos--;
  
  if (text.at(pos) == ' ')
    return false;
  else
  {
    word = text.substr(startPos, pos+1-startPos);
    return true;
  }
}


bool readAllWordsOverlong(
  const string& text,
  const unsigned startPos,
  const unsigned stopPosInclusive,
  string& word)
{
  const unsigned l = static_cast<unsigned>(text.length());
  if (l == 0 || l < startPos)
    return false;

  // Skip over leading spaces.
  unsigned spos = startPos;
  while (spos < l && text.at(spos) == ' ')
    spos++;
  if (spos == l)
    return false;

  // Spill over end until end of a word is reached.
  unsigned pos = Min(l-1, stopPosInclusive);
  while (pos < l && text.at(pos) != ' ')
    pos++;

  if (pos == l)
    pos--;
  else
  {
    while (pos > spos && text.at(pos) == ' ')
      pos--;
  }
  
  word = text.substr(spos, pos+1-spos);
  return true;
}


bool readNextSpacedWord(
  const string& text,
  const unsigned startPos,
  string& word)
{
  const unsigned l = static_cast<unsigned>(text.length());
  stringstream ss;
  unsigned pos;
  bool oneFlag = false; // Lot of ado to turn 10 into T
  for (pos = startPos; pos < l; pos++)
  {
    const char c = text.at(pos);
    if (pos > 0 && c == ' ' && text.at(pos-1) == ' ')
      break;
    else if (c == '1')
    {
      if (oneFlag)
      {
        // Wasn't 10 after all
        ss << "11";
        oneFlag = false;
      }
      else
        oneFlag = true;
    }
    else if (c == ' ')
    {
      if (oneFlag)
      {
        ss << "1";
        oneFlag = false;
      }
    }
    else if (oneFlag)
    {
      if (c == '0')
        ss << "T";
      else
        ss << "1" << c;
      oneFlag = false;
    }
    else
      ss << c;
  }

  if (pos == startPos)
    return false;

  word = ss.str();
  return true;
}


bool readLastWord(
  const string& text,
  string& word)
{
  int pos = static_cast<int>(text.length()) - 1;
  while (pos >= 0 && text.at(static_cast<unsigned>(pos)) != ' ')
    pos--;
  
  if (pos == static_cast<int>(text.length()) - 1)
    return false;
  else if (pos < 0)
  {
    word = text;
    return true;
  }
  else
  {
    word = text.substr(static_cast<unsigned>(pos)+1);
    return true;
  }
}


string guessOriginalLine(
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

