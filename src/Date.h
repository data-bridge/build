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

    struct DateType
    {
      unsigned year;
      unsigned month;
      unsigned day;
    };

    DateType date;

    void checkDate() const;

    void setLIN(const string& t);
    void setPBN(const string& t);
    void setRBN(const string& t);
    void setTXT(const string& t);

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
      const formatType f);

    bool operator == (const Date& d2) const;

    bool operator != (const Date& d2) const;

    string asString(const formatType f) const;
};

#endif

