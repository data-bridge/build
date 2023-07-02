/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>

#include "DistElem.h"

#include "../util/parse.h"
#include "../handling/Bexcept.h"


DistElem::DistElem()
{
  DistElem::reset();
}


void DistElem::reset()
{
  setFlag = false;
  name = "";
  min = 0;
  max = 13;
}


void DistElem::set(
  const string& nameIn,
  const string& spec)
{
  name = nameIn;

  // Look for "a..b" format.
  vector<string> v;
  tokenize(spec, v, "..");

  unsigned no;
  if (v.size() == 2)
  {
    if (! str2upos(v[0], no))
      THROW("Odd first suit-length spec: " + spec);
    min = no;

    if (! str2upos(v[1], no))
      THROW("Odd second suit-length spec: " + spec);
    max = no;
    setFlag = true;
    return;
  }
  else if (v.size() != 1)
    THROW("Odd suit-length spec: " + spec);

  if (! str2upos(v[0], no))
    THROW("Odd second suit-length spec: " + spec);
  min = no;
  max = no;
  setFlag = true;
}


bool DistElem::match(const int value) const
{
  if (! setFlag)
    return true;
  else
    return (static_cast<unsigned>(value) >= min && 
      static_cast<unsigned>(value) <= max);
}


string DistElem::str() const
{
  if (! setFlag)
    return "";

  stringstream ss;
  ss << setw(10) << name << ": ";
  if (min == max)
    ss << min << "\n";
  else
    ss << min << " to " << max << "\n";
  return ss.str();
}
