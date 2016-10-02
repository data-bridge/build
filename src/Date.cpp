/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <regex>
#include <iomanip>
#include "Date.h"
#include "portab.h"
#include "parse.h"
#include "Debug.h"

extern Debug debug;


const string DATE_MONTHS[] =
{
  "None",
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December"
};


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


void Date::StringToMonth(const string& m)
{
  string s = m;
  s.at(0) = static_cast<char>(tolower(m.at(0)));
  // Can't get this to work with the Microsoft compiler.
  // transform(m.begin(), m.end(), s.begin(), (int (*)(int))tolower);

  if (s == "january")
    date.month = 1;
  else if (s == "february")
    date.month = 2;
  else if (s == "march")
   date.month = 3;
  else if (s == "april")
   date.month = 4;
  else if (s == "may")
   date.month = 5;
  else if (s == "june")
   date.month = 6;
  else if (s == "july")
   date.month = 7;
  else if (s == "august")
   date.month = 8;
  else if (s == "september")
   date.month = 9;
  else if (s == "october")
   date.month = 10;
  else if (s == "november")
   date.month = 11;
  else if (s == "december")
   date.month = 12;
  else
    date.month = 0;
}


bool Date::CheckDate() const
{
  if (date.year > 2100)
  {
    LOG("Year is mighty large");
    return false;
  }

  if (date.month > 12)
  {
    LOG("Month is too large");
    return false;
  }

  if (date.day > 31)
  {
    LOG("Month is too large");
    return false;
  }

  if (date.month == 0 && date.day > 0)
  {
    LOG("Don't want day in unknown month");
    return false;
  }

  if (date.month == 0 || date.day == 0)
    return true;

  if (date.month == 2)
    return (date.day < 30); // Yeah yeah, leap years
  else if (date.month == 4 || 
      date.month == 6 || 
      date.month == 9 ||
      date.month == 11)
    return (date.day < 31);
  else
    return true;
}


bool Date::SetLIN(const string& t)
{
  // Note that this parses the date in the *file name*, not a real
  // date field.

  try
  {
    // mm-dd-yy
    regex re("(\\d\\d)-(\\d\\d)-(\\d\\d)");
    smatch match;
    if (regex_search(t, match, re) && match.size() >= 3)
    {
      (void) StringToNonzeroUnsigned(match.str(1), date.month);

      (void) StringToNonzeroUnsigned(match.str(2), date.day);

      (void) StringToNonzeroUnsigned(match.str(3), date.year);
      if (date.year > 20)
        date.year = 1900 + date.year;
      else
        date.year = 2000 + date.year;

    }
    else
    {
      return false;
    }
  }
  catch (regex_error& e)
  {
    UNUSED(e);
    LOG("Bad date");
    return false;
  }

  return Date::CheckDate();
}


bool Date::SetPBN(const string& t)
{
  try
  {
    // regex re("(\\d\\d\\d\\d).(\\d\\d)");
    regex re("(....).(..).(..)");
    smatch match;
    if (regex_search(t, match, re) && match.size() >= 3)
    {
      if (match.str(1) == "????")
        date.year = 0;
      else if (! StringToNonzeroUnsigned(match.str(1), date.year))
        return false;

      if (match.str(2) == "??")
        date.month = 0;
      else if (! StringToNonzeroUnsigned(match.str(2), date.month))
        return false;

      if (match.str(3) == "??")
        date.day = 0;
      else if (! StringToNonzeroUnsigned(match.str(3), date.day))
        return false;
    }
    else
    {
      return false;
    }
  }
  catch (regex_error& e)
  {
    UNUSED(e);
    return false;
  }
  return Date::CheckDate();
}


bool Date::SetRBN(const string& t)
{
  if (t.length() == 6)
  {
    try
    {
      regex re("(\\d\\d\\d\\d)(\\d\\d)");
      smatch match;
      if (regex_search(t, match, re) && match.size() >= 2)
      {
        (void) StringToNonzeroUnsigned(match.str(1), date.year);
        (void) StringToNonzeroUnsigned(match.str(2), date.month);
      }
      else
      {
        return false;
      }
    }
    catch (regex_error& e)
    {
      UNUSED(e);
      return false;
    }
  }
  else
  {
    try
    {
      regex re("(\\d\\d\\d\\d)(\\d\\d)(\\d\\d)");
      smatch match;
      if (regex_search(t, match, re) && match.size() >= 3)
      {
        (void) StringToNonzeroUnsigned(match.str(1), date.year);
        (void) StringToNonzeroUnsigned(match.str(2), date.month);
        (void) StringToNonzeroUnsigned(match.str(3), date.day);
      }
      else
      {
        return false;
      }
    }
    catch (regex_error& e)
    {
      UNUSED(e);
      return false;
    }
  }
  return Date::CheckDate();
}


bool Date::SetTXT(const string& t)
{
  try
  {
    regex re("(\\w+) (\\d\\d\\d\\d)");
    smatch match;
    if (regex_search(t, match, re) && match.size() >= 2)
    {
      (void) StringToNonzeroUnsigned(match.str(2), date.year);
      string m = match.str(1);
      Date::StringToMonth(m); // sets date.month
    }
    else
    {
      return false;
    }
  }
  catch (regex_error& e)
  {
    UNUSED(e);
    return false;
  }
  return Date::CheckDate();
}


bool Date::Set(
  const string& t,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Date::SetLIN(t);
    
    case BRIDGE_FORMAT_PBN:
      return Date::SetPBN(t);
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return Date::SetRBN(t);
    
    case BRIDGE_FORMAT_TXT:
      return Date::SetTXT(t);
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
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


string Date::AsLIN() const
{
  if (date.day == 0 || date.month == 0 || date.year == 0)
    return "";

  stringstream s;
  s <<  
    setfill('0') << setw(2) << date.month << "-" <<
    setfill('0') << setw(2) << date.day << "-" <<
    setfill('0') << setw(2) << (date.year % 100);
  return s.str();
}


string Date::AsPBN() const
{
  if (date.day == 0 && date.month == 0 && date.year == 0)
    return "";

  stringstream s;
  s << "[Date \"";
  if (date.year == 0)
    s << "????.";
  else
    s << setfill('0') << setw(4) << date.year << ".";
  if (date.month == 0)
    s << "??.";
  else
    s << setfill('0') << setw(2) << date.month << ".";
  if (date.day == 0)
    s << "??\"]\n";
  else
    s << setfill('0') << setw(2) << date.day << "\"]\n";
  return s.str();
}


string Date::AsRBNCore() const
{
  if (date.month == 0 || date.year == 0)
    return "";

  stringstream s;
  s << date.year << setfill('0') << setw(2) << date.month;
  if (date.day > 0)
    s << date.day;
  return s.str();
}

string Date::AsRBN() const
{
  return "D " + Date::AsRBNCore() + "\n";
}


string Date::AsRBX() const
{
  return "D{" + Date::AsRBNCore() + "}";
}


string Date::AsTXT() const
{
  if (date.month == 0 || date.year == 0)
    return "";

  stringstream s;
  if (date.day > 0)
    s << date.day << " ";
  s << DATE_MONTHS[date.month] << " " << date.year << "\n";
  return s.str();
}


string Date::AsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Date::AsLIN();
    
    case BRIDGE_FORMAT_PBN:
      return Date::AsPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Date::AsRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Date::AsRBX();
    
    case BRIDGE_FORMAT_TXT:
      return Date::AsTXT();
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}

