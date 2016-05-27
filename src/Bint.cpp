/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Bint.h"


Bint::Bint()
{
  Bint::Reset();
}


Bint::~Bint()
{
}


void Bint::Reset()
{
  setFlag = false;
}


bool Bint::IsSet() const
{
  return setFlag;
}


bool Bint::Set(const int v2)
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


int Bint::Get() const
{
  // Caller must know whether or not the value was set.
  return value;
}


bool Bint::operator == (const Bint& b2)
{
  return (value == b2.value);
}


bool Bint::operator == (const int v2)
{
  return (value == v2);
}


bool Bint::operator != (const Bint& b2)
{
  return (value != b2.value);
}


bool Bint::operator != (const int v2)
{
  return (value != v2);
}
