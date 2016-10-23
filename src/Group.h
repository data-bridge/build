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

    string nameVal;

    Format formatVal;

    struct SegmentPairs
    {
      unsigned extNo;
      Segment segment;
    };

    unsigned len;

    vector<SegmentPairs> segmentPairs;


  public:

    Group();

    ~Group();

    void reset();

    void setName(const string& fname);
    string name() const;

    void setFormat(const Format format);
    Format format() const;

    Segment * make(const unsigned no);

    Segment * get(const unsigned no);

    unsigned size() const;

    unsigned count();

    bool operator == (const Group& group2) const;

    bool operator != (const Group& group2) const;
};

#endif

