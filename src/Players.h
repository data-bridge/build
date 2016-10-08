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

    string players[BRIDGE_PLAYERS];
    roomType room;

    void SetRBNSide(
      const string& side,
      string& p1,
      string& p2);
    
    bool SetPlayersLIN(const string& names);
    bool SetPlayersRBN(const string& names);

    string AsLIN() const;
    string AsRBNCore() const;
    string AsRBN() const;
    string AsRBX() const;
    string AsREC() const;
    string AsTXT() const;


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

    bool SetNorth(const string& name);
    bool SetEast(const string& name);
    bool SetSouth(const string& name);
    bool SetWest(const string& name);

    bool SetRoom(
      const string& room,
      const formatType f);

    bool PlayersAreSet() const;

    roomType GetRoom() const;

    bool operator == (const Players& p2) const;

    bool operator != (const Players& p2) const;

    string AsString(
      const formatType f,
      const bool closedFlag = false) const;

    string AsBareString(const formatType f) const;

    string AsDeltaString(
      const Players& refPlayers,
      const formatType f) const;

    string PlayerAsString(
      const playerType player,
      const formatType f) const;

    string RoomAsString(
      const unsigned no,
      const formatType f) const;
};

#endif

