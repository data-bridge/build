/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_PASSTABLE_H
#define BRIDGE_PASSTABLE_H

#include <map>
#include <list>
#include <string>

#include "PassRow.h"

using namespace std;

class Valuation;

enum CompositeParams: unsigned;

struct PassTableMatch
{
  float prob;
  size_t rowNo;
};


class PassTable
{
  private:

    static map<string, CompositeParams> CompParamLookup;

    struct RowEntry
    {
      // A row may have to be present in order still to be modifiable.
      PassRow row;
      bool activeFlag;
      size_t rowNo;
    };

    list<RowEntry> rows;


    CompositeParams lookupComp(
      const string& comp,
      const string& fname) const;

    unsigned getUnsigned(
      const string& comp,
      const string& fname) const;

    size_t parseComponentsFrom(
      PassRow& row,
      const vector<string>& components,
      const size_t index,
      const string& fname) const;

    unsigned getWhen(
      const vector<string>& components,
      const string& fname) const;
      
    void parseModifyLine(
      const vector<string>& components,
      const float probAdder,
      const string& fname);

    void parsePrimaryLine(
      const vector<string>& components,
      const float prob,
      const string& fname);


  public:


    PassTable();

    void reset();

    static void setStatic();

    void readFile(const string& fname);

    bool empty() const;

    float lookup(const Valuation& valuation) const;

    PassTableMatch lookupFull(const Valuation& valuation) const;

    void getProbVector(vector<float>& rowProbs) const;

    string str() const;
};

#endif

