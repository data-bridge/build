/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DATE_H
#define BRIDGE_DATE_H

#include "bconst.h"
#include <string>

using namespace std;


class Date
{
  private:

    struct dateType
    {
      unsigned year;
      unsigned month;
      unsigned day;
    };

    dateType date;


  public:

    Date();

    ~Date();

    void Reset();

    bool Set(
      const string& t,
      const formatType f);

    bool operator == (const Date& d2) const;

    bool operator != (const Date& d2) const;

    string AsString(const formatType f) const;
};

#endif

