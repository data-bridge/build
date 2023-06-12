/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_LOCATION_H
#define BRIDGE_LOCATION_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#pragma warning(pop)

#include "bconst.h"

using namespace std;


class Location
{
  private:

    string locGeneral;
    string locSpecific;

    void setWithSeparator(
      const string& text,
      const string& separator);

    string strCore(const string& separator) const;


  public:

    Location();

    ~Location();

    void reset();

    void set(
      const string& text,
      const Format f);

    bool operator == (const Location& location2) const;

    bool operator != (const Location& location2) const;

    string str(const Format f) const;
};

#endif

