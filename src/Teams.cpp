/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


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
}


bool Teams::SetPBN(
  const string& t1,
  const string& t2)
{
  UNUSED(t1);
  UNUSED(t2);
  // TODO
  return true;
}


bool Teams::SetRBN(const string& t)
{
  int seen = count(t.begin(), t.end(), ':');
  if (seen == 1)
  {
    vector<string> v(2);
    tokenize(t, v, ":");
    team1.name = v[0];
    team2.name = v[1];
  }
  else if (seen == 3)
  {
    vector<string> v(4);
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
  UNUSED(t);
  // TODO
  return true;
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
      abs(ta.carryf - tb.carryf) > 0.001)
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


string Teams::AsString(const formatType f) const
{
  UNUSED(f);
  // TODO
  return "";
}

