/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <regex>

#include "Date.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


Date::Date()
{
  Date::reset();
}


Date::~Date()
{
}


void Date::reset()
{
  year = 0;
  month = 0;
  day = 0;
}


void Date::check() const
{
  if (year > 2100)
    THROW("Year is mighty large");

  if (month > 12)
    THROW("Month is too large");

  if (day > 31)
    THROW("Month is too large");

  if (month == 0 && day > 0)
    THROW("Don't want day in unknown month");

  if (month == 0 || day == 0)
    return;

  if (month == 2 && day >= 30)
    THROW("February day is too large"); // Yeah yeah, leap years

  if ((month == 4 || 
      month == 6 || 
      month == 9 ||
      month == 11) &&
      day >= 31)
    THROW("Short month is too long");
}


void Date::setLIN(const string& t)
{
  // Note that this parses the date in the *file name*, not a real
  // date field.

  regex re("(\\d\\d)-(\\d\\d)-(\\d\\d)");
  smatch match;
  if (! regex_search(t, match, re) || match.size() < 3)
    THROW("Bad LIN date");

  (void) str2upos(match.str(1), month);
  (void) str2upos(match.str(2), day);
  (void) str2upos(match.str(3), year);

  if (year > 20)
    year = 1900 + year;
  else
    year = 2000 + year;

  Date::check();
}


void Date::setPBN(const string& t)
{
  regex re("(....).(..).(..)");
  smatch match;
  if (! regex_search(t, match, re) || match.size() < 3)
    THROW("Bad PBN date");

  if (match.str(1) == "????")
    year = 0;
  else if (! str2upos(match.str(1), year))
    THROW("Bad PBN year");

  if (match.str(2) == "??")
    month = 0;
  else if (! str2upos(match.str(2), month))
    THROW("Bad PBN month number");

  if (match.str(3) == "??")
    day = 0;
  else if (! str2upos(match.str(3), day))
    THROW("Bad PBN day");

  Date::check();
}


void Date::setRBN(const string& t)
{
  if (t.length() == 6)
  {
    regex re("(\\d\\d\\d\\d)(\\d\\d)");
    smatch match;
    if (! regex_search(t, match, re) || match.size() < 2)
      THROW("Bad RBN date");

    (void) str2upos(match.str(1), year);
    (void) str2upos(match.str(2), month);
  }
  else
  {
    regex re("(\\d\\d\\d\\d)(\\d\\d)(\\d\\d)");
    smatch match;
    if (! regex_search(t, match, re) || match.size() < 3)
      THROW("Bad RBN date");

    (void) str2upos(match.str(1), year);
    (void) str2upos(match.str(2), month);
    (void) str2upos(match.str(3), day);
  }

  Date::check();
}


void Date::setTXT(const string& t)
{
  regex re("(\\w+) (\\d\\d\\d\\d)");
  smatch match;
  if (! regex_search(t, match, re) || match.size() < 2)
    THROW("Bad TXT date");

  (void) str2upos(match.str(2), year);
  month = str2month(match.str(1));

  Date::check();
}


void Date::set(
  const string& t,
  const Format f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      Date::setLIN(t);
      return;
    
    case BRIDGE_FORMAT_PBN:
      Date::setPBN(t);
      return;
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      Date::setRBN(t);
      return;
    
    case BRIDGE_FORMAT_TXT:
      Date::setTXT(t);
      return;
    
    default:
      THROW("Invalid format: " + STR(f));
  }
}


bool Date::operator == (const Date& date2) const
{
  if (year != date2.year)
    DIFF("Different years");

  if (month != date2.month)
    DIFF("Different months");

  if (day != date2.day)
    DIFF("Different days");

  return true;
}


bool Date::operator != (const Date& date2) const
{
  return ! (* this == date2);
}


string Date::asLIN() const
{
  if (day == 0 || month == 0 || year == 0)
    return "";

  stringstream s;
  s <<  
    setfill('0') << setw(2) << month << "-" <<
    setfill('0') << setw(2) << day << "-" <<
    setfill('0') << setw(2) << (year % 100);
  return s.str();
}


string Date::asPBN() const
{
  if (day == 0 && month == 0 && year == 0)
    return "";

  stringstream s;
  s << "[Date \"";
  if (year == 0)
    s << "????.";
  else
    s << setfill('0') << setw(4) << year << ".";

  if (month == 0)
    s << "??.";
  else
    s << setfill('0') << setw(2) << month << ".";

  if (day == 0)
    s << "??\"]\n";
  else
    s << setfill('0') << setw(2) << day << "\"]\n";

  return s.str();
}


string Date::asRBNCore() const
{
  if (month == 0 || year == 0)
    return "";

  stringstream s;
  s << year << setfill('0') << setw(2) << month;
  if (day > 0)
    s << day;
  return s.str();
}

string Date::asRBN() const
{
  return "D " + Date::asRBNCore() + "\n";
}


string Date::asRBX() const
{
  return "D{" + Date::asRBNCore() + "}";
}


string Date::asTXT() const
{
  if (month == 0 || year == 0)
    return "";

  stringstream s;
  if (day > 0)
    s << day << " ";
  s << DATE_MONTHS[month] << " " << year << "\n";
  return s.str();
}


string Date::asString(const Format f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Date::asLIN();
    
    case BRIDGE_FORMAT_PBN:
      return Date::asPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Date::asRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Date::asRBX();
    
    case BRIDGE_FORMAT_TXT:
      return Date::asTXT();
    
    default:
      THROW("Invalid format: " + STR(f));
  }
}

