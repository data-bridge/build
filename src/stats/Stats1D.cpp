/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>

#include "Stats1D.h"


Stats1D::Stats1D()
{
  Stats1D::reset();
}


void Stats1D::reset()
{
  info.reset();
  counts.clear();
}


void Stats1D::set(
  const StatsInfo& infoIn)
{
  info = infoIn;
  counts.resize(info.length);
}


void Stats1D::add(
  const unsigned param,
  const bool flag)
{
  assert(param >= info.low);
  const unsigned index = param - info.low;
  assert(index < counts.size());
  counts[index].add(flag);
}


void Stats1D::operator += (const Stats1D& s2)
{
  assert(counts.size() == s2.counts.size());
  for (unsigned i = 0; i < counts.size(); i++)
    counts[i] += s2.counts[i];
}


bool Stats1D::findLimits(
  unsigned& indexLow,
  unsigned& indexHigh) const
{
  // Find the lowest, used index.
  indexLow = numeric_limits<unsigned>::max();
  for (unsigned i = 0; i < counts.size(); i++)
  {
    if (counts[i].count)
    {
      indexLow = i;
      break;
    }
  }

  if (indexLow == numeric_limits<unsigned>::max())
    return false;

  // Find the highest, used index.
  indexHigh = 0;
  for (unsigned i = 0; i < counts.size(); i++)
  {
    if (counts[i].count)
      indexHigh = i;
  }
  return true;
}


string Stats1D::validateProbs(const vector<float>& rowProbs) const
{
  const size_t upper = min(counts.size(), rowProbs.size());
  if (counts.size() > rowProbs.size())
  {
    // Some extras were allocated, but for sure not used.
    for (size_t i = upper; i < counts.size(); i++)
      assert(counts[i].count == 0);
  }

  unsigned sum = 0;
  for (size_t i = 0; i < upper; i++)
    sum += counts[i].count;
  const float sumf = static_cast<float>(sum);

  float hardErrorActual = 0.;
  float hardErrorTable = 0.;
  float sumSqdiff = 0.;

  for (size_t i = 0; i < upper; i++)
  {
    if (counts[i].count == 0)
      continue;

    const float countf = static_cast<float>(counts[i].count);

    const float prob = countf / sumf;
    const float propActual = static_cast<float>(counts[i].hits) / countf;
    const float hard1 = (propActual <= 0.5 ? propActual : 1.f - propActual);
    const float hard2 = (rowProbs[i] <= 0.5 ? rowProbs[i] :
      1.f - rowProbs[i]);

    hardErrorActual += prob * hard1;
    hardErrorTable += prob * hard2;

    const float diff = propActual - rowProbs[i];
    sumSqdiff += prob * diff * diff;

    cout << "  i " << i << ": prob " << prob << " propActual " <<
      propActual << " rowProbs[i] " << rowProbs[i]<< "\n";
  }

  stringstream ss;
  ss << fixed << setprecision(6) << 
    hardErrorActual << " " <<
    hardErrorTable << " " <<
    sqrt(sumSqdiff);
  return ss.str();
}


string Stats1D::strHeader() const
{
  stringstream ss;
  ss << 
    setw(6) << right << "Value" << 
    setw(6) << "Count" << 
    setw(8) << "%" << 
    setw(6) << "Hits" << 
    setw(8) << "%" << 
    "\n";

  return ss.str();
}


string Stats1D::str() const
{
  unsigned indexLow, indexHigh;
  if (! Stats1D::findLimits(indexLow, indexHigh))
    return "";

  stringstream ss;
  ss << info.name << "\n\n";
  ss << Stats1D::strHeader();

  Count1D countSum;
  countSum.reset();
  for (unsigned i = indexLow; i <= indexHigh; i++)
    countSum += counts[i];

  for (unsigned i = indexLow; i <= indexHigh; i++)
  {
    if (info.factor == 1)
    {
      ss << setw(6) << i + info.low;
    }
    else
    {
      // For CCCC-type statistics, only non-zero ones.
      if (counts[i].count == 0)
        continue;

      ss << setw(6) << fixed << setprecision(2) <<
        static_cast<float>(i + info.low) / 
        static_cast<float>(info.factor);
    }

    ss <<
      setw(6) << counts[i].count << 
      setw(7) << fixed << setprecision(2) <<
        100.f * static_cast<float>(counts[i].count) /
        static_cast<float>(countSum.count) << "%" << 
      setw(6) << counts[i].hits;
    
    if (counts[i].count)
      ss << setw(7)  << fixed << setprecision(2) <<
        100.f * static_cast<float>(counts[i].hits) /
        static_cast<float>(counts[i].count) << "%" << 
        "\n";
    else
      ss << setw(8) << "-" << "\n";
  }
  
  ss << string(34, '-') << "\n";

  ss <<
    setw(6) << "SUM" <<
    setw(6) << countSum.count <<
    setw(8) << "" <<
    setw(6) << countSum.hits <<
    setw(7) <<
        100.f * static_cast<float>(countSum.hits) /
        static_cast<float>(countSum.count) << "%" << 
        "\n";

  return ss.str();
}

