/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_AUCTION_H
#define BRIDGE_AUCTION_H

#include "bconst.h"
#include <string>

using namespace std;


class Auction
{
  private:

    bool setFlag;
    int value;


  public:

    Auction();

    ~Auction();

    void Reset();

    bool IsOver() const;

    bool IsPassedOut() const;

    bool AddCall(
      const string call,
      const string alert = "");

    bool AddPasses();

    bool UndoLastCall();

    bool AddAuction(
      const string s,
      const formatType f);

    bool operator == (const Auction& a2);

    bool operator != (const Auction& a2);

    string AsString(
      const formatType f) const;
    
    string TableAsPBN() const;

    string ResultAsString(
      const formatType f) const;
};

#endif

