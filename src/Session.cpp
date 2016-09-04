/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <algorithm>
#include "Session.h"
#include "portab.h"
#include "parse.h"
#include "debug.h"

extern Debug debug;


const string STAGE_NAMES[] =
{
  "Final", 
  "Playoff", 
  "Semifinal", 
  "Quarterfinal",
  "Round of ", 
  "Initial", 
  "Undefined"
};


Session::Session()
{
  Session::Reset();
}


Session::~Session()
{
}


void Session::Reset()
{
  general = "";
  stage = BRIDGE_SESSION_UNDEFINED;
  roundOf = 0;
  sessionNo = 0;
}


stageType Session::CharToType(const char c) const
{
  switch(c)
  {
    case 'F':
    case 'f':
      return BRIDGE_SESSION_FINAL;

    case 'P':
    case 'p':
      return BRIDGE_SESSION_PLAYOFF;

    case 'S':
    case 's':
      return BRIDGE_SESSION_SEMIFINAL;

    case 'Q':
    case 'q':
      return BRIDGE_SESSION_QUARTERFINAL;

    case 'I':
    case 'i':
      return BRIDGE_SESSION_INITIAL;

    default:
      return BRIDGE_SESSION_UNDEFINED;
  }
}


stageType Session::StringToType(
  const string& t,
  unsigned& rOf) const
{
  string s;
  // Can't get this to work with the Microsoft compiler.
  // transform(t.begin(), t.end(), s.begin(), (int (*)(int))tolower);
  for (unsigned i = 0; i < t.length(); i++)
    s[i] = static_cast<char>(tolower(t[i]));

  if (s == "final")
    return BRIDGE_SESSION_FINAL;
  else if (s == "playoff")
    return BRIDGE_SESSION_PLAYOFF;
  else if (s == "semifinal")
    return BRIDGE_SESSION_SEMIFINAL;
  else if (s == "quarterfinal")
    return BRIDGE_SESSION_QUARTERFINAL;
  else if (s == "qualifying")
    return BRIDGE_SESSION_INITIAL;
  else if (s.at(0) != 'r' || s.length() <= 1)
    return BRIDGE_SESSION_UNDEFINED;

  const string rest = s.substr(1, string::npos);
  if (! StringToUnsigned(rest, rOf))
    return BRIDGE_SESSION_UNDEFINED;

  if (rOf > 1024)
    return BRIDGE_SESSION_UNDEFINED;

  return BRIDGE_SESSION_ROUND_OF;
}


bool Session::Set(
  const string& t,
  const formatType f)
{
  size_t pos;
  stageType r;
  unsigned u;
  string s;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN session format");
      return false;
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
      if ((pos = t.find(":", 0)) == string::npos || t.length() < pos+2)
      {
        general = t;
        return true;
      }

      if (pos == 0)
      {
        // No change.
      }
      else if (pos == 1)
      {
        if ((r = Session::CharToType(t.at(0))) == BRIDGE_SESSION_UNDEFINED)
          general = t;
        else
          stage = r;
      }
      else
      {
        s = t.substr(0, pos-1);
        if ((r = Session::StringToType(t, u)) == BRIDGE_SESSION_UNDEFINED)
          general = t;
        else
        {
          stage = r;
          roundOf = u;
        }
      }

      s = t.substr(pos+1, string::npos);
      if (! StringToUnsigned(s, u))
      {
        stage = BRIDGE_SESSION_UNDEFINED;
        general = t;
      }
      else
        sessionNo = u;
      return true;
    
    case BRIDGE_FORMAT_TXT:
      if ((pos = t.find(", ", 0)) == string::npos || t.length() < pos+3)
      {
        general = t;
        return true;
      }

      if (pos >= 2)
      {
        s = t.substr(0, pos-1);
        if ((r = Session::StringToType(t, u)) == BRIDGE_SESSION_UNDEFINED)
          general = t;
        else
        {
          stage = r;
          roundOf = u;
        }
      }

      s = t.substr(pos+2, string::npos);
      if (! StringToUnsigned(s, u))
      {
        stage = BRIDGE_SESSION_UNDEFINED;
        general = t;
      }
      else
        sessionNo = u;
      return true;
    
    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


bool Session::IsRBNPart(const string& t) const
{
  unsigned u;
  return (Session::StringToType(t, u) != BRIDGE_SESSION_UNDEFINED);
}


bool Session::operator == (const Session& s2) const
{
  if (general != s2.general)
  {
    LOG("General session differs");
    return false;
  }
  else if (stage != s2.stage)
  {
    LOG("Session stage differs");
    return false;
  }
  else if (roundOf != s2.roundOf)
  {
    LOG("Session round-of differs");
    return false;
  }
  else if (sessionNo != s2.sessionNo)
  {
    LOG("Session number differs");
    return false;
  }
  else
    return true;
}


bool Session::operator != (const Session& s2) const
{
  return ! (* this == s2);
}


string Session::AsString(const formatType f) const
{
  UNUSED(f);
  // TODO
  return "";
}

