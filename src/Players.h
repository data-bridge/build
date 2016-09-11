/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_PLAYERS_H
#define BRIDGE_PLAYERS_H

#include "bconst.h"
#include <string>

using namespace std;


class Players
{
  private:

    enum roomType
    {
      BRIDGE_ROOM_OPEN = 0,
      BRIDGE_ROOM_CLOSED = 1,
      BRIDGE_ROOM_UNDEFINED = 2
    };

    string players[BRIDGE_PLAYERS];
    roomType room;

    bool SetRBNSide(
      const string& side,
      string& p1,
      string& p2);
    
    bool SetPlayersRBN(const string& names);

    string AsLIN() const;
    string AsRBN() const;
      


  public:

    Players();

    ~Players();

    void Reset();

    bool SetPlayers(
      const string& names,
      const formatType f);

    bool SetPlayer(
      const string& name,
      const playerType player);

    bool SetRoom(
      const string& room,
      const formatType f);

    bool PlayersAreSet() const;

    bool operator == (const Players& p2) const;

    bool operator != (const Players& p2) const;

    string AsString(const formatType f) const;

    string PlayerAsString(
      const playerType player,
      const formatType f) const;

    string RoomAsString(
      const unsigned no,
      const formatType f) const;
};

#endif

