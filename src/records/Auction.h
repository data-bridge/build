/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_AUCTION_H
#define BRIDGE_AUCTION_H

#include <vector>
#include <string>

#include "../include/bridge.h"

class Contract;

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
    Player dealer;
    Vul vul;

    unsigned len;
    unsigned lenMax;
    vector<Call> sequence;
    unsigned numPasses;
    Multiplier multiplier;
    unsigned activeCNo; // Contract number
    unsigned activeBNo; // Number in auction list


    void setTables();

    void setDealerLIN(const string& text);
    void setDealerPBN(const string& text);

    void extend();

    void addCallNo(
      const unsigned no,
      const string& alert = "");

    void addAlertsRBN(const vector<string>& lines);

    unsigned getRBNAlertNo(
      const string& text,
      size_t& pos,
      const bool extendedFlag) const;

    bool isPBNNote(
      const string& text,
      int& no,
      string& alert) const;

    void addAuctionLIN(const string& list);
    void addAuctionPBN(const vector<string>& list);
    void addAuctionRBN(const string& text);
    void addAuctionRBNCore(
      const string& text,
      const unsigned startPos = 0);

    string strTXTHeader(const int * lengths) const;
    string strEMLHeader() const;
    string strLIN() const;
    string strLIN_RP() const;
    string strPBN() const;
    string strRBNCore(const bool RBNflag) const;
    string strRBN() const;
    string strRBX() const;
    string strEML() const;
    string strTXT(const int * lengths) const;
    string strREC() const;

    bool lateAlerts() const;


  public:

    Auction();

    void reset();

    // Dealer and vulnerability.

    void setDealer(
      const string& text,
      const Format format);

    void setVul(
      const string& text,
      const Format format);

    bool hasDealerVul() const;

    Player getDealer() const;

    Vul getVul() const;

    void copyDealerVul(const Auction& auction2);

    unsigned length() const;


    // Auction itself.

    void addCall(
      const string& call,
      const string& alert = "");

    void addAlert(
      const unsigned alertNo,
      const string& alert);

    void addPasses();

    void undoLastCall();

    void addAuction(
      const string& text,
      const Format format);

    bool getContract(Contract& contract) const;
      
    bool isPassedOut() const;

    bool isOver() const;

    bool isEmpty() const;

    bool startsWith(const vector<string>& calls) const;


    bool operator == (const Auction& a2) const;
    bool operator != (const Auction& a2) const;


    string strCall(
      const unsigned number,
      const Format format) const;

    string str(
      const Format format,
      const int * lengths = nullptr) const;
    
    string strDealer(const Format format) const;

    string strVul(const Format format) const;
};

#endif

