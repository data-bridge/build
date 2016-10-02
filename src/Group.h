/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_GROUP_H
#define BRIDGE_GROUP_H

#include "Segment.h"
#include <string>

using namespace std;


class Group
{
  private:

    struct segmentPairType
    {
      unsigned extNo;
      Segment segment;
    };

    unsigned len;

    string fileName;

    formatType formatOrigin;

    vector<segmentPairType> segments;

    const Segment * GetSegmentReadOnly(const unsigned no) const;


  public:

    Group();

    ~Group();

    void Reset();

    void SetFileName(const string& fname);

    void SetInputFormat(const formatType f);

    bool MakeSegment(const unsigned no);

    Segment * GetSegment(const unsigned no);

    string GetFileName() const;

    formatType GetInputFormat() const;

    unsigned GetCount();

    unsigned GetLength() const;

    bool operator == (const Group& g2) const;

    bool operator != (const Group& g2) const;

};

#endif

