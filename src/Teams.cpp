/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <regex>
#include "Teams.h"
#include "portab.h"
#include "parse.h"
#include "Debug.h"

extern Debug debug;


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
  {
    LOG("Bad carry-over format");
    return false;
  }
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
    {
      LOG("Cannot extract carry");
      return false;
    }
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
      {
        LOG("Cannot extract carry");
        return false;
      }
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
      return true;
    }
    else
      return false;
  }
  else
  {
    LOG("Malformed RBN team line");
    return false;
  }
  return true;
}


bool Teams::SetTXT(const string& t)
{
  size_t pos;
  if ((pos = t.find(" vs. ", 0)) == string::npos || t.size() < pos+6)
  {
    LOG("Cannot find two teams");
    return false;
  }

  string s1 = t.substr(0, pos);
  string s2 = t.substr(pos+5, string::npos);

  if (Teams::SetSingleTXT(s1, team1) && Teams::SetSingleTXT(s2, team2))
    return true;

  Teams::Reset();
  return false;
}


bool Teams::Set(
  const string list[],
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_RBN:
      return Teams::SetRBN(list[0]);
    
    case BRIDGE_FORMAT_PBN:
      return Teams::SetPBN(list[0], list[1]);
    
    
    case BRIDGE_FORMAT_TXT:
      return Teams::SetTXT(list[0]);
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}


bool Teams::TeamIsEqual(
  const teamType& ta,
  const teamType& tb) const
{
  if (ta.name != tb.name)
  {
    LOG("Different team name");
    return false;
  }
  else if (ta.carry != tb.carry)
  {
    LOG("Different team carry type");
    return false;
  }
  else if (ta.carry == BRIDGE_CARRY_INT &&
      ta.carryi != tb.carryi)
  {
    LOG("Different team integer carry value");
    return false;
  }
  else if (ta.carry == BRIDGE_CARRY_FLOAT &&
      abs(ta.carryf - tb.carryf) > 0.001f)
  {
    LOG("Different team integer float value");
    return false;
  }
  else
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
  return tt.name + "," + CarryAsString(tt);
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


string Teams::CarryAsString(const teamType& tt) const
{
  stringstream s;

  switch(tt.carry)
  {
    case BRIDGE_CARRY_NONE:
      return "";

    case BRIDGE_CARRY_INT:
      s << tt.carryi;
      return s.str();

    case BRIDGE_CARRY_FLOAT:
      s << fixed << setprecision(2) << tt.carryf;
      return s.str();

    default:
      LOG("Unknown carry");
      return "";
  }
}


string Teams::AsLIN() const
{
  return Teams::SingleAsLIN(team1) + "," + Teams::SingleAsLIN(team2);
}


string Teams::AsPBN() const
{
  if (team1.name == "" && team2.name == "" &&
      team1.carry == BRIDGE_CARRY_NONE &&
      team2.carry == BRIDGE_CARRY_NONE)
    return "";

  return "[OpenTeam \"" + Teams::SingleAsPBN(team1) + "\"]\n" +
      "[VisitTeam \"" + Teams::SingleAsPBN(team2) + "\"]\n";
}


string Teams::AsRBN() const
{
  if (team1.name == "" && team2.name == "" &&
      team1.carry == BRIDGE_CARRY_NONE &&
      team2.carry == BRIDGE_CARRY_NONE)
    return "";

  stringstream s;
  s << "K " << team1.name << ":" << team2.name;
  if (team1.carry == BRIDGE_CARRY_NONE &&
      team2.carry == BRIDGE_CARRY_NONE)
    return s.str() + "\n";

  s << ":" << Teams::CarryAsString(team1) << 
      ":" << Teams::CarryAsString(team2) << "\n";

  return s.str();
}


string Teams::AsTXT() const
{
  return Teams::SingleAsTXT(team1) + " vs. " + Teams::SingleAsTXT(team2);
}


string Teams::AsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Teams::AsLIN();

    case BRIDGE_FORMAT_PBN:
      return Teams::AsPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Teams::AsRBN();
    
    case BRIDGE_FORMAT_TXT:
      return Teams::AsTXT();
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}

