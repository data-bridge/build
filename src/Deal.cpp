/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <vector>
#include <map>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif

#include "Deal.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"

static mutex mtx;


// DDS encoding, but without the two bottom 00 bits: ((2 << 13) - 1)

#define MAX_HOLDING 0x1fff

static const string CARDS[BRIDGE_TRICKS] =
{
  "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A"
};

static const string CARDS_TXT[BRIDGE_TRICKS] =
{
  "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"
};


// Global translation tables that are set once and for all.
static bool setDealTables = false;
static string HOLDING_TO_SUIT[MAX_HOLDING+1];
static string HOLDING_TO_SUIT_TXT[MAX_HOLDING+1];
static map<string, unsigned> SUIT_TO_HOLDING;
static map<string, Player> PLAYER_TO_DDS;


Deal::Deal()
{
  Deal::reset();
  if (! setDealTables)
  {
    mtx.lock();
    if (! setDealTables)
      Deal::setTables();
    setDealTables = true;
    mtx.unlock();
  }
}


Deal::~Deal()
{
}


void Deal::reset()
{
  setFlag = false;
}


void Deal::setTables()
{
  for (unsigned h = 0; h <= MAX_HOLDING; h++)
  {
    string suit("");
    string suitTXT("");
    for (int bit = 12; bit >= 0; bit--)
    {
      if (h & static_cast<unsigned>(1 << bit))
      {
        suit += CARDS[bit];
        suitTXT += CARDS_TXT[bit] + " ";
      }
    }
    HOLDING_TO_SUIT[h] = suit;
    HOLDING_TO_SUIT_TXT[h] = suitTXT;
    SUIT_TO_HOLDING[suit] = h;
    reverse(suit.begin(), suit.end());
    SUIT_TO_HOLDING[suit] = h;
  }

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    PLAYER_TO_DDS[PLAYER_NAMES_SHORT[p]] = static_cast<Player>(p);
}


bool Deal::isSet() const
{
  return setFlag;
}


Player Deal::strToPlayer(const string& text) const
{
  auto it = PLAYER_TO_DDS.find(text);
  if (it == PLAYER_TO_DDS.end())
    THROW("No such player: '" + text + "'");

  return it->second;
}


unsigned Deal::suitToHolding(const string suit) const
{
  auto it = SUIT_TO_HOLDING.find(suit);
  if (it == SUIT_TO_HOLDING.end())
    THROW("No such suit: '" + suit + "'");

  return it->second;
}


void Deal::setHand(
  const string& hand,
  const string& delimiters,
  const unsigned offset,
  unsigned pholding[])
{
  int seen = 0;
  for (unsigned i = 0; i < delimiters.length(); i++)
    seen += count(hand.begin(), hand.end(), delimiters.at(i));

  if (seen != 3 + static_cast<int>(offset))
    THROW("Not the right number of delimiters");

  vector<string> suits(BRIDGE_SUITS+1);
  suits.clear();
  tokenize(hand, suits, delimiters);

  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    pholding[s] = suitToHolding(suits[s+offset]);
}


void Deal::setHands()
{
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    unsigned sum = 0, xsum = 0;
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      strcpy(cards[p][s], HOLDING_TO_SUIT[holding[p][s]].c_str());
      sum += holding[p][s];
      xsum ^= holding[p][s];
    }
    if (sum != MAX_HOLDING || xsum != MAX_HOLDING)
      THROW("Cards do not add up");
  }
}


void Deal::setLIN(const string& text)
{
  size_t c = countDelimiters(text, ",");
  if (c != 2 && c != 3)
    THROW("Not 2 or 3 commas");

  vector<string> tokens(BRIDGE_PLAYERS);
  tokens.clear();
  tokenize(text, tokens, ",");

  // Last is derived, not given (it is re-derived even if given).
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    holding[PLAYER_LIN_TO_DDS[3]][s] = MAX_HOLDING;
  
  for (unsigned plin = 0; plin <= 2; plin++)
  {
    if (countDelimiters(tokens[plin], "SHDC") != 4)
      THROW("Not 4 suits");

    const Player p = PLAYER_LIN_TO_DDS[plin];
    Deal::setHand(tokens[plin], "SHDC", 1, holding[p]);

    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      holding[PLAYER_LIN_TO_DDS[3]][s] ^= holding[p][s];
  }

  Deal::setHands();
}


void Deal::setPBN(const string& text)
{
  if (countDelimiters(text, ": ") != 4)
    THROW("Not 4 space delimiters in '" + text + "'");

  vector<string> tokens(BRIDGE_PLAYERS+1);
  tokens.clear();
  tokenize(text, tokens, ": ");

  const Player first = strToPlayer(tokens[0]);

  for (unsigned pno = 0; pno < BRIDGE_PLAYERS; pno++)
  {
    if (countDelimiters(tokens[pno+1], ".") != 3)
      THROW("Not 3 periods");

    const unsigned p = (static_cast<unsigned>(first) + pno) % 4;
    Deal::setHand(tokens[pno+1], ".", 0, holding[p]);
  }

  Deal::setHands();
}


void Deal::setRBN(const string& text)
{
  size_t c = countDelimiters(text, ":");
  if (c != 3 && c != 4)
    THROW("Not 3 or 4 colons");

  vector<string> tokens(BRIDGE_PLAYERS+1);
  tokens.clear();
  tokenize(text, tokens, ":");

  const Player first = strToPlayer(tokens[0]);
  const unsigned firstU = static_cast<unsigned>(first);

  // Last is derived, not given (it is re-derived even if given).
  const unsigned last = (firstU + 3) % 4;
  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
    holding[last][suit] = MAX_HOLDING;
  
  for (unsigned pno = 0; pno <= 2; pno++)
  {
    if (countDelimiters(tokens[pno+1], ".") != 3)
      THROW("Not 3 periods");

    const unsigned p = (firstU + pno) % 4;
    Deal::setHand(tokens[pno+1], ".", 0, holding[p]);

    for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
      holding[last][suit] ^= holding[p][suit];
  }

  Deal::setHands();
}


void Deal::set(
  const string& text,
  const Format format)
{
  if (text == "")
    THROW("Empty text");

  if (setFlag)
    THROW("Already set");

  setFlag = true;
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      Deal::setLIN(text);
      break;

    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      Deal::setPBN(text);
      break;

    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      Deal::setRBN(text);
      break;

    default:
      THROW("Invalid format:" + STR(format));
  }
}


void Deal::set(const unsigned holdingArg[][BRIDGE_SUITS])
{
  if (setFlag)
    THROW("Holding already set");

  setFlag = true;

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      holding[p][s] = holdingArg[p][s] >> 2;
}


Player Deal::holdsCard(const string& text) const
{
  if (text.length() != 2)
    return BRIDGE_PLAYER_SIZE;

  unsigned s, h;
  const char sc = text.at(0);
  bool found = false;

  for (unsigned i = 0; i < BRIDGE_NOTRUMP; i++)
  {
    if (DENOM_NAMES_SHORT[i] == sc)
    {
      found = true;
      s = i;
      break;
    }
  }
  if (! found)
    return BRIDGE_PLAYER_SIZE;

  auto it = SUIT_TO_HOLDING.find(text.substr(1));
  if (it == SUIT_TO_HOLDING.end())
    return BRIDGE_PLAYER_SIZE;
  h = it->second;

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    if (holding[p][s] & h)
      return static_cast<Player>(p);
  }
  return BRIDGE_PLAYER_SIZE;
}


void Deal::getDDS(unsigned holdingArg[][BRIDGE_SUITS]) const
{
  if (! setFlag)
    THROW("Holding not set");

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      holdingArg[p][s] = holding[p][s] << 2;
}


bool Deal::operator == (const Deal& deal2) const
{
  if (setFlag != deal2.setFlag)
    DIFF("Different setFlags");

  if (! setFlag && ! deal2.setFlag)
    return true;
  
  for (unsigned plin = 0; plin < BRIDGE_PLAYERS; plin++)
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      if (holding[plin][s] != deal2.holding[plin][s])
        DIFF("Different holdings");

  return true;
}


bool Deal::operator != (const Deal& deal2) const
{
  return ! (* this == deal2);
}


string Deal::strLINReverse(const Player start) const
{
  // This is an evil hack to get the cards printed in reverse order
  // while leaving everything else in the correct order.
  // Players are always in same order.

  stringstream ss;
  ss << ",";
  for (unsigned p = 0; p <= 2; p++)
  {
    unsigned pdds = (4-p) % 4;
    ss << cards[pdds][BRIDGE_CLUBS] << "C" <<
         cards[pdds][BRIDGE_DIAMONDS] << "D" <<
         cards[pdds][BRIDGE_HEARTS] << "H" <<
         cards[pdds][BRIDGE_SPADES] << "S";
    if (p < 2)
      ss << ",";
  }
  ss << PLAYER_DDS_TO_LIN_DEALER[start] << "|dm";

  string st = ss.str();
  return string(st.rbegin(), st.rend()) + "|";
}


string Deal::strLINRegular(
  const Player start,
  const unsigned limit) const
{
  stringstream ss;
  ss << "md|" << PLAYER_DDS_TO_LIN_DEALER[start];
  
  // Players are always in same order.
  for (unsigned p = 0; p < limit; p++)
  {
    unsigned pdds = PLAYER_LIN_TO_DDS[p];
    for (unsigned d = 0; d < BRIDGE_SUITS; d++)
      ss << DENOM_NAMES_SHORT[d] << cards[pdds][d];
    if (p != limit-1)
      ss << ",";
  }
    
  return ss.str() + "|";
}


string Deal::strPBN(const Player start) const
{
  stringstream ss;
  ss << "[Deal \"" << PLAYER_NAMES_SHORT[start] << ":";

  // Players start from dealer.
  for (unsigned pno = 0; pno < BRIDGE_PLAYERS; pno++)
  {
    unsigned p = (static_cast<unsigned>(start) + pno) % 4;
    ss << cards[p][BRIDGE_SPADES] << "." <<
         cards[p][BRIDGE_HEARTS] << "." <<
         cards[p][BRIDGE_DIAMONDS] << "." <<
         cards[p][BRIDGE_CLUBS];
    if (pno < 3)
      ss << " ";
  }
  return ss.str() + "\"]\n";
}


string Deal::strRBNCore(const Player start) const
{
  stringstream ss;
  ss << PLAYER_NAMES_SHORT[start] << ":";

  // Players start from here.
  for (unsigned pno = 0; pno <= 2; pno++)
  {
    unsigned p = (static_cast<unsigned>(start) + pno) % 4;
    ss << cards[p][BRIDGE_SPADES] << "." <<
         cards[p][BRIDGE_HEARTS] << "." <<
         cards[p][BRIDGE_DIAMONDS] << "." <<
         cards[p][BRIDGE_CLUBS] << ":";
  }
  return ss.str();
}


string Deal::strRBN(const Player start) const
{
  return "H " + Deal::strRBNCore(start) + "\n";
}


string Deal::strRBX(const Player start) const
{
  return "H{" + Deal::strRBNCore(start) + "}";
}


string Deal::strTXT() const
{
  stringstream sstop;
  stringstream ssbot;

  sstop << setw(14) << "" << "North\n";
  ssbot << setw(14) << "" << "South\n";

  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    const string cn = (holding[BRIDGE_NORTH][s] == 0 ? 
      "--" : HOLDING_TO_SUIT_TXT[holding[BRIDGE_NORTH][s]]);
    const string cs = (holding[BRIDGE_SOUTH][s] == 0 ? 
      "--" : HOLDING_TO_SUIT_TXT[holding[BRIDGE_SOUTH][s]]);
    sstop << setw(12) << "" <<
      DENOM_NAMES_SHORT_PBN[s] << " " << left << cn << "\n";
    ssbot << setw(12) << "" <<
      DENOM_NAMES_SHORT_PBN[s] << " " << left << cs << "\n";
  }

  sstop << "  West" << setw(20) << "" << "East\n";

  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    const string cw = (holding[BRIDGE_WEST][s] == 0 ? 
      "--" : HOLDING_TO_SUIT_TXT[holding[BRIDGE_WEST][s]]);
    const string ce = (holding[BRIDGE_EAST][s] == 0 ? 
      "--" : HOLDING_TO_SUIT_TXT[holding[BRIDGE_EAST][s]]);

    sstop << left << DENOM_NAMES_SHORT_PBN[s] << " " << 
        setw(22) << left << cw <<
        DENOM_NAMES_SHORT_PBN[s] << " " <<
        left << ce << "\n";
  }

  return sstop.str() + ssbot.str();
}


string Deal::strEML() const
{
  stringstream ss;
  ss << setw(12) << "" << "north\n\n";
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    ss << setw(12) << "" << DENOM_NAMES_SHORT[s] << " " <<
      cards[BRIDGE_NORTH][s] << "\n";
  }

  ss << setw(23) << left << "west" << "east\n\n";

  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    ss << DENOM_NAMES_SHORT[s] << " " << setw(21) << left << 
      cards[BRIDGE_WEST][s] <<
      DENOM_NAMES_SHORT[s] << " " << setw(13) << left <<
      cards[BRIDGE_EAST][s] << "\n";
  }

  ss << setw(12) << "" << "south\n\n";
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    ss << setw(12) << "" << DENOM_NAMES_SHORT[s] << " " <<
      cards[BRIDGE_SOUTH][s] << "\n";
  }
  return ss.str();
}


string Deal::strRECDetail(
  const Player midPlayer,
  const unsigned LRsuit,
  const unsigned mSuit) const
{
  stringstream ss;

  ss << DENOM_NAMES_SHORT[LRsuit] << " " << setw(10) << left <<
      cards[BRIDGE_WEST][LRsuit];

  ss << DENOM_NAMES_SHORT[mSuit] << " " << setw(10) << left <<
      cards[midPlayer][mSuit];

  ss << DENOM_NAMES_SHORT[LRsuit] << " " << setw(10) << left <<
      cards[BRIDGE_EAST][LRsuit];
  
  return ss.str();
}

string Deal::strREC() const
{
  stringstream ss;
  ss <<  "\n";
  for (unsigned s = 0; s < BRIDGE_SUITS-1; s++)
    ss << setw(12) << "" << DENOM_NAMES_SHORT[s] << " " <<
      cards[BRIDGE_NORTH][s] << "\n";

  ss << Deal::strRECDetail(BRIDGE_NORTH, BRIDGE_SPADES, BRIDGE_CLUBS) << 
    "\n";

  for (unsigned s = 1; s < BRIDGE_SUITS-1; s++)
  {
    ss << DENOM_NAMES_SHORT[s] << " " << setw(22) << left <<
        cards[BRIDGE_WEST][s];

    ss << DENOM_NAMES_SHORT[s] << " " << setw(10) << left <<
        cards[BRIDGE_EAST][s] << "\n";
  }

  ss << Deal::strRECDetail(BRIDGE_SOUTH, BRIDGE_CLUBS, BRIDGE_SPADES) << 
    "\n";

  for (unsigned s = 1; s < BRIDGE_SUITS; s++)
    ss << setw(12) << "" << DENOM_NAMES_SHORT[s] << " " <<
      cards[BRIDGE_SOUTH][s] << "\n";

  return ss.str();
}


string Deal::str(
  const Player start,
  const Format format) const
{
  if (! setFlag)
    THROW("Not set");

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_TRN:
      return "st||" + Deal::strLINReverse(start) + "rh||";

    case BRIDGE_FORMAT_LIN_VG:
      return "st||" + Deal::strLINRegular(start, BRIDGE_PLAYERS);

    case BRIDGE_FORMAT_LIN_RP:
      return Deal::strLINRegular(start, BRIDGE_PLAYERS-1);

    case BRIDGE_FORMAT_PBN:
      return Deal::strPBN(start);

    case BRIDGE_FORMAT_RBN:
      return Deal::strRBN(start);

    case BRIDGE_FORMAT_RBX:
      return Deal::strRBX(start);

    case BRIDGE_FORMAT_TXT:
      return Deal::strTXT();

    case BRIDGE_FORMAT_EML:
      return Deal::strEML();

    case BRIDGE_FORMAT_REC:
      return Deal::strREC();

    default:
      THROW("Deal format not implemented: " + STR(format));
  }
}

