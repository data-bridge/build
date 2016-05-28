/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "bconst.h"
#include "Deal.h"
#include "Debug.h"
#include "parse.h"

#include <map>

extern Debug debug;


// DDS encoding, but without the two bottom 00 bits: ((2 << 13) - 1)

#define MAX_HOLDING 0x1fff

const string CARDS[BRIDGE_TRICKS] =
{
  "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A"
};

bool setDealTables = false;
string HOLDING_TO_SUIT[MAX_HOLDING+1];
map<string, unsigned> SUIT_TO_HOLDING;
map<string, playerType> PLAYER_TO_DDS;


Deal::Deal()
{
  Deal::Reset();
  if (! setDealTables)
  {
    setDealTables = true;
    Deal::SetTables();
  }
}


Deal::~Deal()
{
}


void Deal::Reset()
{
  setFlag = false;
}


void Deal::SetTables()
{
  for (unsigned h = 0; h <= MAX_HOLDING; h++)
  {
    string suit("");
    for (int bit = 12; bit >= 0; bit--)
    {
      if (h & (1 << bit))
        suit += CARDS[bit];
    }
    HOLDING_TO_SUIT[h] = suit;
    SUIT_TO_HOLDING[suit] = h;
    reverse(suit.begin(), suit.end());
    SUIT_TO_HOLDING[suit] = h;
  }

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    PLAYER_TO_DDS[PLAYER_NAMES_SHORT[p]] = static_cast<playerType>(p);
}


bool Deal::IsSet() const
{
  return setFlag;
}


bool Deal::StringToPlayer(
  const string& s,
  unsigned& p) const
{
  map<string, playerType>::iterator it = PLAYER_TO_DDS.find(s);
  if (it == PLAYER_TO_DDS.end())
  {
    LOG("No such player: '" + s + "'");
    return false;
  }

  p = static_cast<unsigned>(PLAYER_TO_DDS[s]);
  return true;
}


bool Deal::SetCards(
  const string suit,
  unsigned& res) const
{
  map<string, unsigned>::iterator it = SUIT_TO_HOLDING.find(suit);
  if (it == SUIT_TO_HOLDING.end())
  {
    LOG("No such suit: '" + suit + "'");
    return false;
  }
  res = SUIT_TO_HOLDING[suit];
  return true;
}


bool Deal::SetHand(
  const string& hand,
  const string& delimiters,
  const unsigned offset,
  unsigned pholding[])
{
  unsigned seen = 0;
  for (unsigned i = 0; i < delimiters.length(); i++)
    seen += count(hand.begin(), hand.end(), delimiters.at(i));

  if (seen != 3+offset)
  {
cout << "seen " << seen << " offset " << offset << "\n";
cout << "delimiters " << delimiters << ", " << hand << endl;
    LOG("Not the right number of delimiters");
    return false;
  }

  vector<string> suits(BRIDGE_SUITS+1);
  suits.clear();
  tokenize(hand, suits, delimiters);

  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    if (! Deal::SetCards(suits[s+offset], pholding[s]))
    {
cout << "  s " << s << " " << suits[s+offset] << endl;
      return false;
    }
  }
  return true;
}


bool Deal::SetHands()
{
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    unsigned sum = 0, xsum = 0;
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      strcpy(cards[p][s], HOLDING_TO_SUIT[holding[p][s]].c_str());
      sum += holding[p][s];
      xsum ^= holding[p][s];

cout << "cards[" << p << "][" << s << "]: " <<
  cards[p][s] << "\n";
    }
    if (sum != MAX_HOLDING || xsum != MAX_HOLDING)
    {
      LOG("Cards do not add up");
      return false;
    }
  }
  return true;
}


bool Deal::SetLIN(const string& text)
{
  size_t c = countDelimiters(text, ",");
  if (c != 2 && c != 3)
  {
    LOG("Not 2 or 3 periods");
    return false;
  }

  vector<string> tokens(BRIDGE_PLAYERS);
  tokens.clear();
  tokenize(text, tokens, ",");

  // Last is derived, not given (it is re-derived even if given).
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    holding[PLAYER_LIN_TO_DDS[3]][s] = MAX_HOLDING;
  
  for (unsigned plin = 0; plin <= 2; plin++)
  {
    playerType p = PLAYER_LIN_TO_DDS[plin];
    c = countDelimiters(tokens[p], "SHDC");
    if (c != 4)
    {
cout << "token " << tokens[p] << " " << c << endl;
      LOG("Not 4 suits");
      return false;
    }

cout << "SetLin plin " << plin << endl;
    if (! Deal::SetHand(tokens[p], "SHDC", 1, holding[p]))
    {
cout << "Set token " << tokens[p] << " " << c << endl;
      return false;
    }

    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      holding[PLAYER_LIN_TO_DDS[3]][s] ^= holding[p][s];
  }

  return Deal::SetHands();
}


bool Deal::SetPBN(const string& s)
{
  unsigned c = countDelimiters(s, ":| ");
  if (c != 3)
  {
    LOG("Not 3 delimiters in '" + s + "'");
    return false;
  }

  vector<string> tokens(BRIDGE_PLAYERS+1);
  tokens.clear();
  tokenize(s, tokens, ":");

  unsigned first;
  if (! Deal::StringToPlayer(tokens[0], first))
    return false;

  for (unsigned pno = 0; pno < BRIDGE_PLAYERS; pno++)
  {
    unsigned p = (first + pno) % 4;

    if (countDelimiters(tokens[pno+1], ".") != 3)
    {
      LOG("Not 3 periods");
      return false;
    }

    if (! Deal::SetHand(tokens[pno+1], ".", 0, holding[p]))
      return false;
  }

  return Deal::SetHands();
}


bool Deal::SetRBN(const string& s)
{
  size_t c = countDelimiters(s, ":");
  if (c != 3 && c != 4)
  {
    LOG("Not 3 or 4 colons");
    return false;
  }

  vector<string> tokens(BRIDGE_PLAYERS+1);
  tokens.clear();
  tokenize(s, tokens, ":");

  // Last is derived, not given (it is re-derived even if given).
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    holding[PLAYER_RBN_TO_DDS[3]][s] = MAX_HOLDING;
  
  unsigned first;
  if (! Deal::StringToPlayer(tokens[0], first))
    return false;

  for (unsigned pno = 0; pno <= 2; pno++)
  {
    unsigned p = (first + pno) % 4;

    if (countDelimiters(tokens[pno+1], ".") != 3)
    {
      LOG("Not 3 periods");
      return false;
    }

    if (! Deal::SetHand(tokens[pno+1], ".", 0, holding[p]))
      return false;

    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      holding[PLAYER_RBN_TO_DDS[3]][s] ^= holding[p][s];
  }

  return Deal::SetHands();
}


bool Deal::SetTXT(const string cardsArg[][BRIDGE_SUITS])
{
  for (unsigned plin = 0; plin < BRIDGE_PLAYERS; plin++)
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      if (! Deal::SetCards(cardsArg[plin][s], holding[plin][s]))
        return false;

  return Deal::SetHands();
}


bool Deal::Set(
  const string& s,
  const formatType f)
{
  if (s == "")
  {
    LOG("Empty text");
    return false;
  }

  if (setFlag)
  {
    LOG("Already set");
    return false;
  }

  setFlag = true;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Deal::SetLIN(s);

    case BRIDGE_FORMAT_PBN:
      return Deal::SetPBN(s);

    case BRIDGE_FORMAT_RBN:
      return Deal::SetRBN(s);

    default:
      LOG("Unacceptable format");
      return false;
  }
}


bool Deal::GetDDS(unsigned holdingArg[][BRIDGE_SUITS]) const
{
  if (! setFlag)
  {
    LOG("Holding not set");
    return false;
  }

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      holdingArg[p][s] = holding[p][s] << 2;
  
  return true;
}


bool Deal::operator == (const Deal& b2) const
{
  if (setFlag != b2.setFlag)
  {
    LOG("Different setFlags");
    return false;
  }

  if (! setFlag && ! b2.setFlag)
    return true;
  
  for (unsigned plin = 0; plin < BRIDGE_PLAYERS; plin++)
  {
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    {
      if (holding[plin][s] != b2.holding[plin][s])
      {
        LOG("Different holdings");
	return false;
      }
    }
  }
  return true;
}


bool Deal::operator != (const Deal& b2) const
{
  return ! (* this == b2);
}


string Deal::AsLIN(const playerType start) const
{
  string s = "st||md|" + PLAYER_NAMES_SHORT[start];
cout << "s " << s << endl;
cout << "'" << cards[0][BRIDGE_SPADES] << "'" << endl;
cout << "'" << RevStr(cards[0][BRIDGE_SPADES]) << "'" << endl;
  
  // Players are always in same order.
  for (unsigned p = 0; p <= 2; p++)
  {
    s += "S" + RevStr(cards[p][BRIDGE_SPADES]) +
         "H" + RevStr(cards[p][BRIDGE_HEARTS]) +
         "D" + RevStr(cards[p][BRIDGE_DIAMONDS]) +
         "C" + RevStr(cards[p][BRIDGE_CLUBS]);
    if (p < 2)
      s += ",";
cout << "p " << p << ": s " << s << endl;
  }
  return s + "|rh||";
}


string Deal::AsPBN(const playerType start) const
{
  stringstream s;
  s << "[Deal \"" << PLAYER_NAMES_SHORT[start] << ":";

  // Players start from dealer.
  for (unsigned pno = 0; pno < BRIDGE_PLAYERS; pno++)
  {
    unsigned p = (start + pno) % 4;
    s << cards[p][BRIDGE_SPADES] << "." <<
         cards[p][BRIDGE_HEARTS] << "." <<
         cards[p][BRIDGE_DIAMONDS] << "." <<
         cards[p][BRIDGE_CLUBS];
    if (p < 3)
      s << " ";
  }
  return s.str() + "\"]\n";
}


string Deal::AsRBN(const playerType start) const
{
  stringstream s;
  s << "H " << PLAYER_NAMES_SHORT[start] << ":";

  // Players start from here.
  for (unsigned pno = 0; pno <= 2; pno++)
  {
    unsigned p = (start + pno) % 4;
    s << cards[p][BRIDGE_SPADES] << "." <<
         cards[p][BRIDGE_HEARTS] << "." <<
         cards[p][BRIDGE_DIAMONDS] << "." <<
         cards[p][BRIDGE_CLUBS] << ":";
  }
  return s.str() + "\n";
}


string Deal::AsTXT(
  const Players& players) const
{
  stringstream t;

  const string pn = players.Get(BRIDGE_NORTH);
  const string pe = players.Get(BRIDGE_EAST);
  const string ps = players.Get(BRIDGE_SOUTH);
  const string pw = players.Get(BRIDGE_WEST);
  if (pn != "")
    t << setw(12) << "" << pn << "\n";

  stringstream u;
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    const string cn = (holding[BRIDGE_NORTH][s] == 0 ? 
      "void" : cards[BRIDGE_NORTH][s]);
    const string cs = (holding[BRIDGE_SOUTH][s] == 0 ? 
      "void" : cards[BRIDGE_SOUTH][s]);
    t << setw(12) << "" << left << cn << "\n";
    u << setw(12) << "" << left << cs << "\n";
  }

  if (pw != "" || pe != "")
    t << setw(24) << pw << pe << "\n";

  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    const string cw = (holding[BRIDGE_WEST][s] == 0 ? 
      "void" : cards[BRIDGE_WEST][s]);
    const string ce = (holding[BRIDGE_EAST][s] == 0 ? 
      "void" : cards[BRIDGE_EAST][s]);

    t << setw(24) << cw << ce << "\n";
  }

  if (ps != "")
    t << setw(12) << ps << "\n";

  return t.str() + u.str();
}


string Deal::AsString(
  const playerType start,
  const formatType f) const
{
  if (! setFlag)
  {
    LOG("Not set");
    return false;
  }

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Deal::AsLIN(start);

    case BRIDGE_FORMAT_PBN:
      return Deal::AsPBN(start);

    case BRIDGE_FORMAT_RBN:
      return Deal::AsRBN(start);

    default:
      return "";
  }
}

