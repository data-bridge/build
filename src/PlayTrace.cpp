/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>

#include "PlayTrace.h"

#include "Bexcept.h"
#include "Bdiff.h"


PlayTrace::PlayTrace()
{
  PlayTrace::reset();
}


PlayTrace::~PlayTrace()
{
}


void PlayTrace::reset()
{
  len = 0;
  tricks.clear(); // Starts before lead
  playedBy.clear(); // Start with lead
  playErrors.clear();
  playErrorSet = false;
}


unsigned PlayTrace::hexchar2unsigned(const char c) const
{
  if (c >= '0' && c <= '9')
    return static_cast<unsigned>(c - '0');

  if (c >= 'A' && c <= 'F')
    return static_cast<unsigned>(c + 10 - 'A');

  THROW("Bad hex character: " + c);
}


char PlayTrace::unsigned2hexchar(const unsigned u) const
{
  if (u < 10)
    return static_cast<char>('0' + u);
  else if (u < 16)
    return static_cast<char>('A' + u - 10);
  
  THROW("Bad unsigned: " + STR(u));
}


void PlayTrace::setTricks(const string& st)
{
  // *xy means "repeat y x times".
  // ^xxy means the same, but with two digits for x.
  // x and y are in hex.

  const unsigned slen = st.length();
  unsigned pos = 0;
  while (pos < slen)
  {
    const char c = st.at(pos);
    unsigned repeat = 0, val, step;

    if (c == '^')
    {
      if (pos+3 >= slen)
        THROW("Bad ^ syntax in " + st);

      repeat = 16 * PlayTrace::hexchar2unsigned(st.at(pos+1)) +
        PlayTrace::hexchar2unsigned(st.at(pos+2));
      val = PlayTrace::hexchar2unsigned(st.at(pos+3));
      step = 4;
    }
    else if (c == '*')
    {
      if (pos+2 >= slen)
        THROW("Bad * syntax in " + st);

      repeat = PlayTrace::hexchar2unsigned(st.at(pos+1));
      val = PlayTrace::hexchar2unsigned(st.at(pos+2));
      step = 3;
    }
    else 
    {
      repeat = 1;
      val = PlayTrace::hexchar2unsigned(c);
      step = 1;
    }

    for (unsigned i = 0; i < repeat; i++)
      tricks.push_back(val);

    pos += step;
  }
  len = tricks.size();
}


void PlayTrace::setPlayedBy(const vector<Player>& playedByIn)
{
  if (len != playedByIn.size()+1) // Offset due to before-lead trick
    THROW("Bad playedByIn length: " + STR(len) + " vs. " +
      STR(playedByIn.size()));

  playedBy = playedByIn;
  playErrorSet = false;
}


void PlayTrace::fillError(
  PlayErrorRecord& per,
  const unsigned i,
  const PlayError e)
{
  per.error = e; // General error
  per.player = playedBy[i-1]; // Due to offset
  per.trickNo = (i+3) >> 2;
  per.posNo = (i-1) % 4 + 1;
  per.was = tricks[i-1];
  per.is = tricks[i];
}


void PlayTrace::deriveErrors()
{
  for (unsigned i = 1; i < len; i++)
  {
    if (tricks[i] == tricks[i-1])
      continue;
    
    playErrors.emplace_back(PlayErrorRecord());
    PlayErrorRecord& per = playErrors.back();
    PlayTrace::fillError(per, i, BRIDGE_PLAY_SIZE);
  }
}


void PlayTrace::deriveErrors(const vector<PlayError>& playErrorIn)
{
  unsigned p = 0;
  for (unsigned i = 1; i < len; i++)
  {
    if (tricks[i] == tricks[i-1])
      continue;
    
    playErrors.emplace_back(PlayErrorRecord());
    PlayErrorRecord& per = playErrors.back();
    PlayTrace::fillError(per, i, playErrorIn[p]);
    p++;
  }
}


void PlayTrace::set(
  const int number,
  const int * tricksIn,
  const vector<Player>& playedByIn)
{
  len = static_cast<unsigned>(number);
  for (unsigned i = 0; i < len; i++)
    tricks.push_back(static_cast<unsigned>(tricksIn[i]));
  
  PlayTrace::setPlayedBy(playedByIn);
  PlayTrace::deriveErrors();
}


void PlayTrace::set(
  const int number,
  const int * tricksIn,
  const vector<Player>& playedByIn,
  const vector<PlayError>& playErrorIn)
{
  PlayTrace::set(number, tricksIn, playedByIn);
  PlayTrace::deriveErrors(playErrorIn);

  playErrorSet = true;
}


void PlayTrace::set(
  const string& strCompact,
  const vector<Player>& playedByIn)
{
  const unsigned pos = strCompact.find_first_of("-");
  if (pos == string::npos)
  {
    PlayTrace::setTricks(strCompact);
    PlayTrace::setPlayedBy(playedByIn);
    PlayTrace::deriveErrors();
  }
  else
  {
    PlayTrace::setTricks(strCompact.substr(0, pos));
    PlayTrace::setPlayedBy(playedByIn);

    const string se = strCompact.substr(pos+1);
    vector<PlayError> per;
    unsigned i = 0;
    for (auto& pe: playErrors)
    {
      const unsigned e = PlayTrace::hexchar2unsigned(se.at(i));
      pe.error = static_cast<PlayError>(e);
      i++;
    }
    PlayTrace::deriveErrors(per);

    playErrorSet = true;
  }
}


void PlayTrace::set(
  const string& strCompact,
  const vector<Player>& playedByIn,
  const vector<PlayError>& playErrorIn)
{
  PlayTrace::set(strCompact, playedByIn);
  PlayTrace::deriveErrors(playErrorIn);

  playErrorSet = true;
}


bool PlayTrace::operator == (const PlayTrace& pt2) const
{
  if (len != pt2.len)
    THROW("Lengths differ");

  if (tricks.size() != len ||
      playedBy.size()+1 != len)
    THROW("Vectors have bad length.");
      
  if (pt2.tricks.size() != len ||
      pt2.playedBy.size()+1 != len)
    THROW("Vectors have bad length.");

  if (playErrorSet != pt2.playErrorSet)
    THROW("playErrorSet disagrees.");

  if (playErrorSet)
  {
    if (playErrors.size() != pt2.playErrors.size())
      THROW("playErrors lengths disagree.");

    for (auto it1 = playErrors.begin(), it2 = pt2.playErrors.begin();
         it1 != playErrors.end() && it2 != pt2.playErrors.end();
         it1++, it2++)
    {
      if (it1->error != it2->error ||
          it1->player != it2->player ||
          it1->trickNo != it2->trickNo ||
          it1->posNo != it2->posNo ||
          it1->was != it2->was ||
          it1->is != it2->is)
        THROW("playErrors vectors differ.");
    }
  }
      
  if (tricks != pt2.tricks)
    THROW("Trick vectors differ.");
      
  if (playedBy != pt2.playedBy)
    THROW("playedBy vectors differ.");
  
  return true;
}


bool PlayTrace::operator != (const PlayTrace& pt2) const
{
  return ! (* this == pt2);
}


string PlayTrace::str() const
{
  string st;
  for (unsigned i = 0; i < len; i++)
  {
    if (tricks[i] >= 10)
      st += static_cast<char>('A' + tricks[i] - 10);
    else
      st += to_string(tricks[i]);
  }
  return st;
}


string PlayTrace::strCompact() const
{
  string st = "";
  unsigned i = 0;
  while (i < len)
  {
    const unsigned t = tricks[i];
    const char c = PlayTrace::unsigned2hexchar(t);

    unsigned j = i+1;
    while (j < len && tricks[j] == t)
      j++;

    const unsigned repeat = j-i;
    if (repeat >= 16)
    {
      const unsigned msb = repeat >> 4;
      const unsigned lsb = repeat & 0xf;
      
      st += "^";
      st += PlayTrace::unsigned2hexchar(msb);
      st += PlayTrace::unsigned2hexchar(lsb);
      st += c;
    }
    else if (repeat > 3)
    {
      st += "*";
      st += PlayTrace::unsigned2hexchar(repeat);
      st += c;
    }
    else
    {
      for (unsigned k = 0; k < repeat; k++)
        st += c;
    }
    i = j;
  }

  if (playErrorSet)
  {
    st += "-";
    for (auto &pe: playErrors)
      st += PlayTrace::unsigned2hexchar(static_cast<unsigned>(pe.error));
  }

  return st;
}

