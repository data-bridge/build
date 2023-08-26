/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <cassert>

#include "../Composites.h"

#include "../../stats/RowData.h"
#include "../../util/parse.h"

#include "Sigmoid.h"
#include "RowProbInfo.h"
#include "PassTable.h"



PassTable::PassTable()
{
  PassTable::reset();
}


void PassTable::reset()
{
  rows.clear();
}


map<string, CompositeParams> PassTable::CompParamLookup = {};

void PassTable::setStatic()
{
  for (unsigned i = 0; i < CompInfo.size(); i++)
  {
    PassTable::CompParamLookup[CompInfo[i].textShort] = 
      static_cast<CompositeParams>(i);
  }
}


CompositeParams PassTable::lookupComp(
  const string& comp,
  const string& fname) const
{
  auto it = CompParamLookup.find(comp);
  if (it == CompParamLookup.end())
  {
    cout << "File " << fname << ": no valid component " << comp << "\n";
    assert(false);
  }
  return it->second;
}


unsigned PassTable::getUnsigned(
  const string& comp,
  const string& fname) const
{
  unsigned uv;
  if (! str2unsigned(comp, uv))
  {
    cout << "File " << fname << ": not unsigned " << comp << "\n";
    assert(false);
  }
  return uv;
}


void PassTable::parseProbInfo(
  const string& fname,
  const string& str, 
  RowProbInfo& rowProbInfo) const
{
  if (str.find("sigmoid(") != string::npos)
  {
    // Parse the format "sigmoid(float1, float2, float3, float4)".

    string strCopy = str;
    strCopy.erase(
      remove_if(strCopy.begin(), strCopy.end(), 
        [](char c){ return std::isspace(c); }), strCopy.end());

    if (strCopy.back() != ')')
    {
      cout << "String " << str << " does not end on ')'\n";
      assert(false);
    }

    string strCore = strCopy.substr(8, strCopy.size()-9);
    
    vector<string> tokens;
    tokenize(strCore, tokens, ",");
    assert(tokens.size() == 4);

    vector<float> floats(tokens.size());
    for (unsigned i = 0; i < tokens.size(); i++)
    {
      if (! str2float(tokens[i], floats[i]))
      {
        cout << 
          "File " << fname << ": " <<
          tokens[i] << " is not a floating point number\n";
        assert(false);
      }
    }

    rowProbInfo.prob = 0.;
    rowProbInfo.probAdder = 0.;
    rowProbInfo.algoFlag = true;
    rowProbInfo.sigmoidData.mean = floats[0];
    rowProbInfo.sigmoidData.divisor = floats[1];
    rowProbInfo.sigmoidData.intercept = floats[2];
    rowProbInfo.sigmoidData.slope = floats[3];

    // TODO Can we use an extern one?
    Sigmoid sigmoid;
    rowProbInfo.sigmoidData.crossover = sigmoid.calcSigmoid(
      rowProbInfo.sigmoidData, floats[2]);
    return;
  }

  // Parse a single float.
  if (! str2float(str, rowProbInfo.prob))
  {
    cout << 
      "File " << fname << ": " <<
      str << " is not a floating point number\n";
    assert(false);
  }

  rowProbInfo.algoFlag = false;
}


size_t PassTable::parseComponentsFrom(
  PassRow& row,
  const vector<string>& components,
  const size_t index,
  const string& fname) const
{
  // Return the next unparsed index.
  assert(index+2 < components.size());

  const CompositeParams tag = 
    PassTable::lookupComp(components[index], fname);

  const string oper = components[index+1];
  const unsigned uv1 = PassTable::getUnsigned(components[index+2], fname);

  if (oper == "in" || oper == "notin")
  {
    assert(index+3 < components.size());

    const unsigned uv2 = PassTable::getUnsigned(components[index+3], fname);

    if (oper == "in")
      row.addRange(tag, uv1, uv2);
    else
      row.addOutside(tag, uv1, uv2);
    
    return index+4;
  }
  else
  {
    if (oper == ">=")
      row.addLower(tag, uv1);
    else if (oper == "<=")
      row.addUpper(tag, uv1);
    else if (oper == "==")
      row.addExact(tag, uv1);
    else
      assert(false);
    
    return index+3;
  }
}


unsigned PassTable::getWhen(
  const vector<string>& components,
  const string& fname) const
{
  for (unsigned index = 0; index < components.size(); index++)
  {
    if (components[index] == "when")
      return index;
  }

  cout << "File " << fname << ": no 'when' in modify line\n";
  assert(false);
  return 0;
}


void PassTable::parseModifyLine(
  const vector<string>& components,
  const float prob,
  const string& fname)
{
  // Parse a new line starting with modify.
  // If the new information applies to exactly one inactive line,
  // we modify that line.
  // If not, we modify all active lines to which it applies.
  // Otherwise, we are presently confused.
  
  // First we parse components into a row called rowCore and
  // another one called rowModif.

  RowEntry reCore;
  size_t index = 1;

  const size_t numWhen = PassTable::getWhen(components, fname);
  while (index < numWhen)
    index = PassTable::parseComponentsFrom(reCore.row, components,
      index, fname);

  RowEntry reModif;
  index = numWhen + 1;
  while (index < components.size())
    index = PassTable::parseComponentsFrom(reModif.row, components,
      index, fname);

  // Then we check the active and inactive matches.
  list<RowEntry *> listActive, listInactive;
  size_t maxInactiveTermCount = 0;

  for (auto& re: rows)
  {
    if (re.row.contains(reCore.row) && ! re.row.alreadyUses(reModif.row))
    {
      if (re.activeFlag)
        listActive.push_back(&re);
      else
      {
        listInactive.push_back(&re);
        maxInactiveTermCount = max(maxInactiveTermCount, re.row.count());
      }
    }
  }

  if (listActive.empty())
  {
    if (listInactive.empty())
    {
      cout << "File " << fname << ": Nothing to modify";
      assert(false);
    }

    for (auto& inactivePtr: listInactive)
    {
      // Only modify the maximally long, inactive ones.
      if (inactivePtr->row.count() < maxInactiveTermCount)
        continue;

      rows.emplace_back(* inactivePtr);
      auto& reNew = rows.back();
      reNew.row.add(reModif.row);
      reNew.row.addProb(prob);
      reNew.activeFlag = true;
      reNew.rowNo = rows.size()-1;
    }
  }
  else
  {
    // There are active matches.
    for (auto& activePtr: listActive)
    {
      // Make a copy and turn the original one inactive.
      rows.emplace_back(* activePtr);
      activePtr->activeFlag = false;

      auto& reNew = rows.back();
      reNew.row.add(reModif.row);
      reNew.row.addProb(prob);
      reNew.activeFlag = true;
      reNew.rowNo = rows.size()-1;
    }
  }
}


void PassTable::parsePrimaryLine(
  const vector<string>& components,
  const RowProbInfo& rowProbInfo,
  const string& fname)
{
  rows.emplace_back(RowEntry());

  RowEntry& re = rows.back();
  re.row.setProb(rowProbInfo);
  re.activeFlag = true;
  re.rowNo = rows.size()-1;

  size_t index = 0;
  while (index < components.size())
    index = PassTable::parseComponentsFrom(re.row, components, 
      index, fname);
}


void PassTable::readFile(const string& fname)
{
  ifstream fin(fname.c_str());
  assert(fin.good());

  string line;
  vector<string> tokens, components;
  RowProbInfo rowProbInfo;

  while (getline(fin, line))
  {
    size_t first = line.find_first_not_of(" \t\n\r\f\v");
    if (first == string::npos || (line.size() > 0 && line[0] == '#'))
      continue;
    
    tokens.clear();
    tokenize(line, tokens, ":");
    assert(tokens.size() == 2);

    PassTable::parseProbInfo(fname, tokens[1], rowProbInfo);

    components.clear();
    tokenize(tokens[0], components, " ");

    if (components.size() == 0)
    {
      cout << 
        "File " << fname << ": nothing before the colon in " <<
        line << "\n";
      assert(false);
    }

    if (components[0] == "modify")
    {
      assert(! rowProbInfo.algoFlag);
      PassTable::parseModifyLine(components, rowProbInfo.prob, fname);
    }
    else
      PassTable::parsePrimaryLine(components, rowProbInfo, fname);
  }

  fin.close();

  for (auto& row: rows)
    row.row.saturate();
}


bool PassTable::empty() const
{
  return rows.empty();
}


float PassTable::lookup(const Valuation& valuation) const
{
  PassMatch match;

  for (auto& rowEntry: rows)
  {
    if (rowEntry.activeFlag)
    {
      match = rowEntry.row.match(valuation);
      if (match.matchFlag)
        return match.prob;
    }
  }

  assert(false);
  return 0.;
}


#include "../Valuation.h"
PassTableMatch PassTable::lookupFull(const Valuation& valuation) const
{
  PassMatch match;
  PassTableMatch tableMatch;

  for (auto& rowEntry: rows)
  {
    if (rowEntry.activeFlag)
    {
      match = rowEntry.row.match(valuation);
      if (match.matchFlag)
      {
        tableMatch.prob = match.prob;
        tableMatch.rowNo = rowEntry.rowNo;
        return tableMatch;
      }
    }
  }

  cout << "About to fail table lookup\n";
  cout << PassTable::str() << "\n";
  cout << valuation.str() << "\n";

  assert(false);
  return tableMatch;
}


void PassTable::getProbVector(vector<float>& rowProbs) const
{
  // This is semi-internal, but is used to check whether we hit
  // the rows with roughly the expected probabilities.
  rowProbs.resize(rows.size());

  unsigned i = 0;
  for (auto& rowEntry: rows)
  {
    rowProbs[i] = rowEntry.row.getProb();
    i++;
  }
}


void PassTable::getRowData(vector<RowData>& rowData) const
{
  rowData.resize(rows.size());

  unsigned i = 0;
  for (auto& rowEntry: rows)
  {
    rowEntry.row.getRowData(rowData[i]);
    i++;
  }
}


string PassTable::str() const
{
  string s;
  for (auto& rowEntry: rows)
  {
    s += (rowEntry.activeFlag ? "Active\n" : "Inactive\n");
    s += rowEntry.row.str();
  }
  return s;
}

