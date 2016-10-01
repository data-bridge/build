/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Location.h"
#include "portab.h"
#include "Debug.h"

extern Debug debug;


Location::Location()
{
  Location::Reset();
}


Location::~Location()
{
}


void Location::Reset()
{
  location.general = "";
  location.specific = "";
}


bool Location::Set(
  const string& t,
  const formatType f)
{
  size_t pos;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN location format");
      return false;
    
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
      return true;
    
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
      return true;
    
    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


bool Location::operator == (const Location& l2) const
{
  if (location.general != l2.location.general)
  {
    LOG("General location differs");
    return false;
  }
  else if (location.specific != l2.location.specific)
  {
    LOG("Specific location differs");
    return false;
  }
  else
    return true;
}


bool Location::operator != (const Location& l2) const
{
  return ! (* this == l2);
}


string Location::AsString(const formatType f) const
{
  stringstream s;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      s << location.general;
      if (location.specific != "")
        s << ":" << location.specific;
      return s.str();
    
    case BRIDGE_FORMAT_PBN:
      if (location.specific == "")
        return "";
      s << "[Site \"" << location.general;
      if (location.specific != "")
        s << ":" << location.specific;
      return s.str() + "\"]\n";
    
    case BRIDGE_FORMAT_RBN:
      s << "L " << location.general;
      if (location.specific != "")
        s << ":" << location.specific;
      return s.str() + "\n";
    
    case BRIDGE_FORMAT_RBX:
      s << "L{" << location.general;
      if (location.specific != "")
        s << ":" << location.specific;
      return s.str() + "}";
    
    case BRIDGE_FORMAT_TXT:
      s << location.general;
      if (location.specific != "")
        s << ", " << location.specific;
      return s.str() + "\n";
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}

