/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <regex>

#include "Team.h"

#include "../parse.h"

#include "../handling/Bexcept.h"
#include "../handling/Bdiff.h"


Team::Team()
{
  Team::reset();
}


void Team::reset()
{
  name = "";
  carry = BRIDGE_CARRY_NONE;
  carryi = 0;
  carryf = 0.f;
}


void Team::extractCarry(const string& text)
{
  int i;
  float f;

  if (str2int(text, i))
  {
    carry = BRIDGE_CARRY_INT;
    carryi = i;
  }
  else if (str2float(text, f))
  {
    carry = BRIDGE_CARRY_FLOAT;
    carryf = f;
  }
  else
    THROW("Bad carry-over format");
}


void Team::setPBN(const string& text)
{
  int seen = static_cast<int>(count(text.begin(), text.end(), ':'));
  if (seen > 1)
    THROW("Too many colons");
  if (seen == 0)
  {
    name = text;
    return;
  }

  vector<string> v(2);
  v.clear();
  tokenize(text, v, ":");

  name = v[0];
  Team::extractCarry(v[1]);
}


void Team::setTXT(const string& text)
{
  regex re("^(.+)\\s+\\((.+)\\)");
  smatch match;
  if (regex_search(text, match, re) && match.size() >= 2)
  {
    name = match.str(1);
    Team::extractCarry(match.str(2));
  }
  else
    name = text;
}


void Team::set(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      Team::setPBN(text);
      return;

    case BRIDGE_FORMAT_TXT:
      Team::setTXT(text);
      return;
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
}


void Team::setPair(
  const string& text,
  const string& value)
{
  name = text;
  Team::extractCarry(value);
}


bool Team::operator == (const Team& team2) const
{
  if (name != team2.name)
    DIFF("Different team name: " + name + " / " + team2.name);

  if (carry != team2.carry)
    DIFF("Different team carry type");

  if (carry == BRIDGE_CARRY_INT && carryi != team2.carryi)
    DIFF("Different team integer carry value");
  else if (carry == BRIDGE_CARRY_FLOAT &&
      (carryf - team2.carryf > 0.001f || carryf - team2.carryf < -0.001f))
    DIFF("Different team float carry value");

  return true;
}


bool Team::operator != (const Team& team2) const
{
  return ! (* this == team2);
}


bool Team::hasCarry() const
{
  if (carry == BRIDGE_CARRY_NONE)
    return false;

  // Odd (Pavlicek).
  if (carry == BRIDGE_CARRY_INT && carryi <= 0)
    return false;
  else if (carry == BRIDGE_CARRY_FLOAT && carryf <= 0.f)
    return false;

  return true;
}


unsigned Team::getCarry() const
{
  if (carry == BRIDGE_CARRY_INT && carryi >= 0)
    return static_cast<unsigned>(carryi);
  else
    return 0u;
}


string Team::strCarry(const bool forceFlag) const
{
  stringstream ss;

  switch(carry)
  {
    case BRIDGE_CARRY_NONE:
      return (forceFlag ? "0" : "");

    case BRIDGE_CARRY_INT:
      ss << carryi;
      return ss.str();

    case BRIDGE_CARRY_FLOAT:
      ss << fixed << setprecision(2) << carryf;
      return ss.str();

    default:
      THROW("Unknown carry");
  }
}


string Team::strLIN() const
{
  if (name == "" && 
      (carry == BRIDGE_CARRY_NONE || 
      (carry == BRIDGE_CARRY_INT && carryi == 0)))
    return ",";
  else
    return name + "," + Team::strCarry(true);
}


string Team::strPBN(const string& label) const
{
  if (name == "" && carry == BRIDGE_CARRY_NONE)
    return "";

  stringstream ss;
  ss << "[" + label + " \"" + name;
  if (carry != BRIDGE_CARRY_NONE)
    ss << ":" << Team::strCarry();
  return ss.str() + "\"]\n";
}


string Team::strTXT() const
{
  stringstream ss;
  ss << name;
  if (carry != BRIDGE_CARRY_NONE)
    ss << " (" << Team::strCarry() << ")";
  return ss.str();
}


string Team::strTXT(const int score) const
{
  if (name == "")
    return "\n";

  stringstream ss;
  ss << name << " ";

  switch(carry)
  {
    case BRIDGE_CARRY_NONE:
      ss << score;
      break;

    case BRIDGE_CARRY_INT:
      ss << score + carryi;
      break;

    case BRIDGE_CARRY_FLOAT:
      ss << fixed << setprecision(2) << 
        static_cast<float>(score) + carryf;
      break;

    default:
      THROW("Unknown carry");
  }

  return ss.str();
}


string Team::str(
  const Format format,
  const string& label) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return Team::strLIN();

    case BRIDGE_FORMAT_PBN:
      return Team::strPBN(label);
    
    case BRIDGE_FORMAT_TXT:
      return Team::strTXT();
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
}


string Team::str(
  const Format format,
  const int score) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_TXT:
      return Team::strTXT(score);
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
}

