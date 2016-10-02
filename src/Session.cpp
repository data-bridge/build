/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <algorithm>
#include <regex>
#include "Session.h"
#include "Debug.h"
#include "portab.h"
#include "parse.h"

extern Debug debug;


const string STAGE_NAMES_SHORT[] =
{
  "F", 
  "P", 
  "S", 
  "Q",
  "R", 
  "I", 
  "Undefined"
};

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
  general1 = "";
  stage = BRIDGE_SESSION_UNDEFINED;
  roundOf = 0;
  general2 = "";
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
  string s = t;
  // Can't get this to work with the Microsoft compiler.
  // transform(t.begin(), t.end(), s.begin(), (int (*)(int))tolower);
  for (unsigned i = 0; i < s.length(); i++)
    s[i] = static_cast<char>(tolower(s[i]));

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

  try
  {
    regex re("^round of (.+)");
    smatch match;
    if (regex_search(s, match, re) && match.size() >= 1)
    {
      if (! StringToNonzeroUnsigned(match.str(1), rOf))
        return BRIDGE_SESSION_UNDEFINED;
      else
        return BRIDGE_SESSION_ROUND_OF;
    }
  }
  catch (regex_error& e)
  {
    UNUSED(e);
  }

  const string rest = s.substr(1, string::npos);
  if (! StringToNonzeroUnsigned(rest, rOf))
    return BRIDGE_SESSION_UNDEFINED;

  if (rOf > 1024)
    return BRIDGE_SESSION_UNDEFINED;

  return BRIDGE_SESSION_ROUND_OF;
}


void Session::SetPart1(const string& t)
{
  stageType r;
  unsigned u;
  size_t l = t.length();

  if (l == 0)
    return;
  else if (l == 1)
  {
    if ((r = Session::CharToType(t.at(0))) == BRIDGE_SESSION_UNDEFINED)
    {
      general1 = t;
      stage = BRIDGE_SESSION_UNDEFINED;
    }
    else
      stage = r;
  }
  else if ((r = Session::StringToType(t, u)) == BRIDGE_SESSION_UNDEFINED)
  {
    general1 = t;
    stage = BRIDGE_SESSION_UNDEFINED;
  }
  else if (r == BRIDGE_SESSION_ROUND_OF)
  {
    stage = r;
    roundOf = u;
  }
  else
    stage = r;
}


void Session::SetPart2(const string& t)
{
  unsigned u;
  if (t.length() <= 7)
  {
    if (! StringToNonzeroUnsigned(t, u))
    {
      general2 = t;
      sessionNo = 0;
    }
    else
    {
      general2 = "";
      sessionNo = u;
    }
  }
  else
  {
    try
    {
      regex re("^Segment (.+)");
      smatch match;
      if (regex_search(t, match, re) && match.size() >= 1)
      {
        if (! StringToNonzeroUnsigned(match.str(1), u))
        {
          general2 = t;
          sessionNo = 0;
        }
        else
        {
          general2 = "";
          sessionNo = u;
        }
      }
      else
      {
        general2 = t;
        sessionNo = 0;
      }
    }
    catch (regex_error& e)
    {
      UNUSED(e);
      general2 = t;
      sessionNo = 0;
    }
  }
}


bool Session::Set(
  const string& t,
  const formatType f)
{
  size_t pos;
  string s;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN session format");
      return false;
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if ((pos = t.find(":", 0)) == string::npos || t.length() < pos+2)
      {
        general1 = t;
        stage = BRIDGE_SESSION_UNDEFINED;
        general2 = "";
        sessionNo = 0;
        return true;
      }

      s = t.substr(0, pos);
      Session::SetPart1(s);

      s = t.substr(pos+1, string::npos);
      Session::SetPart2(s);
      return true;
    
    case BRIDGE_FORMAT_TXT:
      if ((pos = t.find(", ", 0)) == string::npos || t.length() < pos+3)
      {
        general1 = t;
        stage = BRIDGE_SESSION_UNDEFINED;
        return true;
      }

      s = t.substr(0, pos);
      Session::SetPart1(s);

      s = t.substr(pos+2, string::npos);
      Session::SetPart2(s);
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
  if (stage != s2.stage)
  {
    LOG("Session stage differs");
    return false;
  }

  if (stage == BRIDGE_SESSION_UNDEFINED)
  {
    if (general1 != s2.general1)
    {
      LOG("General session 1 differs");
      return false;
    }
  }
  else if (stage == BRIDGE_SESSION_ROUND_OF)
  {
    if (roundOf != s2.roundOf)
    {
      LOG("Session round-of differs");
      return false;
    }
  }

  if (general2 != s2.general2)
  {
    LOG("General session 2 differs");
    return false;
  }

  if (general2 == "")
  {
    if (sessionNo != s2.sessionNo)
    {
      LOG("Session number differs");
      return false;
    }
  }

  return true;
}


bool Session::operator != (const Session& s2) const
{
  return ! (* this == s2);
}


string Session::AsLIN() const
{
  // This is a fudge.  We are putting RBN tags in the LIN header.
  if (stage == BRIDGE_SESSION_UNDEFINED)
    return "";

  stringstream s;
  if (stage != BRIDGE_SESSION_ROUND_OF)
    s << STAGE_NAMES_SHORT[stage];
  else
    s << STAGE_NAMES_SHORT[stage] << roundOf;

  if (sessionNo > 0)
    s << ":" << sessionNo;
  else if (general2 != "")
    s << ":" << general2;

  return s.str();
}


string Session::AsLIN_RP() const
{
  // This is a fudge.  We are putting RBN tags in the LIN header.
  if (stage == BRIDGE_SESSION_UNDEFINED)
    return ",";

  stringstream s;
  if (stage != BRIDGE_SESSION_ROUND_OF)
    s << " " << STAGE_NAMES[stage] << ",";
  else
    s << " " << STAGE_NAMES[stage] << roundOf << ",";

  if (sessionNo > 0)
    s << "Segment " << sessionNo;

  return s.str();
}


string Session::AsPBN() const
{
  if (general1 == "" && stage == BRIDGE_SESSION_UNDEFINED)
    return "";

  stringstream s;
  s << "[Stage \"";
  if (stage == BRIDGE_SESSION_UNDEFINED)
    s << general1;
  else if (stage != BRIDGE_SESSION_ROUND_OF)
    s << STAGE_NAMES_SHORT[stage];
  else
    s << STAGE_NAMES_SHORT[stage] << roundOf;

  if (sessionNo > 0)
    s << ":" << sessionNo;
  else if (general2 != "")
    s << ":" << general2;

  return s.str() + "\"]\n";
}


string Session::AsRBNCore() const
{
  stringstream s;
  if (stage == BRIDGE_SESSION_UNDEFINED)
    s << general1;
  else if (stage != BRIDGE_SESSION_ROUND_OF)
    s << STAGE_NAMES_SHORT[stage];
  else
    s << STAGE_NAMES_SHORT[stage] << roundOf;

  if (sessionNo > 0)
    s << ":" << sessionNo;
  else if (general2 != "")
    s << ":" << general2;

  return s.str();
}


string Session::AsRBN() const
{
  return "S " + Session::AsRBNCore() + "\n";
}


string Session::AsRBX() const
{
  return "S{" + Session::AsRBNCore() + "}";
}


string Session::AsTXT() const
{
  stringstream s;
  if (stage == BRIDGE_SESSION_UNDEFINED)
    s << general1;
  else if (stage != BRIDGE_SESSION_ROUND_OF)
    s << STAGE_NAMES[stage];
  else
    s << STAGE_NAMES[stage] << roundOf;

  if (sessionNo > 0)
    s << ", Segment " << sessionNo;
  else if (general2 != "")
    s << ", " << general2;

  return s.str() + "\n";
}


string Session::AsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Session::AsLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Session::AsLIN_RP();

    case BRIDGE_FORMAT_PBN:
      return Session::AsPBN();

    case BRIDGE_FORMAT_RBN:
      return Session::AsRBN();

    case BRIDGE_FORMAT_RBX:
      return Session::AsRBX();

    case BRIDGE_FORMAT_TXT:
      return Session::AsTXT();

    default:
      LOG("No such format");
      return "";
  }
}

