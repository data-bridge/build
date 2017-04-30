/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

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


ValStats::~ValStats()
{
}


void ValStats::reset()
{
  for (unsigned vOrig = 0; vOrig < BRIDGE_FORMAT_LABELS_SIZE; vOrig++)
  {
    for (unsigned vRef = 0; vRef < BRIDGE_FORMAT_LABELS_SIZE; vRef++)
    {
      ValStat& stat = stats[vOrig][vRef];
      for (unsigned s = 0; s < BRIDGE_VAL_SUMM_SIZE; s++)
        stat.count[s] = 0;
    }
  }
}


void ValStats::add(
  const Format formatOrig,
  const Format formatRef,
  const ValProfile& prof)
{
  assert(formatOrig != BRIDGE_FORMAT_LABELS_SIZE);
  assert(formatRef != BRIDGE_FORMAT_LABELS_SIZE);

  ValStat& current = stats[formatOrig][formatRef];

  bool minorFlag, pavlicekBugFlag, programErrorFlag;

  current.profile.addRange(prof, 0, BRIDGE_VAL_TXT_ALL_PASS, 
    minorFlag);

  current.profile.addRange(prof, BRIDGE_VAL_TXT_ALL_PASS, BRIDGE_VAL_ERROR, 
    pavlicekBugFlag);

  current.profile.addRange(prof, BRIDGE_VAL_ERROR, BRIDGE_VAL_SIZE, 
    programErrorFlag);

  current.count[BRIDGE_VAL_ALL]++;

  if (minorFlag)
    current.count[BRIDGE_VAL_MINOR]++;

  if (pavlicekBugFlag)
    current.count[BRIDGE_VAL_PAVLICEK]++;

  if (programErrorFlag)
    current.count[BRIDGE_VAL_MAJOR]++;
}


void ValStats::operator += (const ValStats& statsIn)
{
  for (unsigned vOrig = 0; vOrig < BRIDGE_FORMAT_LABELS_SIZE; vOrig++)
  {
    for (unsigned vRef = 0; vRef < BRIDGE_FORMAT_LABELS_SIZE; vRef++)
    {
      ValStat& current = stats[vOrig][vRef];
      const ValStat& currentIn = statsIn.stats[vOrig][vRef];

      current.profile += currentIn.profile;
      for (unsigned s = 0; s < BRIDGE_VAL_SUMM_SIZE; s++)
        current.count[s] += currentIn.count[s];
    }
  }
}


string ValStats::posOrDash(const unsigned u) const
{
  if (u == 0)
    return "-";
  else
    return STR(u);
}


bool ValStats::rowHasEntries(
  const ValStat stat[],
  const unsigned label) const
{
  for (auto &g: FORMAT_ACTIVE)
    if (stat[g].profile.labelIsSet(label))
      return true;

  return false;
}


bool ValStats::summRowHasEntries(
  const ValStat stat[],
  const unsigned summLabel) const
{
  assert(summLabel < BRIDGE_VAL_SUMM_SIZE);

  for (auto &g: FORMAT_ACTIVE)
    if (stat[g].count[summLabel])
      return true;

  return false;
}


void ValStats::printRow(
  ostream& fstr,
  const ValStat stat[],
  const unsigned label) const
{
  fstr << "  " << setw(5) << left << ValErrors[label].nameShort;
  for (auto &g: FORMAT_ACTIVE)
    fstr << setw(7) << right << posOrDash(stat[g].profile.getCount(label));
  fstr << "\n";
}


void ValStats::printRows(
  ostream& fstr,
  const ValStat stat[],
  const unsigned lower,
  const unsigned upper) const
{
  for (unsigned v = lower; v < upper; v++)
  {
    if (ValStats::rowHasEntries(stat, v))
      printRow(fstr, stat, v);
  }
}


void ValStats::printSummRow(
  ostream& fstr,
  const ValStat stat[],
  const string& header,
  const ValSumm summLabel) const
{
  if (! ValStats::summRowHasEntries(stat, summLabel))
    return;

  fstr << setw(7) << left << header;
  for (auto &g: FORMAT_ACTIVE)
    fstr << setw(7) << right << posOrDash(stat[g].count[summLabel]);
  fstr << "\n";
}


void ValStats::print(
  ostream& fstr,
  const bool detailFlag) const
{
  fstr << setw(7) << "";
  for (auto &f: FORMAT_ACTIVE)
    fstr << setw(7) << right << FORMAT_NAMES[f];
  fstr << "\n\n";

  for (auto &f: FORMAT_ACTIVE)
  {
    ValStats::printSummRow(fstr, stats[f], FORMAT_NAMES[f], BRIDGE_VAL_ALL);

    if (detailFlag)
      ValStats::printRows(fstr, stats[f], 0, BRIDGE_VAL_TXT_ALL_PASS);

    ValStats::printSummRow(fstr, stats[f], "MINOR", BRIDGE_VAL_MINOR);

    if (detailFlag)
      ValStats::printRows(fstr, stats[f], 
        BRIDGE_VAL_TXT_ALL_PASS, BRIDGE_VAL_ERROR);

    ValStats::printSummRow(fstr, stats[f], "RPBUG", BRIDGE_VAL_PAVLICEK);

    if (detailFlag)
      ValStats::printRows(fstr, stats[f], 
        BRIDGE_VAL_ERROR, BRIDGE_VAL_SIZE);

    ValStats::printSummRow(fstr, stats[f], "MAJOR", BRIDGE_VAL_MAJOR);

    if (ValStats::summRowHasEntries(stats[f], BRIDGE_VAL_ALL))
      fstr << "\n";
  }
}

