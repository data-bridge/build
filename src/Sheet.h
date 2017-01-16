/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SHEET_H
#define BRIDGE_SHEET_H

#include <string>

#include "Contract.h"
#include "Deal.h"
#include "Auction.h"
#include "Play.h"
#include "referr.h"


using namespace std;


class Sheet
{
  private:

    struct SheetHeader
    {
      string headline;
      vector<string> links;
      unsigned lineRS;
    };

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

    struct SheetHand
    {
      string label;
      string room;
      unsigned numberQX;
      unsigned lineLIN;

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

      vector<unsigned> refSource;
    };

    SheetHeader headerOrig;
    vector<SheetHand> handsOrig;

    SheetHeader headerFixed;
    vector<SheetHand> handsFixed;
    bool hasFixed;

    vector<RefFix> refFix;

    // ref fixes can affect multiple qx's
    struct RefEffects
    {
      RefErrorsType type;
      vector<unsigned> list;
    };

    vector<RefEffects> refEffects;


    void resetHeader(SheetHeader& hdr);
    void resetHand(SheetHand& hd);

    void parseVG(
      const string& value,
      unsigned& noHdrFirst,
      unsigned& noHdrLast) const;

    void parseRS(
      const string& value,
      vector<string>& clist) const;

    void parseBN(
      const string& value,
      vector<unsigned>& blist) const;

    string parseQX(const string& value) const;
    
    bool isLink(
      const string& value,
      string& link) const;

    string qxToHeaderContract(
      SheetHand& hd,
      const vector<string>& clist,
      const vector<unsigned>& blist,
      const unsigned noHdrFirst,
      const unsigned noHdrLast);

    void strToContract(
      const Contract& contract,
      SheetContract& contractSheet);
    
    void strToTricks(
      const Contract& contract,
      const SheetContract& contractSheet,
      SheetTricks& tricksSheet);
    
    void finishHand(
      SheetHand& hand,
      const vector<string>& clist,
      const vector<unsigned>& blist,
      const unsigned noHdrFirst,
      const unsigned noHdrLast,
      const string& plays,
      const unsigned numPlays);

    void parse(
      Buffer& buffer,
      SheetHeader& hdr,
      vector<SheetHand>& hd);

    unsigned refLineNoToHandNo(const unsigned lineno) const;

    bool lineToList(
      const string& line,
      vector<string>& list) const;

    void parseRefs(const Buffer& buffer);

    RefErrorsType classifyRefLine(
      const RefFix& refEntry,
      const string& bufferLine) const;

    unsigned findFixed(const string& label) const;
    unsigned findOrig(const string& label) const;

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

    bool handsDiffer(
      const SheetHand& ho,
      const SheetHand& hf) const;

    void extendNotes(
      const SheetHand& ho,
      const SheetHand& hf,
      stringstream& notes) const;

    void extendNotesWithChat(
      const SheetHand& ho,
      stringstream& notes) const;

    void extendNotesDetail(
      const SheetHand& hand,
      stringstream& notes) const;


  public:

    Sheet();

    ~Sheet();

    void reset();

    bool read(const string& fname);

    string str() const;
};

#endif

