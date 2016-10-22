/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <regex>
#include <sstream>
#include <iomanip>
#include "bconst.h"
#include "Teams.h"
#include "Bexcept.h"
#include "Bdiff.h"
#include "parse.h"
#include "portab.h"


Teams::Teams()
{
  Teams::Reset();
}


Teams::~Teams()
{
}


void Teams::Reset()
{
  team1.name = "";
  team1.carry = BRIDGE_CARRY_NONE;
  team1.carryi = 0;
  team1.carryf = 0.f;

  team2.name = "";
  team2.carry = BRIDGE_CARRY_NONE;
  team2.carryi = 0;
  team2.carryf = 0.f;
}


bool Teams::ExtractCarry(
  const string& t,
  teamType& tt) const
{
  int i;
  float f;

  if (StringToInt(t, i))
  {
    tt.carry = BRIDGE_CARRY_INT;
    tt.carryi = i;
  }
  else if (StringToFloat(t, f))
  {
    tt.carry = BRIDGE_CARRY_FLOAT;
    tt.carryf = f;
  }
  else
    THROW("Bad carry-over format");

  return true;
}


bool Teams::SetSinglePBN(
  const string& t,
  teamType& tt) const
{
  int seen = count(t.begin(), t.end(), ':');
  if (seen == 1)
  {
    vector<string> v(2);
    v.clear();
    tokenize(t, v, ":");
    tt.name = v[0];
    if (Teams::ExtractCarry(v[1], tt))
      return true;
    else
      THROW("Cannot extract carry");
  }
  else
  {
    tt.name = t;
    return true;
  }
}


bool Teams::SetSingleTXT(
  const string& t,
  teamType& tt) const
{
  try
  {
    regex re("^(.+)\\s+\\((.+)\\)");
    smatch match;
    if (regex_search(t, match, re) && match.size() >= 2)
    {
      if (! Teams::ExtractCarry(match.str(2), tt))
        THROW("Cannot extract carry");

      tt.name = match.str(1);
      return true;
    }
    else
      tt.name = t;
  }
  catch (regex_error& e)
  {
    UNUSED(e);
    tt.name = t;
  }
  return true;
}


bool Teams::SetPBN(
  const string& t1,
  const string& t2)
{
  if (Teams::SetSinglePBN(t1, team1) && Teams::SetSinglePBN(t2, team2))
    return true;

  Teams::Reset();
  return false;
}


bool Teams::SetRBN(const string& t)
{
  int seen = count(t.begin(), t.end(), ':');
  if (seen == 1)
  {
    vector<string> v(2);
    v.clear();
    tokenize(t, v, ":");
    team1.name = v[0];
    team2.name = v[1];
  }
  else if (seen == 3)
  {
    vector<string> v(4);
    v.clear();
    tokenize(t, v, ":");

    teamType tt1, tt2;
    if (Teams::ExtractCarry(v[2], tt1) && Teams::ExtractCarry(v[3], tt2))
    {
      team1 = tt1;
      team1.name = v[0];

      team2 = tt2;
      team2.name = v[1];

      if (team1.carry == BRIDGE_CARRY_INT && 
          team2.carry == BRIDGE_CARRY_INT &&
          team1.carryi == 0 &&
          team2.carryi == 0)
      {
        team1.carry = BRIDGE_CARRY_NONE;
        team2.carry = BRIDGE_CARRY_NONE;
      }
      return true;
    }
    else
      return false;
  }
  else
    THROW("Malformed RBN team line");

  return true;
}


bool Teams::SetTXT(const string& t)
{
  size_t pos;
  if ((pos = t.find(" vs. ", 0)) == string::npos || t.size() < pos+6)
    THROW("Cannot find two teams");

  string s1 = t.substr(0, pos);
  string s2 = t.substr(pos+5, string::npos);

  if (Teams::SetSingleTXT(s1, team1) && Teams::SetSingleTXT(s2, team2))
    return true;

  Teams::Reset();
  return false;
}


bool Teams::Set(
  const string& s,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return Teams::SetRBN(s);
    
    case BRIDGE_FORMAT_PBN:
      THROW("Set not implemented for PBN");
    
    
    case BRIDGE_FORMAT_TXT:
      return Teams::SetTXT(s);
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


bool Teams::SetFirst(
  const string& s,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_TXT:
      THROW("SetFirst not implemented for this format");
    
    case BRIDGE_FORMAT_PBN:
      return Teams::SetSinglePBN(s, team1);
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


bool Teams::SetSecond(
  const string& s,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_TXT:
      THROW("SetFirst not implemented for this format");
    
    case BRIDGE_FORMAT_PBN:
      return Teams::SetSinglePBN(s, team2);
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


bool Teams::TeamIsEqual(
  const teamType& ta,
  const teamType& tb) const
{
  if (ta.name != tb.name)
    DIFF("Different team name");
  else if (ta.carry != tb.carry)
    DIFF("Different team carry type");
  else if (ta.carry == BRIDGE_CARRY_INT && ta.carryi != tb.carryi)
    DIFF("Different team integer carry value");
  else if (ta.carry == BRIDGE_CARRY_FLOAT &&
      (ta.carryf - tb.carryf > 0.001f || ta.carryf - tb.carryf < -0.001f))
    DIFF("Different team integer float value");

  return true;
}


bool Teams::CarryExists() const
{
  // return (team1.carry || team2.carry);
  if (team1.carry == BRIDGE_CARRY_NONE || 
      team2.carry == BRIDGE_CARRY_NONE)
    return false;

  // Odd (Pavlicek).
  if (team1.carry == BRIDGE_CARRY_INT && team1.carryi <= 0)
    return false;
  if (team2.carry == BRIDGE_CARRY_INT && team2.carryi <= 0)
    return false;
  if (team1.carry == BRIDGE_CARRY_FLOAT && team1.carryf <= 0.f)
    return false;
  if (team2.carry == BRIDGE_CARRY_FLOAT && team2.carryf <= 0.f)
    return false;

  return true;
}


bool Teams::operator == (const Teams& t2) const
{
  if (! Teams::TeamIsEqual(team1, t2.team1))
    return false;
  else if (! Teams::TeamIsEqual(team2, t2.team2))
    return false;
  else
    return true;
}


bool Teams::operator != (const Teams& t2) const
{
  return ! (* this == t2);
}


string Teams::SingleAsLIN(const teamType& tt) const
{
  if (tt.name == "" && 
      (tt.carry == BRIDGE_CARRY_NONE || 
        (tt.carry == BRIDGE_CARRY_INT && tt.carryi == 0)))
    return ",";
  else
    return tt.name + "," + Teams::CarryAsString(tt, true);
}


string Teams::SingleAsPBN(const teamType& tt) const
{
  stringstream s;
  s << tt.name;
  if (tt.carry != BRIDGE_CARRY_NONE)
    s << ":" << Teams::CarryAsString(tt);
  return s.str();
}


string Teams::SingleAsTXT(const teamType& tt) const
{
  stringstream s;
  s << tt.name;
  if (tt.carry != BRIDGE_CARRY_NONE)
    s << " (" << Teams::CarryAsString(tt) << ")";
  return s.str();
}


string Teams::CarryAsString(
  const teamType& tt,
  const bool forceFlag) const
{
  stringstream s;

  switch(tt.carry)
  {
    case BRIDGE_CARRY_NONE:
      if (forceFlag)
        return "0";
      else
        return "";

    case BRIDGE_CARRY_INT:
      s << tt.carryi;
      return s.str();

    case BRIDGE_CARRY_FLOAT:
      s << fixed << setprecision(2) << tt.carryf;
      return s.str();

    default:
      THROW("Unknown carry");
  }
}


string Teams::AsLIN(const bool swapFlag) const
{
  if (swapFlag)
    return Teams::SingleAsLIN(team2) + "," + Teams::SingleAsLIN(team1);
  else
    return Teams::SingleAsLIN(team1) + "," + Teams::SingleAsLIN(team2);
}


string Teams::AsRBNCore(const bool swapFlag) const
{
  if (team1.name == "" && team2.name == "" &&
      team1.carry == BRIDGE_CARRY_NONE &&
      team2.carry == BRIDGE_CARRY_NONE)
    return "";

  stringstream s;
  if (swapFlag)
  {
    s << team2.name << ":" << team1.name;
    if (team1.carry == BRIDGE_CARRY_NONE &&
        team2.carry == BRIDGE_CARRY_NONE)
      return s.str();

    s << ":" << Teams::CarryAsString(team2) << 
        ":" << Teams::CarryAsString(team1);
  }
  else
  {
    s << team1.name << ":" << team2.name;
    if (team1.carry == BRIDGE_CARRY_NONE &&
        team2.carry == BRIDGE_CARRY_NONE)
      return s.str();

    s << ":" << Teams::CarryAsString(team1) << 
        ":" << Teams::CarryAsString(team2);
  }

  return s.str();
}

string Teams::AsRBN(const bool swapFlag) const
{
  const string c = Teams::AsRBNCore(swapFlag);
  if (c == "")
    return "";

  return "K " + c + "\n";
}


string Teams::AsRBX(const bool swapFlag) const
{
  const string c = Teams::AsRBNCore(swapFlag);
  if (c == "")
    return "";

  return "K{" + c + "}";
}


string Teams::AsTXT() const
{
  if (team1.name == "" && team2.name == "")
    return "";
  else
    return Teams::SingleAsTXT(team1) + " vs. " + Teams::SingleAsTXT(team2);
}


string Teams::AsTXT(
  const int score1,
  const int score2) const
{
  if (team1.name == "" && team2.name == "")
    return "\n";

  stringstream s1, s2;
  bool order12Flag = true;

  s1 << team1.name << " ";
  s2 << team2.name << " ";

  switch(team1.carry)
  {
    case BRIDGE_CARRY_NONE:
      if (score1 < score2)
        order12Flag = false;
      s1 << score1;
      s2 << score2;
      break;

    case BRIDGE_CARRY_INT:
      if (score1 + team1.carryi < score2 + team2.carryi)
        order12Flag = false;
      s1 << score1 + team1.carryi;
      s2 << score2 + team2.carryi;
      break;
          team1.name + " " + STR(score1 + team1.carryi) + "\n";

    case BRIDGE_CARRY_FLOAT:
      if (score1 + team1.carryf < score2 + team2.carryf)
        order12Flag = false;
      s1 << fixed << setprecision(2) << score1 + team1.carryf;
      s2 << fixed << setprecision(2) << score1 + team2.carryf;

    default:
      THROW("Unknown carry");
  }

  if (order12Flag)
    return s1.str() + "  " + s2.str() + "\n";
  else
    return s2.str() + "  " + s1.str() + "\n";
}


string Teams::AsString(
  const formatType f,
  const bool swapFlag) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Teams::AsLIN(swapFlag);

    case BRIDGE_FORMAT_PBN:
      THROW("AsString not implemented for PBN");
    
    case BRIDGE_FORMAT_RBN:
      return Teams::AsRBN(swapFlag);
    
    case BRIDGE_FORMAT_RBX:
      return Teams::AsRBX(swapFlag);
    
    case BRIDGE_FORMAT_TXT:
      return Teams::AsTXT();
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


string Teams::AsString(
  const formatType f,
  const int score1,
  const int score2) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_EML:
      THROW("AsString with score not implemented for this format");

    case BRIDGE_FORMAT_TXT:
      return Teams::AsTXT(score1, score2);
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


string Teams::FirstAsString(
  const formatType f,
  const bool swapFlag) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_RBN:
      THROW("FirstAsString not implemented");

    case BRIDGE_FORMAT_PBN:
      if (team1.name == "" && team1.carry == BRIDGE_CARRY_NONE)
        return "";
      if (swapFlag)
        return "[HomeTeam \"" + Teams::SingleAsPBN(team2) + "\"]\n";
      else
        return "[HomeTeam \"" + Teams::SingleAsPBN(team1) + "\"]\n";

    case BRIDGE_FORMAT_TXT:
      if (team1.name == "" && team1.carry == BRIDGE_CARRY_NONE)
        return "";
      return team1.name;

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Teams::SecondAsString(
  const formatType f,
  const bool swapFlag) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_RBN:
      THROW("FirstAsString not implemented");

    case BRIDGE_FORMAT_PBN:
      if (team2.name == "" && team2.carry == BRIDGE_CARRY_NONE)
        return "";
      if (swapFlag)
        return "[VisitTeam \"" + Teams::SingleAsPBN(team1) + "\"]\n";
      else
        return "[VisitTeam \"" + Teams::SingleAsPBN(team2) + "\"]\n";

    case BRIDGE_FORMAT_TXT:
      if (team2.name == "" && team2.carry == BRIDGE_CARRY_NONE)
        return "";
      return team2.name;

    default:
      THROW("Invalid format " + STR(f));
  }
}

