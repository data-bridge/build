/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SHEETHAND_H
#define BRIDGE_SHEETHAND_H

#include <string>

#include "Contract.h"
#include "Deal.h"
#include "Auction.h"
#include "Play.h"


using namespace std;


enum SheetContractType
{
  SHEET_CONTRACT_HEADER,
  SHEET_CONTRACT_AUCTION
};

enum SheetTricksType
{
  SHEET_TRICKS_HEADER,
  SHEET_TRICKS_PLAY,
  SHEET_TRICKS_CLAIM,
};


class SheetHand
{
  private:

    struct SheetContract
    {
      string value;
      bool has;
    };

    struct SheetTricks
    {
      unsigned value;
      bool has;
    };

    Deal deal;
    bool hasDeal;

    Auction auction;
    bool hasAuction;

    Play play;
    bool hasPlay;

    SheetContract contractHeader;
    SheetTricks tricksHeader;

    SheetContract contractAuction;
    SheetTricks tricksPlay;
    SheetTricks tricksClaim;

    vector<string> chats;


    void fail() const;

    void strToContract(
      const Contract& contract,
      const SheetContractType type);
    
    void strToTricks(
      const Contract& contract,
      const SheetTricksType type);
    
    string cstr(const SheetContract& ct) const;
    string cstr(
      const SheetContract& ct,
      const SheetContract& cbase) const;
    string tstr(const SheetTricks& tr) const;
    string tstr(
      const SheetTricks& tr,
      const SheetTricks& tbase) const;

    bool contractsDiffer(
      const SheetContract& ct,
      const SheetContract& cf) const;

    bool tricksDiffer(
      const SheetTricks& tr,
      const SheetTricks& tf) const;

    string strNotesDetail() const;


  public:

    SheetHand();

    ~SheetHand();

    void reset();

    bool setDeal(const string& text);

    bool addCall(const string& text);

    bool claim(const string& text);

    void addChat(const string& text);

    void finishHand(
      const string& ct,
      const string& plays,
      const unsigned numPlays);

    bool hasData() const;

    bool operator ==(const SheetHand& href) const;
    bool operator !=(const SheetHand& href) const;

    string strNotes(const SheetHand& href) const;

    string strChat() const;

    string str() const;
    string str(const SheetHand& href) const;

    string strDummy() const;
};

#endif

