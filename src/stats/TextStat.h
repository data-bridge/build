/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TEXTSTAT_H
#define BRIDGE_TEXTSTAT_H

#include <vector>
#include <string>

#include "TextDatum.h"

using namespace std;


class TextStat
{
  private:

    vector<TextDatum> datum;

    unsigned count;

  public:

    void reset();

    void add(
      const string& source,
      const string& example);

    void add(
      const string& source,
      const size_t len);

    void operator += (const TextStat& ts2);

    bool empty() const;

    size_t last_used() const;
      
    string strHeader(const string& label) const;

    string str() const;
};

#endif

