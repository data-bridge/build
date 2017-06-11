/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_GROUP_H
#define BRIDGE_GROUP_H

#include <string>
#include <list>

#include "Segment.h"

using namespace std;


class Group
{
  private:

    string nameVal;
    Format formatVal;
    list<Segment> segments;
    bool flagCOCO;


  public:

    Group();
    ~Group();
    void reset();

    list<Segment>::const_iterator begin() const { return segments.begin(); }
    list<Segment>::const_iterator end() const { return segments.end(); }

    void setName(const string& fname);
    string name() const;

    void setFormat(const Format format);
    Format format() const;

    Segment * make();

    void setCOCO(const Format format = BRIDGE_FORMAT_SIZE);
    bool isCOCO() const;

    unsigned count() const;
    unsigned countBoards() const;

    bool operator == (const Group& group2) const;
    bool operator != (const Group& group2) const;
};

#endif

