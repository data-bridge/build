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
#include <assert.h>
#pragma warning(pop)

#include "CompStats.h"
#include "parse.h"


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
        setw(8) << posOrDash(stats[f].errors) << endl;
    }
  }
  fstr << "\n\n";
}

