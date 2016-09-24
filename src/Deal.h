/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DEAL_H
#define BRIDGE_DEAL_H

#include "Deal.h"
#include "Players.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

using namespace std;


class Deal
{
  private:

    bool setFlag;
    unsigned holding[BRIDGE_PLAYERS][BRIDGE_SUITS];
    char cards[BRIDGE_PLAYERS][BRIDGE_SUITS][BRIDGE_TRICKS];

    void SetTables();

    bool StringToPlayer(
      const string& s,
      unsigned& p) const;

    bool SetCards(
      const string suit,
      unsigned& res) const;

    bool SetHand(
      const string& hand,
      const string& delimiters,
      const unsigned offset,
      unsigned pholding[]);

    bool SetHands();

    bool SetLIN(const string& s);
    bool SetPBN(const string& s);
    bool SetRBN(const string& s);
    bool SetTXT(const string cardsArg[][BRIDGE_SUITS]);

    string AsLIN(const playerType start) const;
    string AsPBN(const playerType start) const;
    string AsRBNCore(const playerType start) const;
    string AsRBN(const playerType start) const;
    string AsRBX(const playerType start) const;
    string AsEML() const;
    string AsTXT() const;


  public:

    Deal();

    ~Deal();

    void Reset();

    bool IsSet() const;

    bool Set(
      const string& s,
      const formatType f = BRIDGE_FORMAT_LIN);

    bool Set(
      const string cardsArg[][BRIDGE_SUITS],
      const formatType f = BRIDGE_FORMAT_LIN);

    bool GetDDS(unsigned cards[][BRIDGE_SUITS]) const;

    bool operator == (const Deal& d2) const;

    bool operator != (const Deal& d2) const;

    string AsString(
      const playerType start = BRIDGE_NORTH,
      const formatType f = BRIDGE_FORMAT_LIN) const;

};

#endif

