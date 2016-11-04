/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


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


Stage Session::charToStage(const char c) const
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


Stage Session::stringToStage(
  const string& text,
  unsigned& rOf) const
{
  string st = text;
  toUpper(st);

  if (st == "FINAL")
    return BRIDGE_SESSION_FINAL;
  else if (st == "PLAYOFF")
    return BRIDGE_SESSION_PLAYOFF;
  else if (st == "SEMIFINAL")
    return BRIDGE_SESSION_SEMIFINAL;
  else if (st == "QUARTERFINAL")
    return BRIDGE_SESSION_QUARTERFINAL;
  else if (st == "QUALIFYING")
    return BRIDGE_SESSION_INITIAL;
  else if (st.at(0) != 'R' || st.length() <= 1)
    return BRIDGE_SESSION_UNDEFINED;

  regex re("^ROUND OF (.+)");
  smatch match;
  if (regex_search(st, match, re) && match.size() >= 1)
  {
    if (! str2upos(match.str(1), rOf))
      return BRIDGE_SESSION_UNDEFINED;
    else
      return BRIDGE_SESSION_ROUND_OF;
  }

  const string rest = st.substr(1);
  if (! str2upos(rest, rOf))
    return BRIDGE_SESSION_UNDEFINED;

  if (rOf > 1024)
    return BRIDGE_SESSION_UNDEFINED;

  return BRIDGE_SESSION_ROUND_OF;
}


void Session::setPart1(const string& text)
{
  size_t l = text.length();
  Stage r;
  unsigned u;

  if (l == 0)
    return;
  else if (l == 1 || l == 2)
  {
    if ((r = Session::charToStage(text.at(0))) == BRIDGE_SESSION_UNDEFINED)
    {
      general1 = text;
      stage = BRIDGE_SESSION_UNDEFINED;
    }
    else
    {
      stage = r;
      extension = text.substr(1);
    }
    return;
  }

  string ext = "";
  string st = text;
  if (st.at(l-1) != ' ' && st.at(l-2) == ' ')
  {
    // Might be Semifinal B, R64 B, Round of 32 B etc.
    // Undocumented Pavlicek extension!
    ext = st.substr(l-1);
    st = st.substr(0, l-2);
  }
  else if (st.at(0) == 'R' && st.at(l-1) != ' ' && 
      (st.at(l-1) < '0' || st.at(l-1) > '9') &&
      st.at(l-2) >= '0' && st.at(l-2) <= '9')
  {
    // Might be R32B.  Undocumented Pavlicek extension!
    ext = st.substr(l-1);
    st = st.substr(0, l-1);
  }

  if ((r = Session::stringToStage(st, u)) == BRIDGE_SESSION_UNDEFINED)
  {
    general1 = st + ext;
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


void Session::setPart2Numeric(
  const string& possNumber,
  const string& text)
{
  unsigned u;
  if (! str2upos(possNumber, u))
  {
    general2 = text;
    sessionNo = 0;
  }
  else
  {
    general2 = "";
    sessionNo = u;
  }
}


void Session::setPart2(const string& text)
{
  unsigned u;
  if (text.length() <= 6)
  {
    Session::setPart2Numeric(text, text);
    return;
  }

  if (str2upos(text, u))
  {
    // Could be "5 (overtime)", for example.
    general2 = "";
    sessionNo = u;

    unsigned pos = 0;
    while (pos < text.length() && text.at(pos) != ' ')
      pos++;
    if (pos == text.length())
      return;

    while (pos < text.length() && text.at(pos) == ' ')
      pos++;
    if (pos == text.length())
      return;

    sessExt = text.substr(pos);
    return;
  }

  regex re("^Segment (.+)");
  regex rer("^Round (.+)");
  smatch match;
  if (regex_search(text, match, re) && match.size() >= 1)
  {
    Session::setPart2Numeric(match.str(1), text);
  }
  else if (regex_search(text, match, rer) && match.size() >= 1)
  {
    Session::setPart2Numeric(match.str(1), text);
  }
  else
  {
    general2 = text;
    sessionNo = 0;
  }
}


void Session::setSeparated(
  const string& text,
  const string& separator)
{
  const size_t l = separator.length();
  size_t pos;
  string st;

  if ((pos = text.find(separator, 0)) == string::npos || 
      text.length() < pos+1+l)
  {
    general1 = text;
    stage = BRIDGE_SESSION_UNDEFINED;
    general2 = "";
    sessionNo = 0;
    return;
  }

  st = text.substr(0, pos);
  Session::setPart1(st);

  st = text.substr(pos+l);
  Session::setPart2(st);
}


void Session::set(
  const string& text,
  const Format format)
{

  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      Session::setSeparated(text, ":");
      return;
    
    case BRIDGE_FORMAT_TXT:
      Session::setSeparated(text, ", ");
      return;
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


bool Session::isStage(const string& text) const
{
  unsigned u;
  return (Session::stringToStage(text, u) != BRIDGE_SESSION_UNDEFINED);
}


bool Session::isSegmentLike(const string& text) const
{
  regex re1("^Segment\\s+\\d+$");
  regex re2("^Round\\s+\\d+$");
  regex re3("^Segment\\s+\\d+ \\(overtime\\)$");

  smatch match;
  return (
      regex_search(text, match, re1) ||
      regex_search(text, match, re2) ||
      regex_search(text, match, re3) ||
      text == "Overtime");
}


bool Session::isRoundOfLike(const string& text) const
{
  regex re1("^R(\\d+)$");
  regex re2("^R(\\d+)[ABCD]$");
  regex re3("^R(\\d+) [ABCD]$");
  regex re4("^[QS][ABCD]$");
  smatch match;

  return (
      regex_search(text, match, re1) ||
      regex_search(text, match, re2) ||
      regex_search(text, match, re3) ||
      regex_search(text, match, re4));
}


bool Session::operator == (const Session& session2) const
{
  if (stage != session2.stage)
    DIFF("Session stage differs");

  if (stage == BRIDGE_SESSION_UNDEFINED)
  {
    if (general1 != session2.general1)
      DIFF("General session 1 differs");
  }
  else if (stage == BRIDGE_SESSION_ROUND_OF)
  {
    if (roundOf != session2.roundOf)
      DIFF("Session round-of differs");
  }

  if (general2 != session2.general2)
    DIFF("General session 2 differs");

  if (general2 == "")
  {
    if (sessionNo != session2.sessionNo)
      DIFF("Session number differs");
  }

  return true;
}


bool Session::operator != (const Session& session2) const
{
  return ! (* this == session2);
}


string Session::strLIN() const
{
  // This is a fudge.  We are putting RBN tags in the LIN header.
  if (stage == BRIDGE_SESSION_UNDEFINED)
    return "";

  stringstream ss;
  if (stage != BRIDGE_SESSION_ROUND_OF)
    ss << STAGE_NAMES_SHORT[stage];
  else
    ss << STAGE_NAMES_SHORT[stage] << roundOf;

  if (sessionNo > 0)
    ss << ":" << sessionNo;
  else if (general2 != "")
    ss << ":" << general2;

  return ss.str();
}


string Session::strLIN_RP() const
{
  // This is a fudge.  We are putting RBN tags in the LIN header.
  if (stage == BRIDGE_SESSION_UNDEFINED)
    return ",";

  stringstream ss;

  // This is an odd Pavlicek choice.
  if (stage != BRIDGE_SESSION_ROUND_OF && extension != "")
  {
    ss << "," << STAGE_NAMES_SHORT[stage] << extension;
  }
  else if (stage == BRIDGE_SESSION_ROUND_OF)
  {
    if (extension != "" && roundOf >= 64)
      ss << ",R" << roundOf << " " << extension;
    else
      ss << ",R" << roundOf << extension;
  }
  else
  {
    // For some reason Pavlicek doesn't do this for Rxy.
    ss << " " << STAGE_NAMES[stage] << extension << ",";
    if (sessionNo > 0)
    {
      ss << SESSION_NAMES[stage] << " " << sessionNo;
      if (sessExt != "")
        ss << " " << sessExt;
    }
    else
      ss << general2;
  }

  return ss.str();
}


string Session::strPBN() const
{
  if (general1 == "" && stage == BRIDGE_SESSION_UNDEFINED)
    return "";

  return "[Stage \"" + Session::strRBNCore() + "\"]\n";
}


string Session::strRBNCore() const
{
  stringstream ss;
  if (stage == BRIDGE_SESSION_UNDEFINED)
    ss << general1;
  else if (stage != BRIDGE_SESSION_ROUND_OF)
    ss << STAGE_NAMES_SHORT[stage] << extension;
  else if (extension != "" && roundOf >= 64)
    // Pavlicek...
    ss << STAGE_NAMES_SHORT[stage] << roundOf << " " << extension;
  else
    ss << STAGE_NAMES_SHORT[stage] << roundOf << extension;

  if (sessionNo > 0)
  {
    ss << ":" << sessionNo;
    if (sessExt != "")
      ss << " " << sessExt;
  }
  else if (general2 != "")
    ss << ":" << general2;

  return ss.str();
}


string Session::strRBN() const
{
  return "S " + Session::strRBNCore() + "\n";
}


string Session::strRBX() const
{
  return "S{" + Session::strRBNCore() + "}";
}


string Session::strTXT() const
{
  stringstream ss;
  if (stage == BRIDGE_SESSION_UNDEFINED)
    ss << general1;
  else if (stage != BRIDGE_SESSION_ROUND_OF)
  {
    ss << STAGE_NAMES[stage];
    if (extension != "")
      ss << " " << extension;
  }
  else if (extension != "" && roundOf >= 64)
    // Pavlicek...
    ss << STAGE_NAMES[stage] << roundOf << " " << extension;
  else
    ss << STAGE_NAMES[stage] << roundOf << extension;

  if (sessionNo > 0)
  {
    if (sessExt == "")
      ss << ", " << SESSION_NAMES[stage] << " " << sessionNo;
    else
      // Pavlicek bug?
      ss << ", " << sessionNo << " " << sessExt;
  }
  else if (general2 != "")
    ss << ", " << general2;

  return ss.str() + "\n";
}


string Session::str(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return Session::strLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Session::strLIN_RP();

    case BRIDGE_FORMAT_PBN:
      return Session::strPBN();

    case BRIDGE_FORMAT_RBN:
      return Session::strRBN();

    case BRIDGE_FORMAT_RBX:
      return Session::strRBX();

    case BRIDGE_FORMAT_TXT:
      return Session::strTXT();

    default:
      THROW("Invalid format: " + STR(format));
  }
}

