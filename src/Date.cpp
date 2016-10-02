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
  date.year = 0;
  date.month = 0;
  date.day = 0;
}


void Date::checkDate() const
{
  if (date.year > 2100)
    THROW("Year is mighty large");

  if (date.month > 12)
    THROW("Month is too large");

  if (date.day > 31)
    THROW("Month is too large");

  if (date.month == 0 && date.day > 0)
    THROW("Don't want day in unknown month");

  if (date.month == 0 || date.day == 0)
    return;

  if (date.month == 2 && date.day >= 30)
    THROW("February day is too large"); // Yeah yeah, leap years

  if ((date.month == 4 || 
      date.month == 6 || 
      date.month == 9 ||
      date.month == 11) &&
      date.day >= 31)
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

  (void) StringToNonzeroUnsigned(match.str(1), date.month);
  (void) StringToNonzeroUnsigned(match.str(2), date.day);
  (void) StringToNonzeroUnsigned(match.str(3), date.year);

  if (date.year > 20)
    date.year = 1900 + date.year;
  else
    date.year = 2000 + date.year;

  Date::checkDate();
}


void Date::setPBN(const string& t)
{
  regex re("(....).(..).(..)");
  smatch match;
  if (! regex_search(t, match, re) || match.size() < 3)
    THROW("Bad PBN date");

  if (match.str(1) == "????")
    date.year = 0;
  else if (! StringToNonzeroUnsigned(match.str(1), date.year))
    THROW("Bad PBN year");

  if (match.str(2) == "??")
    date.month = 0;
  else if (! StringToNonzeroUnsigned(match.str(2), date.month))
    THROW("Bad PBN month number");

  if (match.str(3) == "??")
    date.day = 0;
  else if (! StringToNonzeroUnsigned(match.str(3), date.day))
    THROW("Bad PBN day");

  Date::checkDate();
}


void Date::setRBN(const string& t)
{
  if (t.length() == 6)
  {
    regex re("(\\d\\d\\d\\d)(\\d\\d)");
    smatch match;
    if (! regex_search(t, match, re) || match.size() < 2)
      THROW("Bad RBN date");

    (void) StringToNonzeroUnsigned(match.str(1), date.year);
    (void) StringToNonzeroUnsigned(match.str(2), date.month);
  }
  else
  {
    regex re("(\\d\\d\\d\\d)(\\d\\d)(\\d\\d)");
    smatch match;
    if (! regex_search(t, match, re) || match.size() < 3)
      THROW("Bad RBN date");

    (void) StringToNonzeroUnsigned(match.str(1), date.year);
    (void) StringToNonzeroUnsigned(match.str(2), date.month);
    (void) StringToNonzeroUnsigned(match.str(3), date.day);
  }

  Date::checkDate();
}


void Date::setTXT(const string& t)
{
  regex re("(\\w+) (\\d\\d\\d\\d)");
  smatch match;
  if (! regex_search(t, match, re) || match.size() < 2)
    THROW("Bad TXT date");

  (void) StringToNonzeroUnsigned(match.str(2), date.year);
  date.month = StringToMonth(match.str(1));

  Date::checkDate();
}


void Date::set(
  const string& t,
  const formatType f)
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


bool Date::operator == (const Date& d2) const
{
  if (date.year != d2.date.year)
    DIFF("Different years");

  if (date.month != d2.date.month)
    DIFF("Different months");

  if (date.day != d2.date.day)
    DIFF("Different days");

  return true;
}


bool Date::operator != (const Date& d2) const
{
  return ! (* this == d2);
}


string Date::asLIN() const
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


string Date::asPBN() const
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


string Date::asRBNCore() const
{
  if (date.month == 0 || date.year == 0)
    return "";

  stringstream s;
  s << date.year << setfill('0') << setw(2) << date.month;
  if (date.day > 0)
    s << date.day;
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
  if (date.month == 0 || date.year == 0)
    return "";

  stringstream s;
  if (date.day > 0)
    s << date.day << " ";
  s << DATE_MONTHS[date.month] << " " << date.year << "\n";
  return s.str();
}


string Date::asString(const formatType f) const
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

