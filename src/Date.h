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

    void StringToMonth(const string& m);

    bool CheckDate() const;

    bool SetLIN(const string& t);
    bool SetPBN(const string& t);
    bool SetRBN(const string& t);
    bool SetTXT(const string& t);

    string AsLIN() const;
    string AsPBN() const;
    string AsRBNCore() const;
    string AsRBN() const;
    string AsRBX() const;
    string AsTXT() const;


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

