/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <sstream>

#include "Group.h"
#include "Bdiff.h"


Group::Group()
{
  Group::reset();
}


Group::~Group()
{
}


void Group::reset()
{
  nameVal = "";
  formatVal = BRIDGE_FORMAT_SIZE;
  segments.clear();
  flagCOCO = false;
}


void Group::setName(const string& nameIn)
{
  nameVal = nameIn;
}


string Group::name() const
{
  return nameVal;
}


void Group::setFormat(const Format formatIn)
{
  formatVal = formatIn;
}


Format Group::format() const
{
  return formatVal;
}


void Group::setCOCO(const Format format)
{
  flagCOCO = true;
  for (auto &segment: segments)
    segment.setCOCO(format);
}


bool Group::isCOCO() const
{
  return flagCOCO;
}


Segment * Group::make()
{
  segments.emplace_back(Segment());
  if (flagCOCO)
    segments.back().setCOCO();
  return &segments.back();
}


unsigned Group::count() const
{
  unsigned cnt = 0;
  for (auto &segment: segments)
    cnt += segment.count();

  return cnt;
}


unsigned Group::countBoards() const
{
  unsigned cnt = 0;
  for (auto &segment: segments)
    cnt += segment.countBoards();

  return cnt;
}


bool Group::operator == (const Group& group2) const
{
  const unsigned s1 = segments.size();
  const unsigned s2 = group2.segments.size();
  if (s1 != s2)
    DIFF("Different lengths: " + STR(s1) + " vs. " + STR(s2));

  for (auto it1 = segments.cbegin(),
       it2 = group2.segments.cbegin();
       it1 != segments.cend() && it2 != group2.segments.cend();
       it1++, it2++)
  {
    if (*it1 != *it2)
      return false;
  }

  return true;
}


bool Group::operator != (const Group& group2) const
{
  return ! (* this == group2);
}

