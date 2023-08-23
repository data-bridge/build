/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

// We use a 1D list where the index is
// 16 * distribution index + 4 * relative player index + relative vul.

#ifndef BRIDGE_PASSTABLES_H
#define BRIDGE_PASSTABLES_H

#include <vector>
#include <string>

#include "PassTable.h"

using namespace std;

struct RowData;
class Valuation;
class Distribution;
struct AllStats;


class PassTables
{
  private:

    map<string, unsigned> Positions;
    map<string, unsigned> Vulnerabilities;

    vector<PassTable> tables;


    unsigned string2pos(const string& text) const;

    unsigned string2vul(const string& text) const;

    void makeFileList(
      const string& dir,
      list<string>& files) const;

    void readAnyPosVul(
      const string& fname,
      const vector<string>& parts,
      const Distribution& distribution,
      vector<AllStats>& allStatsList);

    void readAnyVul(
      const string& fname,
      const vector<string>& parts,
      const Distribution& distribution,
      vector<AllStats>& allStatsList);

    void readOne(
      const string& fname,
      const vector<string>& parts,
      const Distribution& distribution,
      vector<AllStats>& allStatsList);


  public:


    PassTables();

    void reset();

    void read(vector<AllStats>& allStatsList);

    float lookup(
      const unsigned distIndex,
      const unsigned relPlayerIndex,
      const unsigned relVulIndex,
      const Valuation& valuation) const;

    PassTableMatch lookupFull(
      const unsigned distIndex,
      const unsigned relPlayerIndex,
      const unsigned relVulIndex,
      const Valuation& valuation) const;

    void getProbVector(
      const unsigned distIndex,
      const unsigned relPlayerIndex,
      const unsigned relVulIndex,
      vector<float>& rowProbs) const;

    void getRowData(
      const unsigned distIndex,
      const unsigned relPlayerIndex,
      const unsigned relVulIndex,
      vector<RowData>& rowData) const;
};

#endif

