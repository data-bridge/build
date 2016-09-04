/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Team.h"
#include "portab.h"
#include "debug.h"

extern Debug debug;


Team::Team()
{
  Team::Reset();
}


Team::~Team()
{
}


void Team::Reset()
{
  team.name = "";
  team.carry = BRIDGE_CARRY_NONE;
  team.carryi = 0;
  team.carryf = 0.f;
}


bool Team::Set(
  const string list[],
  const unsigned teamNo,
  const formatType f)
{
  UNUSED(list);
  UNUSED(teamNo);
  UNUSED(f);
  // TODO
  return true;
}


bool Team::operator == (const Team& t2) const
{
  if (team.name != t2.team.name)
  {
    LOG("Different team name");
    return false;
  }
  else if (team.carry != t2.team.carry)
  {
    LOG("Different team carry type");
    return false;
  }
  else if (team.carry == BRIDGE_CARRY_INT &&
      team.carryi != t2.team.carryi)
  {
    LOG("Different team integer carry value");
    return false;
  }
  else if (team.carry == BRIDGE_CARRY_FLOAT &&
      abs(team.carryf - t2.team.carryf) > 0.001)
  {
    LOG("Different team integer float value");
    return false;
  }
  else
    return true;
}


bool Team::operator != (const Team& t2) const
{
  return ! (* this == t2);
}


string Team::AsString(
  const Team& team2,
  const formatType f) const
{
  UNUSED(team2);
  UNUSED(f);
  // TODO
  return "";
}

