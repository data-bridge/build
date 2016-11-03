/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DEAL_H
#define BRIDGE_DEAL_H

#include <string>
#include "bconst.h"

using namespace std;


class Deal
{
  private:

    bool setFlag;
    unsigned holding[BRIDGE_PLAYERS][BRIDGE_SUITS];
    char cards[BRIDGE_PLAYERS][BRIDGE_SUITS][BRIDGE_TRICKS];


    void setTables();

    Player strToPlayer(const string& s) const;

    unsigned suitToHolding(const string suit) const;

    void setHand(
      const string& hand,
      const string& delimiters,
      const unsigned offset,
      unsigned pholding[]);

    void setHands();

    void setLIN(const string& text);
    void setPBN(const string& text);
    void setRBN(const string& text);

    string strLINReverse(const Player start) const;
    string strLINRegular(
      const Player start,
      const unsigned limit) const;
    string strPBN(const Player start) const;
    string strRBNCore(const Player start) const;
    string strRBN(const Player start) const;
    string strRBX(const Player start) const;
    string strTXT() const;
    string strEML() const;
    string strRECDetail(
      const Player midPlayer,
      const unsigned LRsuit,
      const unsigned mSuit) const;
    string strREC() const;


  public:

    Deal();

    ~Deal();

    void reset();

    bool isSet() const;

    void set(
      const string& text,
      const Format format);

    void getDDS(unsigned cards[][BRIDGE_SUITS]) const;

    bool operator == (const Deal& deal2) const;

    bool operator != (const Deal& deal2) const;

    string str(
      const Player start,
      const Format format) const;
};

#endif

