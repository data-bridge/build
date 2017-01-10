/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TEXTSTATS_H
#define BRIDGE_TEXTSTATS_H

#include <iostream>
#include <vector>

#include "bconst.h"


using namespace std;


class TextStats
{
  private:

    struct TextDatum
    {
      unsigned count;
      string source;
      string example;
    };

    struct TextStat
    {
      vector<TextDatum> datum;
      unsigned count;
    };

    vector<vector<TextStat>> stats;

    void setTables();

    string posOrDash(const unsigned u) const;

    void printDetails(
      const unsigned label,
      ostream& fstr) const;

  public:

    TextStats();

    ~TextStats();

    void reset();

    void add(
      const string& text,
      const string& source,
      const Label label,
      const Format format);

    void add(
      const unsigned len,
      const string& source,
      const Label label,
      const Format format);

    void operator += (const TextStats& statsIn);
      
    void print(
      ostream& fstr,
      const bool detailsFlag = false) const;
};

#endif

