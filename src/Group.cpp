/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

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


Segment * Group::make()
{
  segments.resize(segments.size()+1);
  return &segments[segments.size()-1];
}


unsigned Group::count() 
{
  unsigned cnt = 0;
  for (auto &segment: segments)
    cnt += segment.count();

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

  for (unsigned i = 0; i< segments.size(); i++)
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

