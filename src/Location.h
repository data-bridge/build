/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_LOCATION_H
#define BRIDGE_LOCATION_H

#include "bconst.h"
#include <string>

using namespace std;


class Location
{
  private:

    struct LocationType
    {
      string general;
      string specific;
    };

    LocationType location;

    string asRBN(const string& sep) const;



  public:

    Location();

    ~Location();

    void reset();

    void set(
      const string& t,
      const formatType f);

    bool operator == (const Location& l2) const;

    bool operator != (const Location& l2) const;

    string asString(const formatType f) const;
};

#endif

