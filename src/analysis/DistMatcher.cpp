/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "DistMatcher.h"

#include "../util/parse.h"
#include "../handling/Bexcept.h"


DistMatcher::DistMatcher()
{
  DistMatcher::reset();
}


void DistMatcher::reset()
{
  spec = "";
  setFlag = false;
  spades.reset();
  hearts.reset();
  diamonds.reset();
  clubs.reset();
  long1.reset();
  long2.reset();
  long3.reset();
  long4.reset();
}


void DistMatcher::set(const string& specIn)
{
  if (specIn == "all" || specIn == "")
  {
    setFlag = false;
    return;
  }
  spec = specIn;

  vector<string> v;
  tokenize(spec, v, "=");

  if (v.size() == 4)
  {
    spades.set("Spades", v[0]);
    hearts.set("Hearts", v[1]);
    diamonds.set("Diamonds", v[2]);
    clubs.set("Clubs", v[3]);
    setFlag = true;
    return;
  }
  else if (v.size() != 1)
    THROW("Odd distribution spec: " + spec);

  v.clear();
  tokenize(spec, v, "-");

  if (v.size() == 4)
  {
    long1.set("Long1", v[0]);
    long2.set("Long2", v[1]);
    long3.set("Long3", v[2]);
    long4.set("Long4", v[3]);
    setFlag = true;
  }
  else
    THROW("Odd distribution spec: " + spec);
}


bool DistMatcher::match(
  const int spadesL,
  const int heartsL,
  const int diamondsL,
  const int clubsL,
  const int long1L,
  const int long2L,
  const int long3L,
  const int long4L) const
{
  if (! setFlag)
    return true;
  else
    return 
      spades.match(spadesL) &&
      hearts.match(heartsL) &&
      diamonds.match(diamondsL) &&
      clubs.match(clubsL) &&
      long1.match(long1L) &&
      long2.match(long2L) &&
      long3.match(long3L) &&
      long4.match(long4L);
}


string DistMatcher::str() const
{
  if (! setFlag)
    return "Distribution matcher not set\n";

  stringstream ss;
  ss << spades.str();
  ss << hearts.str();
  ss << diamonds.str();
  ss << clubs.str();
  ss << long1.str();
  ss << long2.str();
  ss << long3.str();
  ss << long4.str();
  return ss.str();
}


string DistMatcher::strSpec() const
{
  return spec;
}
