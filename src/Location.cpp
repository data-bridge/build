/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>

#include "Location.h"
#include "Bexcept.h"
#include "Bdiff.h"


Location::Location()
{
  Location::reset();
}


Location::~Location()
{
}


void Location::reset()
{
  locGeneral = "";
  locSpecific = "";
}


void Location::setWithSeparator(
  const string& text,
  const string& separator)
{
  const size_t len = separator.length();
  size_t pos;

  if ((pos = text.find(separator, 0)) == string::npos)
    locGeneral = text;
  else if (text.length() < pos+len+1)
    locGeneral = text;
  else
  {
    locGeneral = text.substr(0, pos);
    locSpecific = text.substr(pos+len, string::npos);
  }
}


void Location::set(
  const string& text,
  const formatType format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      Location::setWithSeparator(text, ":");
      return;
    
    case BRIDGE_FORMAT_TXT:
      Location::setWithSeparator(text, ", ");
      return;
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


bool Location::operator == (const Location& location2) const
{
  if (locGeneral != location2.locGeneral)
    DIFF("General location differs");

  if (locSpecific != location2.locSpecific)
    DIFF("Specific location differs");

  return true;
}


bool Location::operator != (const Location& location2) const
{
  return ! (* this == location2);
}


string Location::strCore(const string& separator) const
{
  string str;
  str = locGeneral;
  if (locSpecific != "")
    str += separator + locSpecific;
  return str;
}


string Location::str(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return Location::strCore(":");
    
    case BRIDGE_FORMAT_PBN:
      return "[Site \"" + Location::strCore(":") + "\"]\n";
    
    case BRIDGE_FORMAT_RBN:
       if (locGeneral == "")
        return "L\n";
      else
        return "L " + Location::strCore(":") + "\n";
    
    case BRIDGE_FORMAT_RBX:
      return "L{" + Location::strCore(":") + "}";
    
    case BRIDGE_FORMAT_TXT:
       if (locGeneral == "")
         return "\n";
      return Location::strCore(", ") + "\n";
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}

