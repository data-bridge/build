/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TEXTSTATS_H
#define BRIDGE_TEXTSTATS_H

#include <vector>

#include "TextStat.h"

enum Format: unsigned;
enum Label: unsigned;

using namespace std;


class TextStats
{
  private:

    vector<vector<TextStat>> stats;

    void strExamplesPrepare(
      TextStat& labelSum,
      const Label label) const;

    void strPrepare(
      vector<size_t>& activeFormats,
      vector<size_t>& labelMaxima) const;

    string strExamples(const Label label) const;

    string strParamHeader(const vector<size_t>& activeFormats) const;

    string strParams(
      const vector<size_t>& activeFormats,
      const vector<size_t>& labelMaxima) const;

  public:

    TextStats();

    ~TextStats();

    void reset();

    void add(
      const string& example,
      const string& source,
      const Label label,
      const Format format);

    void add(
      const unsigned len,
      const string& source,
      const Label label,
      const Format format);

    void operator += (const TextStats& statsIn);
      
    string str(const bool examplesFlag = false) const;
};

#endif

