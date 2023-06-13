/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This class uses classes TextStat and TextDatum.
   It can generate a summary of labels and formats like this:

     Param.       LIN     max
     Title         49      49
     Event         19      19
     Session        2       2
     Teams         21      21
     Players       15      15
     Auction       38      38

   If examplesFlag is set, it can also generate one output per
   label like this (one example per character length):

     Event      count  source                  example
     19            32  26288.lin               Mixed Teams R4_7 T2
     18            21  26282.lin               mixed team R1_7 T2
     17            71  26092.lin               Final segment 2_3
     ...
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

