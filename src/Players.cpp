/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <vector>
#include "bconst.h"
#include "Players.h"
#include "portab.h"
#include "parse.h"
#include "Debug.h"


extern Debug debug;


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
  {
    LOG("Player out of bounds");
    return false;
  }

  players[player] = name;
  return true;
}


bool Players::SetRBNSide(
  const string& side,
  string& p1,
  string& p2)
{
  int seen = count(side.begin(), side.end(), '+');
  if (seen != 1)
  {
    LOG("RBN side does not have exactly one plus");
    return false;
  }

  vector<string> v(2);
  v.clear();
  tokenize(side, v, "+");

  p1 = v[0];
  p2 = v[1];
  return true;
}


bool Players::SetPlayersRBN(const string& names)
{
  if (names.length() <= 2)
  {
    LOG("Names are not in RBN format");
    return false;
  }

  int seen = count(names.begin(), names.end(), ':');

  if (seen == 0 || seen > 2)
  {
    LOG("Names are not in RBN format");
    return false;
  }

  vector<string> v(3);
  v.clear();
  tokenize(names, v, ":");

  if (! Players::SetRBNSide(v[0], 
      players[BRIDGE_NORTH], players[BRIDGE_SOUTH]))
    return false;

  if (! Players::SetRBNSide(v[1], 
      players[BRIDGE_WEST], players[BRIDGE_EAST]))
    return false;

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
      LOG("No overall LIN player format");
      return "";
    
    case BRIDGE_FORMAT_PBN:
      LOG("No overall PBN player format");
      return "";
    
    case BRIDGE_FORMAT_RBN:
      return Players::SetPlayersRBN(names);
    
    case BRIDGE_FORMAT_TXT:
      LOG("No overall TXT player format");
      return "";
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
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
      if (rm == "O")
        room = BRIDGE_ROOM_OPEN;
      else if (rm == "C")
        room = BRIDGE_ROOM_CLOSED;
      else
        return false;
      return true;
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}


bool Players::PlayersAreSet() const
{
  return (players[0] != "" || players[1] != "" ||
      players[2] != "" || players[3] != "");
}


bool Players::operator == (const Players& p2) const
{
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    if (players[p] != p2.players[p])
    {
      LOG("Players differ");
      return false;
    }
  }

  if (room != p2.room)
  {
    LOG("Rooms differ");
    return false;
  }
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

  return s.str() + "\n";
}


string Players::AsRBN() const
{
  stringstream s;
  s << "N " <<
    players[BRIDGE_NORTH] << "+" <<
    players[BRIDGE_SOUTH] << ":" <<
    players[BRIDGE_WEST] << "+" <<
    players[BRIDGE_EAST];

  if (room == BRIDGE_ROOM_OPEN)
    s << ":O";
  else if (room == BRIDGE_ROOM_CLOSED)
    s << ":C";
  return s.str() + "\n";
}


string Players::AsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No separate LIN players format");
      return "";
    
    case BRIDGE_FORMAT_PBN:
      LOG("No separate PBN players format");
      return "";
    
    case BRIDGE_FORMAT_RBN:
      return Players::AsRBN();
    
    case BRIDGE_FORMAT_TXT:
      LOG("No separate TXT players format");
      return "";
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
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
      LOG("No separate LIN player format");
      return "";
    
    case BRIDGE_FORMAT_PBN:
      s << "[" << PLAYER_NAMES_LONG[player] << " \"" <<
        players[player] << "\"]\n";
      return s.str();
    
    case BRIDGE_FORMAT_RBN:
      LOG("No separate RBN player format");
      return "";
    
    case BRIDGE_FORMAT_TXT:
      return players[player];
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
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
      s << "qx|" << ROOM_LIN[room] << no;
      return s.str();
    
    case BRIDGE_FORMAT_PBN:
      s << "[Room \"" << ROOM_PBN[room] << "\"]\n";
      return s.str();
    
    case BRIDGE_FORMAT_RBN:
      LOG("No separate RBN room");
      return "";
    
    case BRIDGE_FORMAT_TXT:
      s << ROOM_PBN[room];
      return s.str();
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}

