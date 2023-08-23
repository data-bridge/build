/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <list>
#include <filesystem>
#include <cassert>

#include "../Valuation.h"
#include "../Distribution.h"
#include "../Distributions.h"

#include "../../include/bridge.h"
#include "../../util/parse.h"

#include "PassTables.h"


PassTables::PassTables()
{
  PassTables::reset();
}


void PassTables::reset()
{
  tables.clear();

  Positions.clear();
  Positions["first"] = 0;
  Positions["second"] = 1;
  Positions["third"] = 2;
  Positions["fourth"] = 3;

  Vulnerabilities.clear();
  Vulnerabilities["None"] = BRIDGE_VUL_REL_NONE;
  Vulnerabilities["Both"] = BRIDGE_VUL_REL_BOTH;
  Vulnerabilities["We"] = BRIDGE_VUL_REL_ONLY_WE;
  Vulnerabilities["They"] = BRIDGE_VUL_REL_ONLY_THEY;
}


void PassTables::makeFileList(
  const string& dir,
  list<string>& files) const
{
  for (const auto& entry: filesystem::recursive_directory_iterator(dir)) 
    if (entry.is_regular_file() && entry.path().extension() == ".txt")
      files.push_back(entry.path().string());
}


unsigned PassTables::string2pos(const string& text) const
{
  auto it = Positions.find(text);
  if (it == Positions.end())
  {
    cout << "Position " << text << " not found\n";
    assert(false);
  }
  return it->second;
}


unsigned PassTables::string2vul(const string& text) const
{
  auto it = Vulnerabilities.find(text);
  if (it == Vulnerabilities.end())
  {
    cout << "Vulnerability " << text << " not found\n";
    assert(false);
  }
  return it->second;
}


void PassTables::readAnyPosVul(
  const string& fname,
  const vector<string>& parts,
  const Distribution& distribution)
{
  const unsigned distNo = distribution.name2number(parts[1]);
  assert(parts[2] == "Any");
  assert(parts[3] == "txt");

  const unsigned index = 16 * distNo;
  tables[index].readFile(fname);

  // Make up the other positions and vulnerabilities.
  for (unsigned i = index+1; i < index+16; i++)
    tables[i] = tables[index];
}


void PassTables::readAnyVul(
  const string& fname,
  const vector<string>& parts,
  const Distribution& distribution)
{
  const unsigned distNo = distribution.name2number(parts[1]);
  const unsigned posNo = PassTables::string2pos(parts[2]);
  assert(parts[3] == "Any");
  assert(parts[4] == "txt");

  const unsigned index = 16 * distNo + 4 * posNo;
  tables[index].readFile(fname);

  // Make up the other vulnerabilities.
  for (unsigned i = index+1; i < index+4; i++)
    tables[i] = tables[index];
}


void PassTables::readOne(
  const string& fname,
  const vector<string>& parts,
  const Distribution& distribution)
{
  const unsigned distNo = distribution.name2number(parts[1]);
  const unsigned posNo = PassTables::string2pos(parts[2]);
  const unsigned vulNo = PassTables::string2vul(parts[3]);
  assert(parts[4] == "txt");

  const unsigned index = 16 * distNo + 4 * posNo + vulNo;
  tables[index].readFile(fname);
}


void PassTables::read()
{
  tables.resize(16 * DIST_SIZE);

  list<string> files;
  PassTables::makeFileList(
    "../../Valet3/scripts/cluster/passes/tables", files);
  Distribution distribution;

  vector<string> parts;
  unsigned tablesIndex;
  bool firstFlag = true;

  for (auto& fname: files)
  {
    parts.clear();
    tokenize(fname, parts, "/\\.");

    if (firstFlag)
    {
      tablesIndex = 0;
      while (tablesIndex < parts.size() && parts[tablesIndex] != "tables")
        tablesIndex++;
      firstFlag = false;
    }

    parts.erase(parts.begin(), parts.begin() + tablesIndex);

    if (parts.size() == 4)
      PassTables::readAnyPosVul(fname, parts, distribution);
    else if (parts.size() == 5)
    {
      if (parts[3] == "Any")
        PassTables::readAnyVul(fname, parts, distribution);
      else
        PassTables::readOne(fname, parts, distribution);
    }
    else
    {
      cout << "Filename " << fname << " could not be parsed\n";
      assert(false);
    }
  }

  for (unsigned i = 0; i < tables.size(); i++)
  {
    if (tables[i].empty())
    {
      cout << "Table " << i << " is empty\n";
      assert(false);
    }
  }
}


float PassTables::lookup(
  const unsigned distIndex,
  const unsigned relPlayerIndex,
  const unsigned relVulIndex,
  const Valuation& valuation) const
{
  const unsigned index = 16 * distIndex + 4 * relPlayerIndex + relVulIndex;
  return tables[index].lookup(valuation);
}


PassTableMatch PassTables::lookupFull(
  const unsigned distIndex,
  const unsigned relPlayerIndex,
  const unsigned relVulIndex,
  const Valuation& valuation) const
{
  const unsigned index = 16 * distIndex + 4 * relPlayerIndex + relVulIndex;
  return tables[index].lookupFull(valuation);
}


void PassTables::getProbVector(
  const unsigned distIndex,
  const unsigned relPlayerIndex,
  const unsigned relVulIndex,
  vector<float>& rowProbs) const
{
  const unsigned index = 16 * distIndex + 4 * relPlayerIndex + relVulIndex;
  tables[index].getProbVector(rowProbs);
}


void PassTables::getRowData(
  const unsigned distIndex,
  const unsigned relPlayerIndex,
  const unsigned relVulIndex,
  vector<RowData>& rowData) const
{
  const unsigned index = 16 * distIndex + 4 * relPlayerIndex + relVulIndex;
  tables[index].getRowData(rowData);
}

