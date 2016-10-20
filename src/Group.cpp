/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <assert.h>

#include "Group.h"
#include "Board.h"
#include "portab.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


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
  fileName = "";
  formatOrigin = BRIDGE_FORMAT_SIZE;
  segments.clear();
}


void Group::SetFileName(const string& fname)
{
  fileName = fname;
}


void Group::SetInputFormat(const formatType f)
{
  formatOrigin = f;
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


formatType Group::GetInputFormat() const
{
  assert(formatOrigin != BRIDGE_FORMAT_SIZE);
  return formatOrigin;
}


string Group::GetFileName() const
{
  return fileName;
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
    DIFF("Different lengths");

  for (auto &p: segments)
  {
    const Segment * s2 = g2.GetSegmentReadOnly(p.extNo);
    if (s2 == nullptr || p.segment != * s2)
      DIFF("Segments not identical");
  }

  return true;
}


bool Group::operator != (const Group& s2) const
{
  return ! (* this == s2);
}

