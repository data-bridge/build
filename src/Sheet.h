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
#include "SheetHand.h"
#include "Reflines.h"


using namespace std;


class Sheet
{
  private:

    struct SheetHeader
    {
      string headline;
      vector<string> links;
      unsigned linenoRS;
      string lineRS;
      bool multipleRS;
      unsigned lineCount;
    };

    struct SheetHandData
    {
      string label;
      string roomQX;
      unsigned numberQX;
      unsigned linenoQX;
      unsigned linenoMC;
      string lineMC;
      string claim;

      SheetHand hand;

      vector<string> refSource;
    };

    SheetHeader header;
    vector<SheetHandData> hands;
    unsigned bmin, bmax;
    RefLines refLines;


    void resetHeader();
    void resetHand(SheetHandData& hd);

    void fail(const string& text) const;

    void parseVG(const string& value);

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
      SheetHandData& hd,
      const vector<string>& clist);

    void parse(Buffer& buffer);

    unsigned refLineNoToHandNo(const unsigned lineno) const;

    bool lineToList(
      const string& line,
      vector<string>& list) const;

    void parseRefs();

    unsigned findHandNo(const string& label) const;

    string strHeader() const;
    string strLinks() const;
    string strPlays() const;
    string strHand(
      SheetHandData& ho,
      const unsigned index);

    string handRange(const unsigned index) const;

    unsigned tagNo(
      const string& line,
      const string& tag) const;

    string suggestAuction(const unsigned index) const;
    string suggestPlay(const unsigned index) const;
    string suggestTricks(
      SheetHandData& hd,
      const unsigned index);


  public:

    Sheet();

    ~Sheet();

    void reset();

    bool read(const string& fname);

    string str();
};

#endif

