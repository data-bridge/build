/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>

#include "Players.h"
#include "portab.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"



const string ROOM_LIN[] =
{
  "o", "c", ""
};

const string ROOM_PBN[] =
{
  "Open", "Closed", ""
};



Players::Players()
{
  Players::Reset();
}


Players::~Players()
{
}


void Players::Reset()
{
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    players[p] = "";

  room = BRIDGE_ROOM_UNDEFINED;
}


bool Players::SetPlayer(
  const string& name,
  const playerType player)
{
  if (player > BRIDGE_WEST)
    THROW("Player out of bounds");

  unsigned p;
  if (player == BRIDGE_SOUTH)
    p = BRIDGE_SOUTH;
  else if (player == BRIDGE_WEST)
    p = BRIDGE_WEST;
  else if (player == BRIDGE_NORTH)
    p = BRIDGE_NORTH;
  else
    p = BRIDGE_EAST;

  players[p] = name;
  return true;
}


bool Players::SetNorth(const string& name)
{
  return Players::SetPlayer(name, BRIDGE_NORTH);
}


bool Players::SetEast(const string& name)
{
  return Players::SetPlayer(name, BRIDGE_EAST);
}


bool Players::SetSouth(const string& name)
{
  return Players::SetPlayer(name, BRIDGE_SOUTH);
}


bool Players::SetWest(const string& name)
{
  return Players::SetPlayer(name, BRIDGE_WEST);
}


void Players::SetRBNSide(
  const string& side,
  string& p1,
  string& p2)
{
  if (side == "")
    return;

  int seen = count(side.begin(), side.end(), '+');
  if (seen != 1)
    THROW("RBN side does not have exactly one plus");

  vector<string> v(2);
  v.clear();
  tokenize(side, v, "+");

  p1 = v[0];
  p2 = v[1];
}


bool Players::SetPlayersLIN(const string& names)
{
  int seen = count(names.begin(), names.end(), ',');

  if (seen != 3 && seen != 7)
    THROW("Names are not in LIN format");

  vector<string> v(static_cast<unsigned>(seen+1));
  v.clear();
  tokenize(names, v, ",");

  unsigned start = (seen == 7 ? 4u : 0u);
  players[BRIDGE_SOUTH] = v[start];
  players[BRIDGE_WEST] = v[start+1];
  players[BRIDGE_NORTH] = v[start+2];
  players[BRIDGE_EAST] = v[start+3];
  return true;
}


bool Players::SetPlayersRBN(const string& names)
{
  if (names.length() <= 2)
    THROW("Names are not in RBN format");

  int seen = count(names.begin(), names.end(), ':');

  if (seen == 0 || seen > 2)
    THROW("Names are not in RBN format");

  vector<string> v(3);
  v.clear();
  tokenize(names, v, ":");

  Players::SetRBNSide(v[0], players[BRIDGE_NORTH], players[BRIDGE_SOUTH]);
  Players::SetRBNSide(v[1], players[BRIDGE_WEST], players[BRIDGE_EAST]);

  if (seen == 2)
  {
    if (! Players::SetRoom(v[2], BRIDGE_FORMAT_RBN))
      return false;
  }

  return true;
}


bool Players::SetPlayers(
  const string& names,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Players::SetPlayersLIN(names);
    
    case BRIDGE_FORMAT_PBN:
      THROW("No overall PBN player format");
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return Players::SetPlayersRBN(names);
    
    case BRIDGE_FORMAT_TXT:
      THROW("No overall TXT player format");
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


bool Players::SetRoom(
  const string& rm,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      if (rm == "o")
        room = BRIDGE_ROOM_OPEN;
      else if (rm == "c")
        room = BRIDGE_ROOM_CLOSED;
      else
        return false;
      return true;
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_TXT:
      if (rm == "Open")
        room = BRIDGE_ROOM_OPEN;
      else if (rm == "Closed")
        room = BRIDGE_ROOM_CLOSED;
      else
        return false;
      return true;
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if (rm == "O")
        room = BRIDGE_ROOM_OPEN;
      else if (rm == "C")
        room = BRIDGE_ROOM_CLOSED;
      else
        return false;
      return true;
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


bool Players::PlayersAreSet() const
{
  return (players[0] != "" || players[1] != "" ||
      players[2] != "" || players[3] != "");
}


roomType Players::GetRoom() const
{
  return room;
}


bool Players::operator == (const Players& p2) const
{
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    if (players[p] != p2.players[p])
      DIFF("Players differ");

  if (room != p2.room)
    DIFF("Rooms differ");
  else
    return true;
}


bool Players::operator != (const Players& p2) const
{
  return ! (* this == p2);
}


string Players::AsLIN() const
{
  stringstream s;
  s << 
    players[BRIDGE_SOUTH] << "," <<
    players[BRIDGE_WEST] << "," <<
    players[BRIDGE_NORTH] << "," <<
    players[BRIDGE_EAST];

  return s.str();
}


string Players::AsRBNCore() const
{
  stringstream s;
  if (players[BRIDGE_NORTH] != "" || players[BRIDGE_SOUTH] != "")
    s << players[BRIDGE_NORTH] << "+" << players[BRIDGE_SOUTH];
  s << ":";
  if (players[BRIDGE_WEST] != "" || players[BRIDGE_EAST] != "")
    s << players[BRIDGE_WEST] << "+" << players[BRIDGE_EAST];

  string st = s.str();
  // if (st == ":")
    // return "";

  if (room == BRIDGE_ROOM_OPEN)
    st += ":O";
  else if (room == BRIDGE_ROOM_CLOSED)
    st += ":C";
  return st;
}


string Players::AsRBN() const
{
  return "N " + Players::AsRBNCore() + "\n";
}

string Players::AsRBX() const
{
  return "N{" + Players::AsRBNCore() + "}";
}


string Players::AsREC() const
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


string Players::AsTXT() const
{
  stringstream s;
  unsigned l;
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    unsigned pp = (p+3) % 4;
    l = players[pp].length();
    l = Max(12, l+1);
    s << setw(l) << left << players[pp];
  }

  // TODO: Central function to trim trailing spaces.
  string st = s.str();
  int p = static_cast<int>(st.length()) - 1;
  while (p >= 0 && st.at(static_cast<unsigned>(p)) == ' ')
    p--;

  if (p < 0)
    return "";
  else
    return st.substr(0, static_cast<unsigned>(p)+1) + "\n";
}


string Players::AsString(
  const formatType f,
  const bool closedFlag) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return "pn|" + Players::AsLIN() + "|";

    case BRIDGE_FORMAT_LIN_TRN:
      if (closedFlag)
        return "pn|,,,," + Players::AsLIN() + "|";
      else
        return "pn|" + Players::AsLIN() + "|";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
      return Players::AsLIN();
    
    case BRIDGE_FORMAT_PBN:
      THROW("No separate PBN players format");
    
    case BRIDGE_FORMAT_RBN:
      return Players::AsRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Players::AsRBX();
    
    case BRIDGE_FORMAT_TXT:
      return Players::AsTXT();
    
    case BRIDGE_FORMAT_REC:
      return Players::AsREC();
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


string Players::AsBareString(
  const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
        return Players::AsLIN() + ",";

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Players::AsDeltaString(
  const Players& refPlayers,
  const formatType f) const
{
  string s;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      for (unsigned pp = 0; pp < BRIDGE_PLAYERS; pp++)
      {
        unsigned p = (pp+2) % 4; // BBO order, starting with South
        if (players[p] != refPlayers.players[p])
          s += players[p];
        s += ",";
      }
      return s;

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Players::PlayerAsString(
  const playerType player,
  formatType f) const
{
  stringstream s;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      THROW("No separate LIN player format");
    
    case BRIDGE_FORMAT_PBN:
      s << "[" << PLAYER_NAMES_LONG[player] << " \"" <<
        players[player] << "\"]\n";
      return s.str();
    
    case BRIDGE_FORMAT_RBN:
      THROW("No separate RBN player format");
    
    case BRIDGE_FORMAT_EML:
      return players[player].substr(0, 8);

    case BRIDGE_FORMAT_REC:
      if (players[player] == "")
        return PLAYER_NAMES_LONG[player];
      else
        return players[player].substr(0, 11);

    case BRIDGE_FORMAT_TXT:
      return players[player];
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


string Players::RoomAsString(
  const unsigned no,
  const formatType f) const
{
  if (room == BRIDGE_ROOM_UNDEFINED)
    return "";

  stringstream s;
  
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      s << "qx|" << ROOM_LIN[room] << no << "|";
      return s.str();

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      s << "qx|" << ROOM_LIN[room] << no << "|";
      return s.str();
    
    case BRIDGE_FORMAT_PBN:
      s << "[Room \"" << ROOM_PBN[room] << "\"]\n";
      return s.str();
    
    case BRIDGE_FORMAT_RBN:
      THROW("No separate RBN room");
    
    case BRIDGE_FORMAT_TXT:
      s << ROOM_PBN[room];
      return s.str();
    
    default:
      THROW("Invalid format " + STR(f));
  }
}

