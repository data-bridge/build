/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Date.h"
#include "portab.h"
#include "debug.h"

extern Debug debug;


Date::Date()
{
  Date::Reset();
}


Date::~Date()
{
}


void Date::Reset()
{
  date.year = 0;
  date.month = 0;
  date.day = 0;
}


bool Date::Set(
  const string& t,
  const formatType f)
{
  UNUSED(t);
  UNUSED(f);
  // TODO
  return true;
}


bool Date::operator == (const Date& d2) const
{
  if (date.year != d2.date.year)
  {
    LOG("Different year");
    return false;
  }
  else if (date.month != d2.date.month)
  {
    LOG("Different month");
    return false;
  }
  else if (date.day != d2.date.day)
  {
    LOG("Different day");
    return false;
  }
  else
    return true;
}


bool Date::operator != (const Date& d2) const
{
  return ! (* this == d2);
}


string Date::AsString(const formatType f) const
{
  UNUSED(f);
  // TODO
  return "";
}

