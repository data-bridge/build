/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <regex>
#include <sstream>
#include <iomanip>

#include "Teams.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


Teams::Teams()
{
  Teams::reset();
}


Teams::~Teams()
{
}


void Teams::reset()
{
  team1.reset();
  team2.reset();
}


void Teams::setPBN(
  const string& text1,
  const string& text2)
{
  team1.set(text1, BRIDGE_FORMAT_PBN);
  team2.set(text2, BRIDGE_FORMAT_PBN);
}


void Teams::setRBN4(const vector<string>& v)
{
  team1.setPair(v[0], v[2]);
  team2.setPair(v[1], v[3]);

  if (team1.carry == BRIDGE_CARRY_INT && 
      team2.carry == BRIDGE_CARRY_INT &&
      team1.carryi == 0 &&
      team2.carryi == 0)
  {
    team1.carry = BRIDGE_CARRY_NONE;
    team2.carry = BRIDGE_CARRY_NONE;
  }
}


void Teams::setRBN(const string& text)
{
  int seen = static_cast<int>(count(text.begin(), text.end(), ':'));
  if (seen == 1)
  {
    vector<string> v(2);
    v.clear();
    tokenize(text, v, ":");

    team1.set(v[0], BRIDGE_FORMAT_PBN); // Correct
    team2.set(v[1], BRIDGE_FORMAT_PBN);
  }
  else if (seen == 3)
  {
    vector<string> v(4);
    v.clear();
    tokenize(text, v, ":");

    Teams::setRBN4(v);
  }
  else if (seen == 4)
  {
    // Not really supposed to happen, but we apply some heuristics.
    vector<string> v(5), w(4);
    v.clear();
    tokenize(text, v, ":");

    w[2] = v[3];
    w[3] = v[4];

    const unsigned l0 = v[0].length();
    const unsigned l1 = v[1].length();
    const unsigned l2 = v[2].length();
    const char l0end = (l0 > 0 ? v[0].at(l0-1) : '$');
    const char l1end = (l1 > 0 ? v[1].at(l1-1) : '$');

    if (l0end == 'S' ||
        l1end == ')' ||
        (l1end == ' ' && v[2] != ")") ||
        (l0 > 1 && v[0].substr(l0-2) == "St"))
    {
      w[0] = v[0] + ":" + v[1];
      w[1] = v[2];
    }
    else if (l1end == 'S' ||
        l0end == ')' ||
        v[2] == ")" ||
        (l1 > 1 && v[1].substr(l1-2) == "St"))
    {
      w[0] = v[0];
      w[1] = v[1] + ":" + v[2];
    }
    else
      THROW("Malformed RBN team line: " + text);

    Teams::setRBN4(w);
  }
  else
    THROW("Malformed RBN team line: " + text);
}


void Teams::setTXT(const string& t)
{
  size_t pos;
  if ((pos = t.find(" vs. ", 0)) == string::npos || t.size() < pos+6)
    THROW("Cannot find two teams");

  string s1 = t.substr(0, pos);
  string s2 = t.substr(pos+5, string::npos);

  team1.set(s1, BRIDGE_FORMAT_TXT);
  team2.set(s2, BRIDGE_FORMAT_TXT);
}


void Teams::set(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return Teams::setRBN(text);
    
    case BRIDGE_FORMAT_TXT:
      return Teams::setTXT(text);
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Teams::setFirst(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      team1.set(text, BRIDGE_FORMAT_PBN);
      return;
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Teams::setSecond(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      team2.set(text, BRIDGE_FORMAT_PBN);
      return;
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Teams::swap()
{
  Team tmp = team1;
  team1 = team2;
  team2 = tmp;
}


bool Teams::hasCarry() const
{
  return (team1.hasCarry() && team2.hasCarry());
}


bool Teams::operator == (const Teams& teams2) const
{
  if (team1 != teams2.team1)
    DIFF("Different first teams");
  if (team2 != teams2.team2)
    DIFF("Different second teams");

  return true;
}


bool Teams::operator != (const Teams& teams2) const
{
  return ! (* this == teams2);
}


string Teams::strLIN(const bool swapFlag) const
{
  if (swapFlag)
    return 
      team2.str(BRIDGE_FORMAT_LIN) + "," + 
      team1.str(BRIDGE_FORMAT_LIN);
  else
    return 
      team1.str(BRIDGE_FORMAT_LIN) + "," + 
      team2.str(BRIDGE_FORMAT_LIN);
}


string Teams::strRBNCore(const bool swapFlag) const
{
  if (team1.name == "" && team2.name == "" &&
      team1.carry == BRIDGE_CARRY_NONE &&
      team2.carry == BRIDGE_CARRY_NONE)
    return "";

  stringstream ss;
  if (swapFlag)
  {
    ss << team2.name << ":" << team1.name;

    if (team1.carry != BRIDGE_CARRY_NONE ||
        team2.carry != BRIDGE_CARRY_NONE)
      ss << ":" << team2.strCarry() << ":" << team1.strCarry();
  }
  else
  {
    ss << team1.name << ":" << team2.name;

    if (team1.carry != BRIDGE_CARRY_NONE ||
        team2.carry != BRIDGE_CARRY_NONE)
      ss << ":" << team1.strCarry() << ":" << team2.strCarry();
  }

  return ss.str();
}

string Teams::strRBN(const bool swapFlag) const
{
  const string c = Teams::strRBNCore(swapFlag);
  if (c == "")
    return "";

  return "K " + c + "\n";
}


string Teams::strRBX(const bool swapFlag) const
{
  const string c = Teams::strRBNCore(swapFlag);
  if (c == "")
    return "";

  return "K{" + c + "}";
}


string Teams::strTXT(const bool swapFlag) const
{
  if (team1.name == "" && team2.name == "")
    return "";
  if (swapFlag)
    return 
      team2.str(BRIDGE_FORMAT_TXT) + " vs. " + 
      team1.str(BRIDGE_FORMAT_TXT);
  else
    return 
      team1.str(BRIDGE_FORMAT_TXT) + " vs. " + 
      team2.str(BRIDGE_FORMAT_TXT);
}


string Teams::strTXT(
  const int score1,
  const int score2,
  const bool swapFlag) const
{
  if (team1.name == "" && team2.name == "")
    return "\n";

  stringstream s1, s2;
  bool order12Flag = true;

  switch(team1.carry)
  {
    case BRIDGE_CARRY_NONE:
      if (score1 < score2)
        order12Flag = false;
      else if (swapFlag && score1 == score2)
        order12Flag = false;
      break;

    case BRIDGE_CARRY_INT:
      if (score1 + team1.carryi < score2 + team2.carryi)
        order12Flag = false;
      else if (swapFlag && (score1 + team1.carryi == score2 + team2.carryi))
        order12Flag = false;
      break;

    case BRIDGE_CARRY_FLOAT:
      if (score1 + team1.carryf < score2 + team2.carryf)
        order12Flag = false;
      break;

    default:
      THROW("Unknown carry");
  }

  if (order12Flag)
    return 
      team1.str(BRIDGE_FORMAT_TXT, score1) + "  " + 
      team2.str(BRIDGE_FORMAT_TXT, score2) + "\n";
  else
    return 
      team2.str(BRIDGE_FORMAT_TXT, score2) + "  " + 
      team1.str(BRIDGE_FORMAT_TXT, score1) + "\n";
}


string Teams::str(
  const Format format,
  const bool swapFlag) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
      return Teams::strLIN(swapFlag);

    case BRIDGE_FORMAT_RBN:
      return Teams::strRBN(swapFlag);
    
    case BRIDGE_FORMAT_RBX:
      return Teams::strRBX(swapFlag);
    
    case BRIDGE_FORMAT_TXT:
      return Teams::strTXT(swapFlag);
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Teams::str(
  const int score1,
  const int score2,
  const Format format,
  const bool swapFlag) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_TXT:
      return Teams::strTXT(score1, score2, swapFlag);
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Teams::strFirst(
  const Format format,
  const bool swapFlag) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      if (swapFlag)
        return team2.str(format, "HomeTeam");
      else
        return team1.str(format, "HomeTeam");

    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_PAR:
      return team1.name;

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Teams::strSecond(
  const Format format,
  const bool swapFlag) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      if (swapFlag)
        return team1.str(format, "VisitTeam");
      else
        return team2.str(format, "VisitTeam");

    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_PAR:
      return team2.name;

    default:
      THROW("Invalid format: " + STR(format));
  }
}

