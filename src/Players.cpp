/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>

#include "Players.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"



static const string ROOM_LIN[] =
{
  "o", "c", ""
};

static const string ROOM_PBN[] =
{
  "Open", "Closed", ""
};



Players::Players()
{
  Players::reset();
}


Players::~Players()
{
}


void Players::reset()
{
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    players[p] = "";

  roomVal = BRIDGE_ROOM_UNDEFINED;
}


void Players::setRBNSide(
  const string& text,
  const Player p1,
  const Player p2)
{
  if (text == "")
    return;

  int seen = static_cast<int>(count(text.begin(), text.end(), '+'));
  if (seen != 1)
    THROW("RBN side does not have exactly one plus");

  vector<string> v(2);
  v.clear();
  tokenize(text, v, "+");

  players[p1] = v[0];
  players[p2] = v[1];
}


void Players::setLIN(
  const string& text,
  const bool hardFlag)
{
  const int seen = static_cast<int>(count(text.begin(), text.end(), ','));
  vector<string> v(static_cast<unsigned>(seen+1));
  v.clear();
  tokenize(text, v, ",");

  if (seen != 3 && seen != 7)
  {
    unsigned s0;
    if (seen > 7)
      s0 = 8;
    else if (seen > 3)
      s0 = 4;
    else
      THROW("Names are not in LIN format: '" + text + "'");

    // Allow trailing commas.
    for (unsigned i = s0; i <= static_cast<unsigned>(seen); i++)
    {
      if (v[i] != "")
        THROW("Names are not in LIN format: '" + text + "'");
    }
  }

  const unsigned start = (seen == 7 ? 4u : 0u);
  if (v[start] == "South" && v[start+1] == "West" &&
       v[start+2] == "North" && v[start+3] == "East")
  {
    // Skip.
  }
  else if (hardFlag)
  {
    players[BRIDGE_SOUTH] = v[start];
    players[BRIDGE_WEST] = v[start+1];
    players[BRIDGE_NORTH] = v[start+2];
    players[BRIDGE_EAST] = v[start+3];
  }
  else
  {
    if (v[start] != "")
      players[BRIDGE_SOUTH] = v[start];
    if (v[start+1] != "")
      players[BRIDGE_WEST] = v[start+1];
    if (v[start+2] != "")
      players[BRIDGE_NORTH] = v[start+2];
    if (v[start+3] != "")
      players[BRIDGE_EAST] = v[start+3];
  }
}


void Players::setRBN(const string& text)
{
  if (text.length() <= 2)
    THROW("Names are not in RBN format");

  int seen = static_cast<int>(count(text.begin(), text.end(), ':'));
  if (seen == 0 || seen > 2)
    THROW("Names are not in RBN format");

  vector<string> v(3);
  v.clear();
  tokenize(text, v, ":");

  Players::setRBNSide(v[0], BRIDGE_NORTH, BRIDGE_SOUTH);
  Players::setRBNSide(v[1], BRIDGE_WEST, BRIDGE_EAST);

  if (seen == 2)
    Players::setRoom(v[2], BRIDGE_FORMAT_RBN);
}


void Players::set(
  const string& text,
  const Format format,
  const bool hardFlag)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      Players::setLIN(text, hardFlag);
      break;
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      Players::setRBN(text);
      break;
    
    default:
      THROW("Invalid format: " + STR(format) + ", " + text);
  }
}


void Players::setPlayer(
  const string& name,
  const Player player)
{
  unsigned p;
  if (player == BRIDGE_SOUTH)
    p = BRIDGE_SOUTH;
  else if (player == BRIDGE_WEST)
    p = BRIDGE_WEST;
  else if (player == BRIDGE_NORTH)
    p = BRIDGE_NORTH;
  else if (player == BRIDGE_EAST)
    p = BRIDGE_EAST;
  else
    THROW("Player out of bounds");

  players[p] = name;
}


void Players::setNorth(const string& name)
{
  Players::setPlayer(name, BRIDGE_NORTH);
}


void Players::setEast(const string& name)
{
  Players::setPlayer(name, BRIDGE_EAST);
}


void Players::setSouth(const string& name)
{
  Players::setPlayer(name, BRIDGE_SOUTH);
}


void Players::setWest(const string& name)
{
  Players::setPlayer(name, BRIDGE_WEST);
}


void Players::setRoom(
  const string& roomIn,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      if (roomIn == "o")
        roomVal = BRIDGE_ROOM_OPEN;
      else if (roomIn == "c")
        roomVal = BRIDGE_ROOM_CLOSED;
      else
        THROW("Unknown room: " + roomIn);
      break;
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      if (roomIn == "Open")
        roomVal = BRIDGE_ROOM_OPEN;
      else if (roomIn == "Closed")
        roomVal = BRIDGE_ROOM_CLOSED;
      else
        THROW("Unknown room: " + roomIn);
      break;
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if (roomIn == "O")
        roomVal = BRIDGE_ROOM_OPEN;
      else if (roomIn == "C")
        roomVal = BRIDGE_ROOM_CLOSED;
      else
        THROW("Unknown room: " + roomIn);
      break;
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


Room Players::room() const
{
  return roomVal;
}


bool Players::operator == (const Players& players2) const
{
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    if (players[p] != players2.players[p])
    {
      if ((players[p] == "" && 
          players2.players[p] == PLAYER_NAMES_LONG[p]) || 
          (players[p] == PLAYER_NAMES_LONG[p] && 
          players2.players[p] == ""))
      {
        // Accept.
      }
      else
        DIFF("Players differ");
    }

  if (roomVal != players2.roomVal)
    DIFF("Rooms differ");

  return true;
}


bool Players::operator != (const Players& players2) const
{
  return ! (* this == players2);
}


string Players::strLIN() const
{
  return 
    players[BRIDGE_SOUTH] + "," +
    players[BRIDGE_WEST] + "," +
    players[BRIDGE_NORTH] + "," +
    players[BRIDGE_EAST];
}


string Players::strRBNCore() const
{
  stringstream ss;
  if (players[BRIDGE_NORTH] != "" || players[BRIDGE_SOUTH] != "")
    ss << players[BRIDGE_NORTH] << "+" << players[BRIDGE_SOUTH];
  ss << ":";
  if (players[BRIDGE_WEST] != "" || players[BRIDGE_EAST] != "")
    ss << players[BRIDGE_WEST] << "+" << players[BRIDGE_EAST];

  string st = ss.str();

  if (roomVal == BRIDGE_ROOM_OPEN)
    st += ":O";
  else if (roomVal == BRIDGE_ROOM_CLOSED)
    st += ":C";
  return st;
}


string Players::strRBN() const
{
  return "N " + Players::strRBNCore() + "\n";
}


string Players::strRBX() const
{
  return "N{" + Players::strRBNCore() + "}";
}


string Players::strTXT() const
{
  stringstream ss;
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    const unsigned pTXT = PLAYER_DDS_TO_TXT[p];
    const unsigned l = 1u + 
      static_cast<unsigned>(Max(11, players[pTXT].length()));
    ss << setw(static_cast<int>(l)) << left << players[pTXT];
  }

  string st = trimTrailing(ss.str());
  if (st == "")
    return "";
  else
    return st + "\n";
}


string Players::strREC() const
{
  stringstream s;
  s << setw(9) << left << "West" <<
      setw(9) << left << "North" <<
      setw(9) << left << "East" <<
      "South\n";

  s << setw(8) << left << players[BRIDGE_WEST].substr(0, 8) << " " <<
      setw(8) << left << players[BRIDGE_NORTH].substr(0, 8) << " " <<
      setw(8) << left << players[BRIDGE_EAST].substr(0, 8) << " " <<
      players[BRIDGE_SOUTH].substr(0, 8) << "\n";

  return s.str();
}


string Players::str(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return "pn|" + Players::strLIN() + "|";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
      return Players::strLIN();
    
    case BRIDGE_FORMAT_LIN_TRN:
      if (roomVal == BRIDGE_ROOM_CLOSED)
        return "pn|,,,," + Players::strLIN() + "|";
      else
        return "pn|" + Players::strLIN() + "|";

    case BRIDGE_FORMAT_RBN:
      return Players::strRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Players::strRBX();
    
    case BRIDGE_FORMAT_TXT:
      return Players::strTXT();
    
    case BRIDGE_FORMAT_REC:
      return Players::strREC();
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Players::strDelta(
  const Players& refPlayers,
  const Format format) const
{
  string st;
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
      {
        unsigned pLIN = PLAYER_DDS_TO_LIN[p];
        if (players[pLIN] != refPlayers.players[pLIN])
          st += players[pLIN];
        st += ",";
      }
      return st;

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Players::strPlayer(
  const Player player,
  const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      return "[" + PLAYER_NAMES_LONG[player] + " \"" +
        players[player] + "\"]\n";
    
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_PAR:
      return players[player];
    
    case BRIDGE_FORMAT_EML:
      return players[player].substr(0, 8);

    case BRIDGE_FORMAT_REC:
      if (players[player] == "")
        return PLAYER_NAMES_LONG[player];
      else
        return players[player].substr(0, 11);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Players::strRoom(
  const unsigned no,
  const Format format) const
{
  if (roomVal == BRIDGE_ROOM_UNDEFINED)
    return "";

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return "qx|" + ROOM_LIN[roomVal] + STR(no) + "|";
    
    case BRIDGE_FORMAT_PBN:
      return "[Room \"" + ROOM_PBN[roomVal] + "\"]\n";
    
    case BRIDGE_FORMAT_TXT:
      return ROOM_PBN[roomVal];
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}

