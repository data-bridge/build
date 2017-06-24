/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <string>
#include <iomanip>
#include <sstream>

#include "DuplStat.h"
#include "Group.h"
#include "RefLines.h"
#include "Bexcept.h"


DuplStat::DuplStat()
{
  DuplStat::reset();
}


DuplStat::~DuplStat()
{
}


void DuplStat::reset()
{
  groupPtr = nullptr;
  segmentPtr = nullptr;
  values.clear();
}


void DuplStat::set(
  const Group * group,
  const Segment * segment,
  const unsigned segNo,
  const RefLines * reflines)
{
  DuplStat::reset();
  groupPtr = group;
  segmentPtr = segment;
  segNoVal = segNo;
  reflinesPtr = reflines;
}


void DuplStat::append(const int hashVal)
{
  values.push_back(static_cast<unsigned>(hashVal));
}


void DuplStat::sort()
{
  values.sort();
}


unsigned DuplStat::first() const
{
  if (values.size() == 0)
    THROW("List is empty");
  return values.front();
}


bool DuplStat::sameOrigin(const DuplStat& ds2) const
{
  if (groupPtr == nullptr)
    THROW("No group");
  return (groupPtr->name() == ds2.groupPtr->name());
}


bool DuplStat::lexLessThan(const DuplStat& ds2) const
{
  // First by list, then by length, then by basename, then by format.

  for (auto it1 = values.cbegin(), it2 = ds2.values.cbegin();
      it1 != values.cend() && it2 != ds2.values.cend(); it1++, it2++)
  {
    if (*it1 < *it2)
      return true;
    if (*it1 > *it2)
      return false;
  }

  const unsigned l1 = values.size();
  const unsigned l2 = ds2.values.size();
  if (l1 < l2)
    return true;
  if (l1 > l2)
    return false;

  const string n1 = groupPtr->name();
  const string n2 = ds2.groupPtr->name();
  if (n1 < n2)
    return true;
  if (n1 > n2)
    return false;

  return (groupPtr->format() < ds2.groupPtr->format());
}


bool DuplStat::operator ==(const DuplStat& ds2) const
{
  try
  {
    * groupPtr == * ds2.groupPtr;
    return true;
  }
  catch (Bexcept& bex)
  {
    UNUSED(bex);
    return false;
  }
}


bool DuplStat::operator <=(const DuplStat& ds2) const
{
  try
  {
    * groupPtr <= * ds2.groupPtr;
    return false;
  }
  catch (Bexcept& bex)
  {
    UNUSED(bex);
    return false;
  }
}


string DuplStat::str() const
{
  if (segmentPtr == nullptr)
    THROW("No segment");
  string st = segmentPtr->strTitle(BRIDGE_FORMAT_TXT);
  if (segmentPtr->size() != 1)
    st += "WARNING: Multiple segments";
  return st;
}


string DuplStat::strSuggest(const string& tag) const
{
  if (reflinesPtr == nullptr)
    THROW("No reflines");
  return tag + "{" + tag + "(" + reflinesPtr->strHeader() + ")}\n";
}

