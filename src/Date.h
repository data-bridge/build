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

    unsigned year;
    unsigned month;
    unsigned day;

    void check() const;

    void setLIN(const string& text);
    void setPBN(const string& text);
    void setRBN(const string& text);
    void setTXT(const string& text);

    string asLIN() const;
    string asPBN() const;
    string asRBNCore() const;
    string asRBN() const;
    string asRBX() const;
    string asTXT() const;


  public:

    Date();

    ~Date();

    void reset();

    void set(
      const string& t,
      const Format f);

    bool operator == (const Date& date2) const;

    bool operator != (const Date& date2) const;

    string asString(const Format f) const;
};

#endif

