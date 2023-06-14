/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <assert.h>

#include "CompStats.h"

#include "../parse.h"


CompStats::CompStats()
{
  CompStats::reset();
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


string CompStats::str() const
{
  stringstream ss;

  ss << setw(8) << left << "format" << 
    setw(8) << right << "count" << 
    setw(8) << "errors" << endl;

  for (auto &f: FORMAT_ACTIVE)
  {
    if (stats[f].count)
    {
      ss << setw(8) << left << FORMAT_NAMES[f] <<
        setw(8) << right << stats[f].count <<
        setw(8) << posOrDash(stats[f].errors) << endl;
    }
  }
  return ss.str() + "\n\n";
}

