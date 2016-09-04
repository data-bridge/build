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

    struct locationType
    {
      string general;
      string specific;
    };

    locationType location;



  public:

    Location();

    ~Location();

    void Reset();

    bool Set(
      const string& t,
      const formatType f);

    bool operator == (const Location& l2) const;

    bool operator != (const Location& l2) const;

    string AsString(const formatType f) const;
};

#endif

