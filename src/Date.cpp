/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iomanip>
#include <sstream>
#include <regex>
#pragma warning(pop)

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
    THROW("Year is too large");

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


void Date::setLIN(const string& text)
{
  // Note that this parses the date in the *file name*, not a real
  // date field.

  // 12-31-14
  regex re("(\\d\\d)-(\\d\\d)-(\\d\\d)");
  smatch match;
  if (! regex_search(text, match, re) || match.size() < 3)
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


void Date::setTriple(
  const string& y,
  const string& m,
  const string& d)
{
  if (y == "????")
    year = 0;
  else if (! str2upos(y, year))
    THROW("Bad PBN year: " + y);

  if (m == "??")
    month = 0;
  else if (! str2upos(m, month))
    THROW("Bad PBN month number: " + m);

  if (d == "??")
    day = 0;
  else if (! str2upos(d, day))
    THROW("Bad PBN day: " + d);

  Date::check();
}


void Date::setPBN(const string& text)
{
  smatch match;

  // 2010.12.31
  regex re("(....)\\.(..)\\.(..)");
  if (regex_search(text, match, re) && match.size() >= 3)
  {
    Date::setTriple(match.str(1), match.str(2), match.str(3));
    return;
  }

  // 2010-12-31 -- not legal PBN.
  regex re2("(....)-(..)-(..)");
  if (regex_search(text, match, re2) && match.size() >= 3)
  {
    Date::setTriple(match.str(1), match.str(2), match.str(3));
    return;
  }

  // 02.17.2013 -- definitely not legal PBN.
  regex re3("(..)\\.(..)\\.(....)");
  if (regex_search(text, match, re3) && match.size() >= 3)
  {
    Date::setTriple(match.str(3), match.str(1), match.str(2));
    return;
  }

  THROW("Bad PBN date:" + text);
}


void Date::setRBN(const string& text)
{
  if (text.length() == 6)
  {
    // 201001
    regex re("(\\d\\d\\d\\d)(\\d\\d)");
    smatch match;
    if (! regex_search(text, match, re) || match.size() < 2)
      THROW("Bad RBN date");

    (void) str2upos(match.str(1), year);
    (void) str2upos(match.str(2), month);
  }
  else
  {
    // 20101231
    regex re("(\\d\\d\\d\\d)(\\d\\d)(\\d\\d)");
    smatch match;
    if (! regex_search(text, match, re) || match.size() < 3)
      THROW("Bad RBN date");

    (void) str2upos(match.str(1), year);
    (void) str2upos(match.str(2), month);
    (void) str2upos(match.str(3), day);
  }

  Date::check();
}


void Date::setTXT(const string& text)
{
  // January 1999
  regex re("(\\w+) (\\d\\d\\d\\d)");
  smatch match;
  if (! regex_search(text, match, re) || match.size() < 2)
    THROW("Bad TXT date");

  (void) str2upos(match.str(2), year);
  month = str2month(match.str(1));

  Date::check();
}


void Date::set(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
      Date::setLIN(text);
      return;
    
    case BRIDGE_FORMAT_PBN:
      Date::setPBN(text);
      return;
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      Date::setRBN(text);
      return;
    
    case BRIDGE_FORMAT_TXT:
      Date::setTXT(text);
      return;
    
    default:
      THROW("Invalid format: " + to_string(format));
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


string Date::strLIN() const
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


string Date::strPBN() const
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


string Date::strRBNCore() const
{
  if (month == 0 || year == 0)
    return "";

  stringstream s;
  s << year << setfill('0') << setw(2) << month;
  if (day > 0)
    s << day;
  return s.str();
}

string Date::strRBN() const
{
  return "D " + Date::strRBNCore() + "\n";
}


string Date::strRBX() const
{
  return "D{" + Date::strRBNCore() + "}";
}


string Date::strTXT() const
{
  if (month == 0 || year == 0)
    return "\n";

  stringstream s;
  if (day > 0)
    s << day << " ";
  s << DATE_MONTHS[month] << " " << year << "\n";
  return s.str();
}


string Date::str(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
      return Date::strLIN();
    
    case BRIDGE_FORMAT_PBN:
      return Date::strPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Date::strRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Date::strRBX();
    
    case BRIDGE_FORMAT_TXT:
      return Date::strTXT();
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
}

