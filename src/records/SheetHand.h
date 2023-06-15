/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

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

enum SheetPlayType
{
  SHEET_PLAY_OK,
  SHEET_PLAY_OPEN,
  SHEET_PLAY_BAD,
  SHEET_PLAY_SIZE
};

const vector<string> SheetPlayNames =
{
  "OK",
  "?",
  "BAD"
};

struct SheetPlayDistance
{
  unsigned numTricks;
  unsigned numCards;
  unsigned goodTricks;
  unsigned distance;
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
    bool auctionFlawed;
    string auctionFlaw;

    Play play;
    bool hasPlay;
    bool playFlawed;
    SheetPlayDistance playDistance;

    Contract cHeader;
    SheetContract contractHeader;
    Contract cAuction;
    SheetContract contractAuction;

    SheetTricks tricksHeader;
    SheetTricks tricksPlay;
    SheetTricks tricksClaim;
    SheetTricks tricksDDS;

    vector<string> chats;


    void strToContract(
      const Contract& contract,
      const SheetContractType type);
    
    void strToTricks(
      const Contract& contract,
      const SheetTricksType type);
    
    void setPlayDistance(const string& plays);
    void incrPlayDistance(const string& trick);

    void fail(const string& text) const;

    string cstr(const SheetContract& ct) const;
    string cstr(
      const SheetContract& ct,
      const SheetContract& cbase) const;
    string tstr(const SheetTricks& tr) const;
    string tstr(
      const SheetTricks& tr,
      const SheetTricks& tbase) const;

    bool contractsDiffer() const;

    bool tricksDiffer(
      const SheetTricks& tr,
      const SheetTricks& tf) const;

    string suggestTrick(
      const SheetTricks& tricks1,
      const SheetTricks& tricks2,
      const SheetTricks& tbase) const;

    string strNotesDetail() const;


  public:

    SheetHand();

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

    bool auctionIsFlawed() const;

    bool contractsOrTricksDiffer() const;

    bool playIsFlawed() const;

    SheetPlayType playValidity() const;

    const SheetPlayDistance& getPlayDistance() const;

    string tricksAlt() const;
    string strContractHeader() const;
    string strContractAuction() const;
    string strContractTag() const;

    string strNotes() const;
    string strChat() const;
    string str() const;
    string str(const SheetHand& href) const;
    string strDummy() const;
};

#endif

