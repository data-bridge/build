/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_AUCTION_H
#define BRIDGE_AUCTION_H

#include "bconst.h"
#include "Contract.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;


class Auction
{
  private:

    struct Call
    {
      unsigned no;
      string alert;
    };

    bool setDVFlag;
    playerType dealer;
    vulType vul;

    unsigned len;
    unsigned lenMax;
    vector<Call> sequence;
    unsigned numPasses;
    multiplierType multiplier;
    unsigned activeCNo; // Contract number
    unsigned activeBNo; // Number in auction list

    void SetTables();

    void AddCallNo(
      const unsigned no,
      const string& alert = "");

    bool ParseRBNDealer(const char c);

    bool ParseRBNVul(const char c);

    bool GetRBNAlertNo(
      const string& s,
      unsigned& pos,
      unsigned& aNo,
      const bool extendedFlag) const;

    string AsLIN() const;
    string AsPBN() const;
    string AsRBN() const;
    string AsTXT(const string& names = "") const;

  public:

    Auction();

    ~Auction();

    void Reset();

    bool SetDealerVul(
      const playerType d,
      const vulType v);

    bool IsOver() const;

    bool IsPassedOut() const;

    bool AddCall(
      const string& call,
      const string& alert = "");

    bool AddAlert(
      const unsigned alertNo,
      const string& alert);

    void AddPasses();

    bool UndoLastCall();

    bool AddAuctionRBN(
      const string& s);

    bool AddAuction(
      const string& s,
      const formatType f);

    bool operator == (const Auction& a2) const;

    bool operator != (const Auction& a2) const;

    bool ConsistentWith(const Contract& cref) const;
      
    string AsString(
      const formatType f,
      const string& names = "") const;
};

#endif

