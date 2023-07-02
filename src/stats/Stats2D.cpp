/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <assert.h>

#include "Stats2D.h"

const vector<string> FIELD_NAMES = { "Count", "Hits" };


Stats2D::Stats2D()
{
  Stats2D::reset();
}


void Stats2D::reset()
{
  info1.reset();
  info2.reset();
  counts.clear();
}


void Stats2D::set1(const StatsInfo& info1In)
{
  info1 = info1In;
  counts.resize(info1.length);
  counts1.resize(info1.length);
}


void Stats2D::set2(const StatsInfo& info2In)
{
  info2 = info2In;
  for (auto& c: counts)
    c.resize(info2.length);
  counts2.resize(info2.length);
}


void Stats2D::add(
  const unsigned param1,
  const unsigned param2,
  const bool flag)
{
  assert(param1 >= info1.low);
  assert(param2 >= info2.low);

  const unsigned index1 = param1 - info1.low;
  const unsigned index2 = param2 - info2.low;

  assert(index1 < info1.length);
  assert(index2 < info2.length);

  counts[index1][index2].add(flag);
  counts1[index1].add(flag);
  counts2[index2].add(flag);
}


void Stats2D::operator += (const Stats2D& s2)
{
  assert(info1.length == s2.info1.length);
  assert(info2.length == s2.info2.length);

  for (unsigned i = 0; i < info1.length; i++)
    for (unsigned j = 0; j < info2.length; j++)
      counts[i][j] += s2.counts[i][j];
}


bool Stats2D::findLimits(
  const vector<Count1D>& countsIn,
  unsigned& indexLow,
  unsigned& indexHigh) const
{
  // Find the lowest, used index.
  indexLow = numeric_limits<unsigned>::max();
  for (unsigned i = 0; i < countsIn.size(); i++)
  {
    if (countsIn[i].count)
    {
      indexLow = i;
      break;
    }
  }

  if (indexLow == numeric_limits<unsigned>::max())
    return false;

  // Find the highest, used index.
  indexHigh = 0;
  for (unsigned i = 0; i < countsIn.size(); i++)
  {
    if (countsIn[i].count)
      indexHigh = i;
  }
  return true;
}


string Stats2D::strHeader(
  const string& shead,
  const unsigned index2Low,
  const unsigned index2High) const
{
  stringstream ss;
  ss << setw(12) << right << "";

  for (unsigned j = index2Low; j <= index2High; j++)
  {
    if (info2.factor == 1)
      ss << setw(6) << j + info2.low;
    else
      ss << setw(6) << fixed << setprecision(2) <<
        static_cast<float>(j + info2.low) /
        static_cast<float>(info2.factor);
  }
  ss << "\n";

  ss << setw(6) << "Value" <<
    setw(6) << shead;
  for (unsigned j = index2Low; j <= index2High; j++)
    ss << setw(6) << counts2[j].count;
  ss << "\n";

  return ss.str();
}


string Stats2D::strTable(
  const unsigned tableIndex,
  const unsigned index1Low,
  const unsigned index1High,
  const unsigned index2Low,
  const unsigned index2High) const
{
  stringstream ss;
  ss << info1.name << " vs. " << info2.name << "\n\n";
  ss << Stats2D::strHeader(FIELD_NAMES[tableIndex], 
    index2Low, index2High);

  for (unsigned i = index1Low; i <= index1High; i++)
  {
    if (info1.factor == 1)
    {
      ss << setw(6) << i + info1.low;
    }
    else
    {
      ss << setw(6) << fixed << setprecision(2) <<
        static_cast<float>(i + info1.low) / 
        static_cast<float>(info1.factor);
    }

    ss << setw(6) << counts1[i].count;

    for (unsigned j = index2Low; j <= index2High; j++)
    {
      if (tableIndex == 0)
        ss << setw(6) << counts[i][j].count;
      else if (counts[i][j].count == 0)
        ss << setw(6) << "-";
      else
        ss << setw(5) << fixed << setprecision(0) <<
          100.f * static_cast<float>(counts[i][j].hits) /
          static_cast<float>(counts[i][j].count) << "%";
    }
    ss << "\n";
  }
  ss << "\n";
  
  return ss.str();
}


string Stats2D::str() const
{
  unsigned index1Low, index1High;
  if (! Stats2D::findLimits(counts1, index1Low, index1High))
    return "";

  unsigned index2Low, index2High;
  if (! Stats2D::findLimits(counts2, index2Low, index2High))
    return "";

  return 
    Stats2D::strTable(0, index1Low, index1High, index2Low, index2High) + 
    Stats2D::strTable(1, index1Low, index1High, index2Low, index2High);
}

