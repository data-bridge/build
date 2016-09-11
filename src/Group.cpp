/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Group.h"
#include "portab.h"
#include "parse.h"
#include "debug.h"

extern Debug debug;


Group::Group()
{
  Group::Reset();
}


Group::~Group()
{
}


void Group::Reset()
{
  len = 0;
  segments.clear();
}


bool Group::MakeSegment(const unsigned no)
{
  if (Group::GetSegment(no) != nullptr)
    return false;

  segments.resize(len+1);
  segments[len].extNo = no;
  len++;
  return true;
}


Segment * Group::GetSegment(const unsigned extNo) 
{
  for (auto &p: segments)
  {
    if (p.extNo == extNo)
      return &p.segment;
  }
  return nullptr;
}


const Segment * Group::GetSegmentReadOnly(const unsigned extNo) const
{
  for (auto &p: segments)
  {
    if (p.extNo == extNo)
      return &p.segment;
  }
  return nullptr;
}


unsigned Group::GetLength() const
{
 return len;
}


bool Group::operator == (const Group& g2) const
{
  if (len != g2.len)
  {
    LOG("Different lengths");
    return false;
  }

  for (auto &p: segments)
  {
    const Segment * s2 = g2.GetSegmentReadOnly(p.extNo);
    if (s2 == nullptr || p.segment != * s2)
    {
      LOG("Segments not identical");
      return false;
    }
  }

  return true;
}


bool Group::operator != (const Group& s2) const
{
  return ! (* this == s2);
}

