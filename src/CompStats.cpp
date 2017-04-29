/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <string>
#include <iomanip>
#include <sstream>
#include <assert.h>

#include "CompStats.h"


CompStats::CompStats()
{
  CompStats::reset();
}


CompStats::~CompStats()
{
}


void CompStats::reset()
{
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
  {
    stats[f].count = 0;
    stats[f].errors = 0;
  }
}


void CompStats::add(
  const bool agreeFlag,
  const Format format)
{
  assert(format != BRIDGE_FORMAT_SIZE);
  stats[format].count++;
  if (! agreeFlag)
    stats[format].errors++;
}


void CompStats::operator += (const CompStats& statsIn)
{
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
  {
    stats[f].count += statsIn.stats[f].count;
    stats[f].errors += statsIn.stats[f].errors;
  }
}


string CompStats::posOrDash(const unsigned u) const
{
  if (u == 0)
    return "-";
  else
    return STR(u);
}


void CompStats::print(ostream& fstr) const
{
  fstr << setw(8) << left << "format" << 
    setw(8) << right << "count" << 
    setw(8) << "errors" << endl;

  for (auto &f: FORMAT_ACTIVE)
  {
    if (stats[f].count)
    {
      fstr << setw(8) << left << FORMAT_NAMES[f] <<
        setw(8) << right << stats[f].count <<
        setw(8) << CompStats::posOrDash(stats[f].errors) << endl;
    }
  }
  fstr << "\n\n";
}

