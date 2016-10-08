/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_AUCTION_H
#define BRIDGE_AUCTION_H

#include "bconst.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Contract;


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

    bool SetDealerLIN(
      const string& d,
      playerType& dOut) const;

    bool SetDealerPBN(
      const string& d,
      playerType& dOut) const;

    bool SetDealerTXT(
      const string& d,
      playerType& dOut) const;

    bool SetVulLIN(
      const string& v,
      vulType& vOut) const;

    bool SetVulPBN(
      const string& v,
      vulType& vOut) const;

    bool SetVulRBN(
      const string& v,
      vulType& vOut) const;

    bool SetVulTXT(
      const string& v,
      vulType& vOut) const;

    bool ParseDealerVul(
      const string& d,
      const string& v,
      const formatType f,
      playerType& dOut,
      vulType& vOut) const;

    void AddCallNo(
      const unsigned no,
      const string& alert = "");

    bool ParseRBNDealer(const char c);

    bool ParseRBNVul(const char c);

    bool GetRBNAlertNo(
      const string& s,
      size_t& pos,
      unsigned& aNo,
      const bool extendedFlag) const;

    bool AddAlertsRBN(const vector<string>& lines);

    bool IsPBNNote(
      const string& s,
      int& no,
      string& alert) const;

    bool AddAuctionLIN(const string& list);
    bool AddAuctionPBN(const vector<string>& list);

    string AsLIN() const;
    string AsLIN_RP() const;
    string AsPBN() const;
    string AsRBNCore(const bool RBNflag) const;
    string AsRBN() const;
    string AsRBX() const;
    string AsEML() const;
    string AsTXT(const unsigned lengths[BRIDGE_PLAYERS]) const;
    string AsREC() const;

    string DealerAsLIN() const;
    string DealerAsPBN() const;
    string DealerAsEML() const;
    string DealerAsTXT() const;

    string VulAsLIN() const;
    string VulAsLIN_RP() const;
    string VulAsPBN() const;
    string VulAsRBN() const;
    string VulAsEML() const;
    string VulAsTXT() const;

  public:

    Auction();

    ~Auction();

    void Reset();

    bool SetDealerVul(
      const string& d,
      const string& v,
      const formatType f);

    bool SetDealer(
      const string& d,
      const formatType f);

    playerType GetDealer() const;

    bool SetVul(
      const string& v,
      const formatType f);

    bool CheckDealerVul(
      const string& d,
      const string& v,
      const formatType f) const;

    void CopyDealerVulFrom(const Auction& a2);

    bool IsOver() const;

    bool IsEmpty() const;

    bool IsPassedOut() const;

    bool DVIsSet() const;

    vulType GetVul() const;

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

    bool AddAuctionEML(
      const string& s,
      const unsigned startPos = 0);

    bool AddAuction(
      const string& s,
      const formatType f);

    bool AddAuction(
      const vector<string>& s,
      const formatType f);

    bool operator == (const Auction& a2) const;

    bool operator != (const Auction& a2) const;

    bool ExtractContract(Contract& contract) const;
      
    bool ConsistentWith(const Contract& cref) const;
      
    string AsString(
      const formatType f,
      const string& names = "") const;
    
    string AsString(
      const formatType f,
      const unsigned lengths[BRIDGE_PLAYERS]) const;
    
    string DealerAsString(const formatType f) const;

    string VulAsString(const formatType f) const;
};

#endif

