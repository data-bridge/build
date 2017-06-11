/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

#include "HeaderLIN.h"
#include "parse.h"
#include "Bexcept.h"



HeaderLIN::HeaderLIN()
{
  HeaderLIN::reset();
}


HeaderLIN::~HeaderLIN()
{
}


void HeaderLIN::reset()
{
  len = 0;
  LINdata.clear();
  playersListFlag = false;
}


bool HeaderLIN::isShortPass(const string& st) const
{
  return (st.length() == 1 && (st == "P" || st == "p"));
}


void HeaderLIN::setResultsList(
  const string& text,
  const Format format)
{
  if (FORMAT_INPUT_MAP[format] != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));
  
  size_t c = countDelimiters(text, ",");
  if (c == 0 || c > 256)
    THROW("Bad number of fields");

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c % 2 == 0)
  {
    if (tokens[c] == "")
      c--;
    else
    {
      // Some old lin files lack a single trailing comma.
      c++;
      tokens.push_back("");
    }
  }

  if (c+1 > 2*len)
  {
    len = static_cast<unsigned>((c+1)/2);
    LINdata.resize(len);
  }

  for (size_t b = 0, d = 0; b < c+1; b += 2, d++)
  {
    if (HeaderLIN::isShortPass(tokens[b]))
      LINdata[d].data[0].contract = "PASS";
    else
      LINdata[d].data[0].contract = tokens[b];

    if (HeaderLIN::isShortPass(tokens[b+1]))
      LINdata[d].data[1].contract = "PASS";
    else
      LINdata[d].data[1].contract = tokens[b+1];
  }
}


void HeaderLIN::setPlayersList(
  const string& text,
  const string& scoring,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN && 
      format != BRIDGE_FORMAT_LIN_VG &&
      format != BRIDGE_FORMAT_LIN_TRN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c == 7)
  {
    // Assume a single set of 8 players repeating.

    for (size_t b = 0; b < len; b++)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
      {
        LINdata[b].data[0].players[(d+2) % 4] = tokens[d];
        LINdata[b].data[1].players[(d+2) % 4] = tokens[d+4];
      }
    }
  }
  else if (c == 3)
  {
    // Assume a single set of 4 players repeating.
    //
    for (size_t b = 0; b < len; b++)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
        LINdata[b].data[0].players[(d+2) % 4] = tokens[d];
    }
  }
  else if (format == BRIDGE_FORMAT_LIN_VG ||
      format == BRIDGE_FORMAT_LIN_RP)
  {
    if (format ==  BRIDGE_FORMAT_LIN_VG && scoring == "P")
    {
      // Errors in some early LIN_VG files.
      if (c+2 == 8*len)
      {
        tokens.push_back("");
        c++;
      }

      // Guess whether players repeat in blocks of 4 or 8.
      if (c+1 == 8*len)
      {
        for (size_t b = 0; b < c; b += 8)
        {
          for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
          {
            LINdata[b >> 3].data[0].players[(d+2) % 4] = 
              HeaderLIN::getEffectivePlayer(b, d, 8, tokens);
            LINdata[b >> 3].data[1].players[(d+2) % 4] =
              HeaderLIN::getEffectivePlayer(b, d+4, 8, tokens);
          }
        }
        playersListFlag = true;
      }
      else
      {
        if (c+5 == 4*len || c+9 == 4*len)
        {
          const unsigned cnt = 4*len-c-1;
          for (unsigned i = 0; i < cnt; i++)
            tokens.push_back("");
          c += cnt;
        }
        else if (len > 0 && c+1 > 4*len)
        {
          HeaderLIN::checkPlayersTrailing(4*len, c, tokens);
          c = 4*len-1;
        }

        if (c+1 != 4*len && (c != 4*len || tokens[c] != ""))
          THROW("Wrong number of fields: " + STR(c) + " vs. " + 
            " 4*len " + STR(4*len));

        for (size_t b = 0; b < c; b += 4)
          for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
            LINdata[b >> 2].data[0].players[(d+2) % 4] = 
              HeaderLIN::getEffectivePlayer(b, d, 4, tokens);

        playersListFlag = true;
      }
    }
    else
    {
      if (c+1 != 8*len)
        THROW("Wrong number of fields: " + 
          STR(c+1) + " vs. "  + STR(8*len));
    
      for (size_t b = 0; b < c; b += 8)
      {
        for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
        {
          LINdata[b >> 3].data[0].players[(d+2) % 4] = 
            HeaderLIN::getEffectivePlayer(b, d, 8, tokens);
          LINdata[b >> 3].data[1].players[(d+2) % 4] = 
            HeaderLIN::getEffectivePlayer(b, d+4, 8, tokens);
        }
      }
      playersListFlag = true;
    }
  }
  else
  {
    if (c+1 != 4*len)
      THROW("Wrong number of fields: " + STR(c) + " vs. " + 
        " 4*len " + STR(4*len));

    for (size_t b = 0; b < c; b += 4)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
      {
        LINdata[b >> 2].data[0].players[(d+2) % 4] = tokens[b+d];
      }
    }
    playersListFlag = true;
  }
}


string HeaderLIN::getEffectivePlayer(
  const unsigned start,
  const unsigned offset,
  const unsigned step,
  const vector<string>& tokens) const
{
  // start must be a multiple of step.
  // Search backwards for the first non-empty entry.

  for (unsigned e = 0; e <= start; e += step)
  {
    if (tokens[(start-e)+offset] != "")
      return tokens[(start-e)+offset];
  }
  return "";
}
  

void HeaderLIN::setPlayersHeader(
  const string& text,
  const string& scoring,
  const Format format)
{
  if (FORMAT_INPUT_MAP[format] != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c < 3)
      THROW("Bad number of fields");

  if (c > 7)
  {
    HeaderLIN::checkPlayersTrailing(8, c, tokens);
    c = 7;
  }

  if (c == 7)
  {
    for (size_t b = 0; b < len; b++)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
      {
        LINdata[b].data[0].players[(d+2) % 4] = tokens[d];
        LINdata[b].data[1].players[(d+2) % 4] = tokens[d+4];
      }
    }


    return;
  }

  if (format ==  BRIDGE_FORMAT_LIN_VG && scoring == "P")
  {
    if (c > 3)
      HeaderLIN::checkPlayersTrailing(4, c, tokens);

    for (size_t b = 0; b < len; b++)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
        LINdata[b].data[0].players[(d+2) % 4] = tokens[d];
    }
  }
  else if (c < 7)
    THROW("Bad number of fields");
}


void HeaderLIN::checkPlayersTrailing(
  const unsigned first,
  const unsigned lastIncl,
  const vector<string>& tokens) const
{
  for (unsigned i = first; i <= lastIncl; i++)
    if (tokens[i] != "" &&
        tokens[i] != PLAYER_NAMES_LONG[PLAYER_LIN_TO_DDS[i % 4]])
      THROW("Bad number of fields: " + STR(first) + " vs. " + 
                " lastIncl " + STR(lastIncl));
}


void HeaderLIN::setScoresList(
  const string& text,
  const string& scoring,
  const Format format)
{
  if (FORMAT_INPUT_MAP[format] != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  if (format ==  BRIDGE_FORMAT_LIN_VG && scoring == "P")
  {
  }
  else
  {
    vector<string> tokens(c+1);
    tokens.clear();
    tokenize(text, tokens, ",");

    if (c+1 != 2*len && (c != 2*len || tokens[c] != ""))
      THROW("Wrong number of fields: " + STR(c+1));

    for (size_t b = 0, d = 0; b < c; b += 2, d++)
    {
      if (tokens[b] == "")
      {
        if (tokens[b+1] == "")
          LINdata[d].data[0].mp = "";
        else
          LINdata[d].data[0].mp = "-" + tokens[b+1];
      }
      else
        LINdata[d].data[0].mp = tokens[b];
    }
  }
}


void HeaderLIN::setBoardsList(
  const string& text,
  const Format format)
{
  if (FORMAT_INPUT_MAP[format] != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));
  
  size_t c = countDelimiters(text, ",");
  if (c > 100)
    THROW("Too many fields");

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c == len && tokens[c] == "")
    c--;

  if (c+1 != len)
    THROW("Odd number of boards");

  for (unsigned i = 0; i <= c; i++)
    LINdata[i].no = tokens[i];
}


LINData const * HeaderLIN::getEntry(const unsigned intNo) const
{
  return &LINdata[intNo];
}


bool HeaderLIN::isSet() const
{
  return (len > 0);
}


bool HeaderLIN::hasPlayerList() const
{
  return playersListFlag;
}


string HeaderLIN::strPlayers(
  const unsigned intNo,
  const unsigned no) const
{
  string st;
  for (unsigned i = 0; i < no; i++)
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
      st += LINdata[intNo].data[i].players[PLAYER_DDS_TO_LIN[p]] + ",";
  return st;
}


string HeaderLIN::strBoard(const unsigned intNo) const
{
  return LINdata[intNo].no;
}


string HeaderLIN::strContracts(const unsigned intNo) const
{
  if (len == 0)
    return ",,";
  else
    return LINdata[intNo].data[0].contract + "," +
      LINdata[intNo].data[1].contract + ",";
}


string HeaderLIN::strContractsList() const
{
  if (len == 0)
    return "";

  string st;
  for (unsigned i = 0; i < len; i++)
    st += HeaderLIN::strContracts(i);
  st.pop_back();
  return st;
}

