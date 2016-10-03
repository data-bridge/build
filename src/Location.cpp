/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>

#include "Location.h"
#include "Bdiff.h"
#include "Bexcept.h"


Location::Location()
{
  Location::reset();
}


Location::~Location()
{
}


void Location::reset()
{
  location.general = "";
  location.specific = "";
}


void Location::set(
  const string& t,
  const formatType f)
{
  size_t pos;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      THROW("No LIN location format");
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if ((pos = t.find(":", 0)) == string::npos)
        location.general = t;
      else if (t.length() < pos+2)
        location.general = t;
      else
      {
        location.general = t.substr(0, pos);
        location.specific = t.substr(pos+1, string::npos);
      }
      return;
    
    case BRIDGE_FORMAT_TXT:
      if ((pos = t.find(", ", 0)) == string::npos)
        location.general = t;
      else
      {
        if (t.length() < pos+3)
          location.general = t;
        else
        {
          location.general = t.substr(0, pos);
          location.specific = t.substr(pos+2, string::npos);
        }
      }
      return;
    
    default:
      THROW("Invalid format: " + STR(f));
  }
}


bool Location::operator == (const Location& l2) const
{
  if (location.general != l2.location.general)
    DIFF("General location differs");

  if (location.specific != l2.location.specific)
    DIFF("Specific location differs");

  return true;
}


bool Location::operator != (const Location& l2) const
{
  return ! (* this == l2);
}


string Location::asRBN() const
{
  stringstream s;
  s << location.general;
  if (location.specific != "")
    s << ":" << location.specific;
  return s.str();
}


string Location::asString(const formatType f) const
{
  stringstream s;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Location::asRBN();
    
    case BRIDGE_FORMAT_PBN:
      if (location.specific == "")
        return "";
      return "[Site \"" + Location::asRBN() + "\"]\n";
    
    case BRIDGE_FORMAT_RBN:
      return "L " + Location::asRBN() + "\n";
    
    case BRIDGE_FORMAT_RBX:
      return "L{" + Location::asRBN() + "}";
    
    case BRIDGE_FORMAT_TXT:
      s << location.general;
      if (location.specific != "")
        s << ", " << location.specific << "\n";
      return s.str();
    
    default:
      THROW("Invalid format: " + STR(f));
  }
}

