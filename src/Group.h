/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_GROUP_H
#define BRIDGE_GROUP_H

#include <string>

#include "Segment.h"

using namespace std;


class Group
{
  private:

    string nameVal;

    Format formatVal;

    vector<Segment> segments;


  public:

    Group();

    ~Group();

    vector<Segment>::iterator begin() { return segments.begin(); }

    vector<Segment>::iterator end() { return segments.end(); }

    void reset();

    void setName(const string& fname);
    string name() const;

    void setFormat(const Format format);
    Format format() const;

    Segment * make();

    unsigned count();

    bool operator == (const Group& group2) const;

    bool operator != (const Group& group2) const;
};

#endif

