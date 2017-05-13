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
#include "Refline.h"
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

    struct SheetHandData
    {
      string label;
      string room;
      unsigned numberQX;
      unsigned lineLIN;

      SheetHand hand;

      vector<unsigned> refSource;
    };

    SheetHeader headerOrig;
    vector<SheetHandData> handsOrig;

    SheetHeader headerFixed;
    vector<SheetHandData> handsFixed;
    bool hasFixed;

    vector<Refline> reflines;

    // ref fixes can affect multiple qx's
    struct RefEffects
    {
      RefErrorsType type;
      vector<unsigned> list;
      string line;
      unsigned numTags;
    };

    vector<RefEffects> refEffects;


    void resetHeader(SheetHeader& hdr);
    void resetHand(SheetHandData& hd);

    void fail(const string& text) const;

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
      SheetHandData& hd,
      const vector<string>& clist,
      const vector<unsigned>& blist,
      const unsigned noHdrFirst,
      const unsigned noHdrLast);

    void parse(
      Buffer& buffer,
      SheetHeader& hdr,
      vector<SheetHandData>& hd);

    unsigned refLineNoToHandNo(const unsigned lineno) const;

    bool lineToList(
      const string& line,
      vector<string>& list) const;

    void parseRefs(const Buffer& buffer);

    unsigned findFixed(const string& label) const;
    unsigned findOrig(const string& label) const;

    string Sheet::strHeader() const;
    string Sheet::strLinks() const;
    string Sheet::strPlays() const;


  public:

    Sheet();

    ~Sheet();

    void reset();

    bool read(const string& fname);

    string str() const;
};

#endif

