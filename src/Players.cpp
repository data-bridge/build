/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Players.h"


Players::Players()
{
  Players::Reset();
}


Players::~Players()
{
}


void Players::Reset()
{
  setFlag = false;
}


bool Players::IsSet() const
{
  return setFlag;
}


bool Players::Set(const int v2)
{
  if (setFlag)
    return (value == v2);
  else
  {
    setFlag = true;
    value = v2;
    return true;
  }
}


string Players::Get(const playerType p) const
{
  // TODO
  return "";
}


bool Players::operator == (const Players& b2)
{
  // TODO
  return true;
}


bool Players::operator != (const Players& b2)
{
  // TODO
  return false;
}

