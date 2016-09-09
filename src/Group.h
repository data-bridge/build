/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_GROUP_H
#define BRIDGE_GROUP_H

#include "Segment.h"
#include "bconst.h"
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

    vector<segmentPairType> segments;

    const Segment * GetSegmentReadOnly(const unsigned no) const;


  public:

    Group();

    ~Group();

    void Reset();

    bool MakeSegment(const unsigned no);

    Segment * GetSegment(const unsigned no);

    bool operator == (const Group& g2) const;

    bool operator != (const Group& g2) const;

};

#endif

