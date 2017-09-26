/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SHEET_H
#define BRIDGE_SHEET_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#pragma warning(pop)

#include "Contract.h"
#include "Deal.h"
#include "Auction.h"
#include "Play.h"
#include "Buffer.h"
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
      vector<unsigned> linenoRS;
      vector<string> lineRS; // May be several
      vector<unsigned> indexmin; // Ditto
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

    void fail(const string& text) const;

    void parse(Buffer& buffer);

    unsigned refLineNoToHandNo(const unsigned lineno) const;

    void parseRefs();

    unsigned findHandNo(const string& label) const;

    string strHeader() const;
    string strLinks() const;
    string strHand(
      const SheetHandData& ho,
      const unsigned index) const;
    string strPlays() const;

    string handRange(const unsigned index) const;

    unsigned tagNo(
      const string& line,
      const string& tag) const;

    string suggestAuction(const unsigned index) const;
    string suggestPlay(const unsigned index) const;
    string suggestTricks(
      const SheetHandData& hd,
      const unsigned index) const;


  public:

    Sheet();

    ~Sheet();

    void reset();

    bool read(const string& fname);

    string str() const;
};

#endif

