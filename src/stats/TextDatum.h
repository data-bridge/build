/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TEXTDATUM_H
#define BRIDGE_TEXTDATUM_H

#include <string>

using namespace std;


class TextDatum
{
  private:

    string source;
    string example;
    size_t count;

  public:

    void reset();

    void add(
      const string& sourceIn,
      const string& exampleIn,
      const size_t countIn = 1);

    void operator += (const TextDatum& td2);

    bool empty() const;
      
    string strHeader() const;

    string str() const;
};

#endif

