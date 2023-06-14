/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <assert.h>

#include "ValStats.h"


ValStats::ValStats()
{
  ValStats::reset();
}


void ValStats::reset()
{
  for (unsigned vOrig = 0; vOrig < BRIDGE_FORMAT_LABELS_SIZE; vOrig++)
    stats[vOrig].reset();
}


void ValStats::add(
  const Format formatOrig,
  const Format formatRef,
  const ValProfile& prof)
{
  assert(formatOrig != BRIDGE_FORMAT_LABELS_SIZE);

  stats[formatOrig].add(formatRef, prof);
}


void ValStats::operator += (const ValStats& vs2)
{
  for (unsigned fOrig = 0; fOrig < BRIDGE_FORMAT_LABELS_SIZE; fOrig++)
    stats[fOrig] += vs2.stats[fOrig];
}


string ValStats::strDetails(
  const unsigned lower,
  const unsigned upper,
  const Format fOrig) const
{
  string s = "";
  for (unsigned v = lower; v < upper; v++)
  {
    if (stats[fOrig].profileHasLabel(v))
      s += stats[fOrig].strProfile(v);
  }
  return s;
}


string ValStats::str(const bool detailFlag) const
{
  stringstream ss;

  ss << setw(7) << "";
  for (auto &f: FORMAT_ACTIVE)
    ss << setw(7) << right << FORMAT_NAMES[f];
  ss << "\n\n";

  for (auto &fOrig: FORMAT_ACTIVE)
  {
    ss << stats[fOrig].strCount(FORMAT_NAMES[fOrig], BRIDGE_VAL_ALL);

    if (detailFlag)
      ss << ValStats::strDetails(0, BRIDGE_VAL_TXT_ALL_PASS, fOrig);

    ss << stats[fOrig].strCount("MINOR", BRIDGE_VAL_MINOR);

    if (detailFlag)
      ss << ValStats::strDetails(BRIDGE_VAL_TXT_ALL_PASS, 
        BRIDGE_VAL_ERROR, fOrig);

    ss << stats[fOrig].strCount("RPBUG", BRIDGE_VAL_PAVLICEK);

    if (detailFlag)
      ss << ValStats::strDetails(BRIDGE_VAL_ERROR, 
        BRIDGE_VAL_SIZE, fOrig);

    ss << stats[fOrig].strCount("MAJOR", BRIDGE_VAL_MAJOR);

    if (stats[fOrig].countHasLabel(BRIDGE_VAL_ALL))
      ss << "\n";
  }
  return ss.str();
}

