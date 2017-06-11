/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_PLAYERS_H
#define BRIDGE_PLAYERS_H

#include <string>

#include "bconst.h"

using namespace std;


class Players
{
  private:

    string players[BRIDGE_PLAYERS];

    Room roomVal;

    void setRBNSide(
      const string& side,
      const Player p1,
      const Player p2);
    
    void setLIN(
      const string& text,
      const bool hardFlag);
    void setRBN(const string& text);

    string strLIN() const;
    string strRBNCore() const;
    string strRBN() const;
    string strRBX() const;
    string strREC() const;
    string strTXT() const;


  public:

    Players();

    ~Players();

    void reset();

    void set(
      const string& text,
      const Format format,
      const bool hardFlag = true);

    void set(const string list[]);

    void setPlayer(
      const string& name,
      const Player player);

    void setNorth(const string& name);
    void setEast(const string& name);
    void setSouth(const string& name);
    void setWest(const string& name);

    void setRoom(
      const string& room,
      const Format format);

    Room room() const;

    unsigned missing() const;
    bool overlap(const Players& players2) const;

    bool operator == (const Players& players2) const;
    bool operator != (const Players& players2) const;

    string str(const Format format) const;

    string strDelta(
      const Players& refPlayers,
      const Format format) const;

    string strPlayer(
      const Player player,
      const Format format) const;

    string strRoom(
      const unsigned no,
      const Format format) const;
};

#endif

