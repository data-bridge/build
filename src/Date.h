/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DATE_H
#define BRIDGE_DATE_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#pragma warning(pop)

#include "bconst.h"

using namespace std;


class Date
{
  private:

    unsigned year;
    unsigned month;
    unsigned day;

    void check() const;

    void setTriple(
      const string& y,
      const string& m,
      const string& d);
    void setLIN(const string& text);
    void setPBN(const string& text);
    void setRBN(const string& text);
    void setTXT(const string& text);

    string strLIN() const;
    string strPBN() const;
    string strRBNCore() const;
    string strRBN() const;
    string strRBX() const;
    string strTXT() const;


  public:

    Date();

    ~Date();

    void reset();

    void set(
      const string& text,
      const Format f);

    bool operator == (const Date& date2) const;

    bool operator != (const Date& date2) const;

    string str(const Format format) const;
};

#endif

