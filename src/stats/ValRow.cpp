/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <assert.h>

#include "ValRow.h"


ValRow::ValRow()
{
  ValRow::reset();
}


void ValRow::reset()
{
  for (unsigned vRef = 0; vRef < BRIDGE_FORMAT_LABELS_SIZE; vRef++)
    for (unsigned s = 0; s < BRIDGE_VAL_SUMM_SIZE; s++)
      row[vRef].count[s] = 0;
}


void ValRow::add(
  const Format formatRef,
  const ValProfile& prof)
{
  assert(formatRef < BRIDGE_FORMAT_LABELS_SIZE);
  row[formatRef].add(prof);
}


void ValRow::operator += (const ValRow& vr2)
{
  for (unsigned vRef = 0; vRef < BRIDGE_FORMAT_LABELS_SIZE; vRef++)
  {
    ValStat& current = row[vRef];
    const ValStat& current2 = vr2.row[vRef];

    current.profile += current2.profile;
    for (unsigned s = 0; s < BRIDGE_VAL_SUMM_SIZE; s++)
      current.count[s] += current2.count[s];
  }
}


string ValRow::posOrDash(const size_t u) const
{
  if (u == 0)
    return "-";
  else
    return to_string(u);
}


bool ValRow::profileHasLabel(const unsigned label) const
{
  for (auto &f: FORMAT_ACTIVE)
    if (row[f].profile.labelIsSet(label))
      return true;

  return false;
}


bool ValRow::countHasLabel(const ValSumm label) const
{
  assert(label < BRIDGE_VAL_SUMM_SIZE);

  for (auto &f: FORMAT_ACTIVE)
    if (row[f].count[label])
      return true;

  return false;
}


string ValRow::strProfile(const unsigned label) const
{
  stringstream ss;
  ss << "  " << setw(5) << left << ValErrors[label].nameShort;
  for (auto &f: FORMAT_ACTIVE)
    ss << setw(7) << right << posOrDash(row[f].profile.getCount(label));
  return ss.str() + "\n";
}


string ValRow::strCount(
  const string& header,
  const ValSumm label) const
{
  if (! ValRow::countHasLabel(label))
    return "";

  stringstream ss;
  ss << setw(7) << left << header;
  for (auto &f: FORMAT_ACTIVE)
    ss << setw(7) << right << ValRow::posOrDash(row[f].count[label]);
  return ss.str() + "\n";
}

