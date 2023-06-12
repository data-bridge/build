/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#pragma warning(pop)

#include "DuplStats.h"

#include "../Group.h"
#include "../RefLines.h"
#include "../Bexcept.h"

#define HASH_SIZE 4096 // Relies on <= 12-bit hash values


DuplStats::DuplStats()
{
  DuplStats::reset();
}


DuplStats::~DuplStats()
{
}


void DuplStats::reset()
{
  statList.clear();
  hashedList.clear();
  activeList = nullptr;

  hashedList.resize(HASH_SIZE);
}


void DuplStats::set(
  const Group * group,
  const Segment * segment,
  const unsigned segNo,
  const RefLines * reflines)
{
  statList.emplace_back(DuplElem());
  activeList = &statList.back();
  activeList->activeFlag = true;
  activeList->stat.set(group, segment, segNo, reflines);
}


void DuplStats::append(const int hashVal)
{
  if (activeList == nullptr)
    THROW("No active list");

  activeList->stat.append(hashVal);
}


void DuplStats::sortActive()
{
  if (activeList == nullptr)
    THROW("No active list");

  activeList->stat.sort();

  unsigned f = activeList->stat.first();
  hashedList[f].push_back(activeList);
}


void DuplStats::operator += (const DuplStats& dupl2)
{
  for (unsigned i = 0; i < HASH_SIZE; i++)
    for (unsigned j = 0; j < dupl2.hashedList[i].size(); j++)
      hashedList[i].push_back(dupl2.hashedList[i][j]);
}


void DuplStats::sortOverall()
{
  for (unsigned i = 0; i < HASH_SIZE; i++)
  {
    sort(hashedList[i].begin(), hashedList[i].end(),
      [](const DuplElem * a, const DuplElem * b)->bool
      {
        return a->stat.lexLessThan(b->stat);
      });
  }
}


string DuplStats::strSame() const
{
  string st;
  for (unsigned i = 0; i < HASH_SIZE; i++)
  {
    const size_t lh = hashedList[i].size();
    if (lh == 0)
      continue;
    for (unsigned j = 0; j < lh-1; j++)
    {
      if (! hashedList[i][j]->activeFlag)
        continue;

      if (hashedList[i][j]->stat.sameOrigin(hashedList[i][j+1]->stat))
      {
        // Pavlicek files with the same origin: Take the first one.
        hashedList[i][j]->activeFlag = false;
        continue;
      }

      for (unsigned k = j+1; k < lh; k++)
      {
        if (! hashedList[i][k]->activeFlag)
          continue;
        
        const DuplStat& stat1 = hashedList[i][j]->stat;
        const DuplStat& stat2 = hashedList[i][k]->stat;
        if (stat1 == stat2)
        {
          // Take the later one.
          st += stat1.str() + "---------------\n";
          st += stat2.str(stat1) + "---------------\n";
          st += stat1.strSuggest(true) + "\n\n";

          hashedList[i][j]->activeFlag = false;
        }
      }
    }
  }
  if (st == "")
    return "";
  else
    return "Duplicates\n\n" + st;
}


string DuplStats::strSubset() const
{
  string st;
  for (size_t i = 0; i < HASH_SIZE; i++)
  {
    const size_t lh = hashedList[i].size();
    if (lh == 0)
      continue;

    for (size_t j = 0; j < lh; j++)
    {
      if (! hashedList[i][j]->activeFlag)
        continue;

      for (size_t i2 = 0; i2 <= i; i2++)
      {
        // i's can only be subsets of ones with lower first.
        for (size_t k = 0; k < hashedList[i2].size(); k++)
        {
          if (! hashedList[i2][k]->activeFlag)
            continue;
          if (i == i2 && j == k)
            continue;
        
          const DuplStat& stat1 = hashedList[i][j]->stat;
          const DuplStat& stat2 = hashedList[i2][k]->stat;
          if (stat1 <= stat2)
          {
            // Take the later one.
            st += stat1.str() + "---------------\n";
            st += stat2.str(stat1) + "---------------\n";
            st += stat1.strSuggest(false) +  "\n\n";

            hashedList[i][j]->activeFlag = false;
          }
        }
      }
    }
  }
  if (st == "")
    return "";
  else
    return "Strict subsets\n\n" + st;
}


void DuplStats::print(ostream& fstr) const
{
  fstr << DuplStats::strSame();
  fstr << DuplStats::strSubset();
}

