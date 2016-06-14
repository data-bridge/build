/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Auction.h"
#include "portab.h"


Auction::Auction()
{
  Auction::Reset();
}


Auction::~Auction()
{
}


void Auction::Reset()
{
  setFlag = false;
}


bool Auction::IsOver() const
{
  return setFlag;
}


bool Auction::IsPassedOut() const
{
  // TODO
  return true;
}


bool Auction::AddCall(
  const string call,
  const string alert)
{
  // TODO
  UNUSED(call);
  UNUSED(alert);
  return true;
}


bool Auction::AddPasses()
{
  // TODO
  return true;
}


bool Auction::UndoLastCall()
{
  // TODO
  return true;
}


bool Auction::AddAuction(
  const string s,
  const formatType f)
{
  // TODO
  UNUSED(s);
  UNUSED(f);
  return true;
}


bool Auction::operator == (const Auction& a2)
{
  // TODO
  UNUSED(a2);
  return true;
}


bool Auction::operator != (const Auction& a2)
{
  // TODO
  UNUSED(a2);
  return false;
}


string Auction::AsString(const formatType f) const
{
  // TODO
  UNUSED(f);
  return "";
}


string Auction::TableAsPBN() const
{
  // TODO
  return "";
}


string Auction::ResultAsString(const formatType f) const
{
  // TODO
  UNUSED(f);
  return "";
}

