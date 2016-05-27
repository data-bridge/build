/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DEAL_H
#define BRIDGE_DEAL_H

#include "Deal.h"


class Deal
{
  private:

    bool setFlag;
    unsigned holding[BRIDGE_PLAYERS];

    bool SetLIN(const string& s);

    bool SetPBN(const string& s);

    bool SetRBN(const string& s);

    bool SetTXT(const string& s);

    string AsLIN() const;

    string AsPBN() const;

    string AsRBN() const;

    string AsTXT() const;


  public:

    Deal();

    ~Deal();

    void Reset();

    bool IsSet() const;

    bool Set(
      const string& s,
      const formatType f = BRIDGE_FORMAT_LIN);

    bool GetDDS(unsigned cards[]) const;

    bool operator == (const Deal& d2) const;

    bool operator != (const Deal& d2) const;

    string AsString(
      const formatType f = BRIDGE_FORMAT_LIN) const;
};

#endif

