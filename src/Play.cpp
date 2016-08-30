/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Play.h"
#include "portab.h"
#include "Debug.h"

extern Debug debug;


Play::Play()
{
  Play::Reset();
}


Play::~Play()
{
}


void Play::Reset()
{
  setFlag = false;
}


bool Play::SetContract(const Contract& contract)
{
  UNUSED(contract);
  return true;
}


bool Play::SetDeclAndDenom(
  const playerType decl,
  const denomType denom)
{
  UNUSED(decl);
  UNUSED(denom);
  return true;
}


bool Play::AddPlay(const string& str)
{
  UNUSED(str);
  return true;
}


bool Play::AddPlay(
  const string& str,
  const Deal& deal)
{
  UNUSED(str);
  UNUSED(deal);
  return true;
}


bool Play::AddPlays(const string& str)
{
  UNUSED(str);
  return true;
}


bool Play::AddPlays(
  const string& str,
  const Deal& deal)
{
  UNUSED(str);
  UNUSED(deal);
  return true;
}


bool Play::AddPlaysAbsolute(const string& str)
{
  UNUSED(str);
  return true;
}


bool Play::AddPlaysAbsolute(
  const string& str,
  const Deal& deal)
{
  UNUSED(str);
  UNUSED(deal);
  return true;
}


bool Play::UndoPlay()
{
  return true;
}


bool Play::PlayIsOver()
{
  return true;
}


bool Play::Claim(const unsigned tricks)
{
  UNUSED(tricks);
  return true;
}

bool Play::ClaimIsMade() const
{
  return true;
}


bool Play::operator == (const Play& p2)
{
  UNUSED(p2);
  return true;
}


bool Play::operator != (const Play& p2)
{
  UNUSED(p2);
  return false;
}


string Play::AsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return "";

    case BRIDGE_FORMAT_PBN:
      return "";

    case BRIDGE_FORMAT_RBN:
      return "";

    case BRIDGE_FORMAT_TXT:
      return "";

    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}


string Play::ClaimAsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return "";

    case BRIDGE_FORMAT_PBN:
      return "";

    case BRIDGE_FORMAT_RBN:
      return "";

    case BRIDGE_FORMAT_TXT:
      return "";

    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}

