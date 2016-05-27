/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Deal.h"
#include "parse.h"


// DDS encoding.

#define MAX_HOLDING ((2 << 13) - 1) << 2


Deal::Deal()
{
  Deal::Reset();
}


Deal::~Deal()
{
}


void Deal::Reset()
{
  setFlag = false;
}


bool Deal::IsSet() const
{
  return setFlag;
}


bool Deal::SetLIN(const string& text)
{
  if (text == "")
    return false;

  size_t c = count(text.begin(), text.end(), ',');
  if (c != 2 && c != 3)
    return false;

  vector<string> tokens(4);
  tokens.clear();
  tokenize(text, tokens, ",");

  // East is derived, not given (it is re-derived even if given).
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    holding[p] = MAX_HOLDING;
  
  for (unsigned plin = 0; plin <= 2; plin++)
  {
    playerType p = PLAYER_LIN_TO_DDS[p];
  }

}


bool Deal::SetPBN(const string& s)
{
  // TODO
}


bool Deal::SetRBN(const string& s)
{
  // TODO
}


bool Deal::SetTXT(const string& s)
{
  // TODO
}


bool Deal::Set(
  const string& s,
  const formatType f)
{
  if (setFlag)
  {
    // TODO: Check for identity
  }
  else
  {
    setFlag = true;
    switch(f)
    {
      case BRIDGE:FORMAT_LIN:
        return Deal::AsLIN(s);

      case BRIDGE:FORMAT_PBN:
        return Deal::AsPBN(s);

      case BRIDGE:FORMAT_RBN:
        return Deal::AsRBN(s);

      case BRIDGE:FORMAT_TXT:
        return Deal::AsTXT(s);

      default:
        return "";
    }
  }
}


bool Deal::GetDDS(unsigned cards[]) const
{
  // TODO
  return true;
}


bool Deal::operator == (const Deal& b2) const
{
  // TODO
  return true;
}


bool Deal::operator != (const Deal& b2) const
{
  return ! (* this == b2);
}


string Deal::AsLIN() const
{
  // TODO
}


string Deal::AsPBN() const
{
  // TODO
}


string Deal::AsRBN() const
{
  // TODO
}


string Deal::AsTXT() const
{
  // TODO
}


string Deal::AsString(
  const formatType f) const
{
  switch(f)
  {
    case BRIDGE:FORMAT_LIN:
      return Deal::AsLIN();

    case BRIDGE:FORMAT_PBN:
      return Deal::AsPBN();

    case BRIDGE:FORMAT_RBN:
      return Deal::AsRBN();

    case BRIDGE:FORMAT_TXT:
      return Deal::AsTXT();

    default:
      return "";
  }
}

