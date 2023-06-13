/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#include <sstream>
#include <iomanip>
#include <filesystem>
#include <assert.h>

#include "TextStat.h"

#define BRIDGE_STATS_MAX_LENGTH 64


void TextStat::reset()
{
  datum.resize(BRIDGE_STATS_MAX_LENGTH);

  for (auto& d: datum)
    d.reset();
  
  count = 0;
}


void TextStat::add(
  const string& source,
  const string& example)
{
  const size_t len = example.length();
  assert(len < BRIDGE_STATS_MAX_LENGTH);

  filesystem::path p(source);
  datum[len].add(p.filename().string(), example);

  count++;
}


void TextStat::add(
  const string& source,
  const size_t len)
{
  assert(len < BRIDGE_STATS_MAX_LENGTH);

  filesystem::path p(source);
  datum[len].add(p.filename().string(), "");

  count++;
}


void TextStat::operator += (const TextStat& ts2)
{
  for (size_t i = 0; i < BRIDGE_STATS_MAX_LENGTH; i++)
    datum[i] += ts2.datum[i];

  count += ts2.count;
}


bool TextStat::empty() const
{
  return (count == 0);
}


size_t TextStat::last_used() const
{
  size_t l = 0;
  for (size_t i = 0; i < BRIDGE_STATS_MAX_LENGTH; i++)
    if (! datum[i].empty())
      l = i;

  return l;
}


string TextStat::str() const
{
  stringstream ss;
  for (size_t i = BRIDGE_STATS_MAX_LENGTH; i-- > 0; 0)
  {
    const auto& d = datum[i];
    if (! d.empty())
      ss << setw(10) << left << i << datum[i].str() << endl;
  }

  return ss.str() + "\n";
}

