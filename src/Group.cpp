/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <assert.h>

#include "Group.h"
// #include "Board.h"
// #include "portab.h"
// #include "parse.h"
#include "Bexcept.h"
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
  len = 0;
  nameVal = "";
  formatVal = BRIDGE_FORMAT_SIZE;
  segmentPairs.clear();
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


formatType Group::format() const
{
  return formatVal;
}


Segment * Group::make(const unsigned extNo)
{
  if (Group::get(extNo) != nullptr)
    THROW("Segment already exists: " + STR(extNo));

  len++;
  segmentPairs.resize(len);
  segmentPairs[len-1].extNo = extNo;
  return &segmentPairs[len-1].segment;
}


Segment * Group::get(const unsigned extNo)
{
  for (auto &p: segmentPairs)
  {
    if (p.extNo == extNo)
      return &p.segment;
  }
  return nullptr;
}


unsigned Group::size() const
{
 return segmentPairs.size();
}


unsigned Group::count() 
{
  unsigned count = 0;
  for (auto &pair: segmentPairs)
    count += pair.segment.count();

  return count;
}


bool Group::operator == (const Group& group2) const
{
  if (len != group2.len)
    DIFF("Different lengths");

  for (auto &pair: segmentPairs)
  {
    bool seen = false;
    for (auto &pair2: group2.segmentPairs)
    {
      if (pair.extNo == pair2.extNo)
      {
        seen = true;
        if (pair.segment == pair2.segment)
          break;
        else
          return false;
      }
    }
    if (! seen)
      return false;

    // const Segment * segment2 = group2.GetSegmentReadOnly(pair.extNo);
    // if (segment2 == nullptr || pair.segment != * segment2)
      // DIFF("Segments not identical");
  }

  return true;
}


bool Group::operator != (const Group& group2) const
{
  return ! (* this == group2);
}

