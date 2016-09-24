/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Group.h"
#include "Board.h"
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
  filename = "";
  segments.clear();
}


void Group::SetFileName(const string& fname)
{
  filename = fname;
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


string Group::GetFileName() const
{
  return filename;
}


unsigned Group::GetLength() const
{
 return len;
}


unsigned Group::GetCount() 
{
  unsigned count = 0;
  for (unsigned g = 0; g < len; g++)
    for (unsigned b = 0; b < segments[g].segment.GetLength(); b++)
      count += segments[g].segment.GetBoard(b)->GetLength();

  return count;
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

