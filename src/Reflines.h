/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFLINES_H
#define BRIDGE_REFLINES_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#pragma warning(pop)

#include "RefLine.h"

using namespace std;

class Buffer;


class RefLines
{
  private:

    enum RefControl
    {
      ERR_REF_STANDARD = 0,
      ERR_REF_SKIP = 1,
      ERR_REF_NOVAL = 2,
      ERR_REF_OUT_COCO = 3,
      ERR_REF_OUT_OOCC = 4
    };

    vector<RefLine> lines;

    RefControl control;
    RefComment headerComment;

    unsigned bufferLines;
    unsigned numHands;
    unsigned numBoards;


    bool parseComment(
      const string& fname,
      string& line);

    void checkEntries(
      const RefEntry& re,
      const RefEntry& ra) const;


  public:

    RefLines();

    ~RefLines();

    vector<RefLine>::const_iterator begin() const { return lines.begin(); }
    vector<RefLine>::const_iterator end() const { return lines.end(); }

    void reset();

    void setFileData(
      const unsigned bufIn,
      const unsigned numHandsIn,
      const unsigned numBoardsIn);

    void read(const string& fname);

    bool hasComments() const;
    bool skip() const;
    bool validate() const;

    void setOrder(const BoardOrder order);
    bool orderCOCO() const;
    bool orderOOCC() const;
    BoardOrder order() const;

    bool getHeaderEntry(
      CommentType& cat,
      RefEntry& re) const;

    void getHeaderData(
      unsigned& nl,
      unsigned& nh,
      unsigned& nb) const;

    bool getControlEntry(
      CommentType& cat,
      RefEntry& re) const;
    
    void checkHeader() const;
};

#endif

