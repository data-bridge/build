/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_LOCATION_H
#define BRIDGE_LOCATION_H

#include <string>

enum Format: unsigned;

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

    void reset();

    void set(
      const string& text,
      const Format f);

    bool operator == (const Location& location2) const;

    bool operator != (const Location& location2) const;

    string str(const Format f) const;
};

#endif

