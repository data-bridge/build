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


void Group::setCOCO()
{
  flagCOCO = true;
}


bool Group::isCOCO()
{
  return flagCOCO;
}


Segment * Group::make()
{
  segments.resize(segments.size()+1);
  if (flagCOCO)
    segments[segments.size()-1].setCOCO();
  return &segments[segments.size()-1];
}


unsigned Group::count() 
{
  unsigned cnt = 0;
  for (auto &segment: segments)
    cnt += segment.count();

  return cnt;
}


unsigned Group::countBoards() 
{
  unsigned cnt = 0;
  for (auto &segment: segments)
    cnt += segment.countBoards();

  return cnt;
}


bool Group::operator == (const Group& group2) const
{
  if (segments.size() != group2.segments.size())
  {
    const unsigned s1 = segments.size();
    const unsigned s2 = group2.segments.size();
    DIFF("Different lengths: " + STR(s1) + " vs. " + STR(s2));
  }

  for (unsigned i = 0; i < segments.size(); i++)
  {
    if (segments[i] != group2.segments[i])
      return false;
  }

  return true;
}


bool Group::operator != (const Group& group2) const
{
  return ! (* this == group2);
}

