/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <regex>

#include "Session.h"
#include "parse.h"
#include "Bdiff.h"
#include "Bexcept.h"


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
  "Qualifying", 
  "Undefined"
};

const string SESSION_NAMES[] =
{
  "Segment",
  "Segment",
  "Segment",
  "Segment",
  "Segment",
  "Round",
  "Undefined",
};


Session::Session()
{
  Session::reset();
}


Session::~Session()
{
}


void Session::reset()
{
  general1 = "";
  stage = BRIDGE_SESSION_UNDEFINED;
  roundOf = 0;
  extension = "";

  general2 = "";
  sessionNo = 0;
  sessExt = "";
}


StageType Session::charToType(const char c) const
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


StageType Session::stringToType(
  const string& t,
  unsigned& rOf) const
{
  string s = t;
  toUpper(s);

  if (s == "FINAL")
    return BRIDGE_SESSION_FINAL;
  else if (s == "PLAYOFF")
    return BRIDGE_SESSION_PLAYOFF;
  else if (s == "SEMIFINAL")
    return BRIDGE_SESSION_SEMIFINAL;
  else if (s == "QUARTERFINAL")
    return BRIDGE_SESSION_QUARTERFINAL;
  else if (s == "QUALIFYING")
    return BRIDGE_SESSION_INITIAL;
  else if (s.at(0) != 'R' || s.length() <= 1)
    return BRIDGE_SESSION_UNDEFINED;

  regex re("^ROUND OF (.+)");
  smatch match;
  if (regex_search(s, match, re) && match.size() >= 1)
  {
    if (! StringToNonzeroUnsigned(match.str(1), rOf))
      return BRIDGE_SESSION_UNDEFINED;
    else
      return BRIDGE_SESSION_ROUND_OF;
  }

  const string rest = s.substr(1, string::npos);
  if (! StringToNonzeroUnsigned(rest, rOf))
    return BRIDGE_SESSION_UNDEFINED;

  if (rOf > 1024)
    return BRIDGE_SESSION_UNDEFINED;

  return BRIDGE_SESSION_ROUND_OF;
}


void Session::setPart1(const string& t)
{
  StageType r;
  unsigned u;
  size_t l = t.length();

  if (l == 0)
    return;
  else if (l == 1 || l == 2)
  {
    if ((r = Session::charToType(t.at(0))) == BRIDGE_SESSION_UNDEFINED)
    {
      general1 = t;
      stage = BRIDGE_SESSION_UNDEFINED;
    }
    else
    {
      stage = r;
      extension = t.substr(1);
    }
    return;
  }

  string ext = "";
  string s = t;
  if (s.at(l-1) != ' ' && s.at(l-2) == ' ')
  {
    // Might be Semifinal B, R64 B, Round of 32 B etc.
    // Undocumented Pavlicek extension!
    ext = s.substr(l-1);
    s = s.substr(0, l-2);
  }
  else if (s.at(0) == 'R' && s.at(l-1) != ' ' && 
      (s.at(l-1) < '0' || s.at(l-1) > '9') &&
      s.at(l-2) >= '0' && s.at(l-2) <= '9')
  {
    // Might be R32B.  Undocumented Pavlicek extension!
    ext = s.substr(l-1);
    s = s.substr(0, l-1);
  }

  if ((r = Session::stringToType(s, u)) == BRIDGE_SESSION_UNDEFINED)
  {
    general1 = s + ext;
    stage = BRIDGE_SESSION_UNDEFINED;
  }
  else if (r == BRIDGE_SESSION_ROUND_OF)
  {
    stage = r;
    roundOf = u;
    extension = ext;
  }
  else
  {
    stage = r;
    extension = ext;
  }
}


void Session::setPart2(const string& t)
{
  unsigned u;
  if (t.length() <= 6)
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
    return;
  }

  if (StringToNonzeroUnsigned(t, u))
  {
    // Could be "5 (overtime)", for example.
    general2 = "";
    sessionNo = u;

    unsigned pos = 0;
    while (pos < t.length() && t.at(pos) != ' ')
      pos++;
    if (pos == t.length())
      return;

    while (pos < t.length() && t.at(pos) == ' ')
      pos++;
    if (pos == t.length())
      return;

    sessExt = t.substr(pos);
    return;
  }

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
    return;
  }

  regex rer("^Round (.+)");
  if (regex_search(t, match, rer) && match.size() >= 1)
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


void Session::set(
  const string& t,
  const formatType f)
{
  size_t pos;
  string s;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      THROW("Cannot set LIN session format");
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if ((pos = t.find(":", 0)) == string::npos || t.length() < pos+2)
      {
        general1 = t;
        stage = BRIDGE_SESSION_UNDEFINED;
        general2 = "";
        sessionNo = 0;
        return;
      }

      s = t.substr(0, pos);
      Session::setPart1(s);

      s = t.substr(pos+1, string::npos);
      Session::setPart2(s);
      return;
    
    case BRIDGE_FORMAT_TXT:
      if ((pos = t.find(", ", 0)) == string::npos || t.length() < pos+3)
      {
        general1 = t;
        stage = BRIDGE_SESSION_UNDEFINED;
        return;
      }

      s = t.substr(0, pos);
      Session::setPart1(s);

      s = t.substr(pos+2, string::npos);
      Session::setPart2(s);
      return;
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


bool Session::isRBNPart(const string& t) const
{
  unsigned u;
  return (Session::stringToType(t, u) != BRIDGE_SESSION_UNDEFINED);
}


bool Session::operator == (const Session& s2) const
{
  if (stage != s2.stage)
    DIFF("Session stage differs");

  if (stage == BRIDGE_SESSION_UNDEFINED)
  {
    if (general1 != s2.general1)
      DIFF("General session 1 differs");
  }
  else if (stage == BRIDGE_SESSION_ROUND_OF)
  {
    if (roundOf != s2.roundOf)
      DIFF("Session round-of differs");
  }

  if (general2 != s2.general2)
    DIFF("General session 2 differs");

  if (general2 == "")
  {
    if (sessionNo != s2.sessionNo)
      DIFF("Session number differs");
  }

  return true;
}


bool Session::operator != (const Session& s2) const
{
  return ! (* this == s2);
}


string Session::asLIN() const
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


string Session::asLIN_RP() const
{
  // This is a fudge.  We are putting RBN tags in the LIN header.
  if (stage == BRIDGE_SESSION_UNDEFINED)
    return ",";

  stringstream s;

  // This is an odd Pavlicek choice.
  if (stage != BRIDGE_SESSION_ROUND_OF && extension != "")
  {
    s << "," << STAGE_NAMES_SHORT[stage] << extension;
  }
  else if (stage == BRIDGE_SESSION_ROUND_OF)
  {
    if (extension != "" && roundOf >= 64)
      s << ",R" << roundOf << " " << extension;
    else
      s << ",R" << roundOf << extension;
  }
  else
  {
    // For some reason Pavlicek doesn't do this for Rxy.
    s << " " << STAGE_NAMES[stage] << extension << ",";
    if (sessionNo > 0)
    {
      s << SESSION_NAMES[stage] << " " << sessionNo;
      if (sessExt != "")
        s << " " << sessExt;
    }
    else
      s << general2;
  }

  return s.str();
}


string Session::asPBN() const
{
  if (general1 == "" && stage == BRIDGE_SESSION_UNDEFINED)
    return "";

  stringstream s;
  s << "[Stage \"";
  if (stage == BRIDGE_SESSION_UNDEFINED)
    s << general1;
  else if (stage != BRIDGE_SESSION_ROUND_OF)
    s << STAGE_NAMES_SHORT[stage] << extension;
  else if (extension != "" && roundOf >= 64)
    // Pavlicek...
    s << STAGE_NAMES_SHORT[stage] << roundOf << " " << extension;
  else
    s << STAGE_NAMES_SHORT[stage] << roundOf << extension;

  if (sessionNo > 0)
  {
    s << ":" << sessionNo;
    if (sessExt != "")
      s << " " << sessExt;
  }
  else if (general2 != "")
    s << ":" << general2;

  return s.str() + "\"]\n";
}


string Session::asRBNCore() const
{
  // TODO: Could also be used in asPBN
  stringstream s;
  if (stage == BRIDGE_SESSION_UNDEFINED)
    s << general1;
  else if (stage != BRIDGE_SESSION_ROUND_OF)
    s << STAGE_NAMES_SHORT[stage] << extension;
  else if (extension != "" && roundOf >= 64)
    // Pavlicek...
    s << STAGE_NAMES_SHORT[stage] << roundOf << " " << extension;
  else
    s << STAGE_NAMES_SHORT[stage] << roundOf << extension;

  if (sessionNo > 0)
  {
    s << ":" << sessionNo;
    if (sessExt != "")
      s << " " << sessExt;
  }
  else if (general2 != "")
    s << ":" << general2;

  return s.str();
}


string Session::asRBN() const
{
  return "S " + Session::asRBNCore() + "\n";
}


string Session::asRBX() const
{
  return "S{" + Session::asRBNCore() + "}";
}


string Session::asTXT() const
{
  stringstream s;
  if (stage == BRIDGE_SESSION_UNDEFINED)
    s << general1;
  else if (stage != BRIDGE_SESSION_ROUND_OF)
  {
    s << STAGE_NAMES[stage];
    if (extension != "")
      s << " " << extension;
  }
  else if (extension != "" && roundOf >= 64)
    // Pavlicek...
    s << STAGE_NAMES[stage] << roundOf << " " << extension;
  else
    s << STAGE_NAMES[stage] << roundOf << extension;

  if (sessionNo > 0)
  {
    if (sessExt == "")
      s << ", " << SESSION_NAMES[stage] << " " << sessionNo;
    else
      // Pavlicek bug?
      s << ", " << sessionNo << " " << sessExt;
  }
  else if (general2 != "")
    s << ", " << general2;

  return s.str() + "\n";
}


string Session::asString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Session::asLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Session::asLIN_RP();

    case BRIDGE_FORMAT_PBN:
      return Session::asPBN();

    case BRIDGE_FORMAT_RBN:
      return Session::asRBN();

    case BRIDGE_FORMAT_RBX:
      return Session::asRBX();

    case BRIDGE_FORMAT_TXT:
      return Session::asTXT();

    default:
      THROW("No such format");
  }
}

