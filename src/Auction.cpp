/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Auction.h"
#include "Debug.h"
#include <map>
#include "portab.h"

extern Debug debug;


#define AUCTION_SEQ_INIT 12
#define AUCTION_SEQ_INCR 8

#define AUCTION_NUM_CALLS 38

string AUCTION_NO_TO_CALL_LIN[AUCTION_NUM_CALLS]; // And RBN
string AUCTION_NO_TO_CALL_PBN[AUCTION_NUM_CALLS];
string AUCTION_NO_TO_CALL_TXT[AUCTION_NUM_CALLS];

const string AUCTION_DENOM_LIN[BRIDGE_DENOMS] =
{
  "C", "D", "H", "S", "N"
};

const string AUCTION_DENOM_PBN[BRIDGE_DENOMS] =
{
  "C", "D", "H", "S", "NT"
};

map<string, unsigned> AUCTION_CALL_TO_NO; // All syntaxes

bool setAuctionTables = false;


Auction::Auction()
{
  Auction::Reset();
  if (! setAuctionTables)
  {
    setAuctionTables = true;
    Auction::SetTables();
  }
}


Auction::~Auction()
{
}


void Auction::Reset()
{
  setDVFlag = false;
  len = 0;
  lenMax = AUCTION_SEQ_INIT;
  sequence.resize(lenMax);
  numPasses = 0;
  multiplier = BRIDGE_MULT_UNDOUBLED;
  activeCNo = 0;
  activeBNo = 0;
}


void Auction::SetTables()
{
  AUCTION_NO_TO_CALL_LIN[0] = "P";
  AUCTION_NO_TO_CALL_LIN[1] = "D";
  AUCTION_NO_TO_CALL_LIN[2] = "R";

  AUCTION_NO_TO_CALL_PBN[0] = "Pass";
  AUCTION_NO_TO_CALL_PBN[1] = "X";
  AUCTION_NO_TO_CALL_PBN[2] = "XX";

  AUCTION_NO_TO_CALL_TXT[0] = "pass";
  AUCTION_NO_TO_CALL_TXT[1] = "DBL";
  AUCTION_NO_TO_CALL_TXT[2] = "RDBL";

  unsigned p = 3;
  for (unsigned level = 1; level <= 7; level++)
  {
    for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    {
      stringstream s1;
      s1 << level << AUCTION_DENOM_LIN[d] << "";
      AUCTION_NO_TO_CALL_LIN[p] = s1.str();

      stringstream s2;
      s2 << level << AUCTION_DENOM_PBN[d] << "";
      AUCTION_NO_TO_CALL_PBN[p] = s2.str();
      AUCTION_NO_TO_CALL_TXT[p] = s2.str();

      p++;
    }
  }

  for (p = 0; p < AUCTION_NUM_CALLS; p++)
  {
    AUCTION_CALL_TO_NO[AUCTION_NO_TO_CALL_LIN[p]] = p;
    AUCTION_CALL_TO_NO[AUCTION_NO_TO_CALL_PBN[p]] = p;
    AUCTION_CALL_TO_NO[AUCTION_NO_TO_CALL_TXT[p]] = p;
  }

  AUCTION_CALL_TO_NO["PASS"] = 0;
}


bool Auction::SetDealerVul(
  const playerType d,
  const vulType v)
{
  if (setDVFlag)
  {
    LOG("Dealer and vulnerability already set");
    return false;
  }

  if (d >= BRIDGE_NORTH_SOUTH)
  {
    LOG("Invalid dealer " + STR(d));
    return false;
  }

  if (v >= BRIDGE_VUL_SIZE)
  {
    LOG("Invalid vulnerability " + STR(v));
    return false;
  }

  setDVFlag = true;
  dealer = d;
  vul = v;
  return true;
}


bool Auction::IsOver() const
{
  return (Auction::IsPassedOut() ||
    (numPasses == 3 && activeCNo > 0));
}


bool Auction::IsPassedOut() const
{
  return (numPasses == 4);
}


void Auction::AddCallNo(
  const unsigned no,
  const string& alert)
{
  sequence[len].no = no;
  sequence[len].alert = alert;
  len++;
}


bool Auction::AddCall(
  const string& call,
  const string& alert)
{
  if (Auction::IsOver())
  {
    LOG("Call after auction is over");
    return false;
  }

  if (call == "")
  {
    LOG("Empty call");
    return false;
  }

  string c = call;
  bool alertFlag = false;
  if (c.back() == '!')
  {
    alertFlag = true;
    c.pop_back();
  }

  map<string, unsigned>::iterator it = AUCTION_CALL_TO_NO.find(c);
  if (it == AUCTION_CALL_TO_NO.end())
  {
    LOG("Illegal call: " + call);
    return false;
  }

  if (len == lenMax)
  {
    lenMax += AUCTION_SEQ_INCR;
    sequence.resize(lenMax);
  }

  unsigned n = AUCTION_CALL_TO_NO[c];
  if (n == 0)
    numPasses++;
  else if (n == 1)
  {
    if (multiplier != BRIDGE_MULT_UNDOUBLED)
    {
      LOG("Illegal double");
      return false;
    }

    multiplier = BRIDGE_MULT_DOUBLED;
    numPasses = 0;
  }
  else if (n == 2)
  {
    if (multiplier != BRIDGE_MULT_DOUBLED)
    {
      LOG("Illegal redouble");
      return false;
    }
  }
  else if (n <= activeCNo)
  {
    LOG("Call " + STR(n) + " too low");
    return false;
  }
  else
  {
    numPasses = 0;
    multiplier = BRIDGE_MULT_UNDOUBLED;
    activeCNo = n;
    activeBNo = len;
  }

  if (alert == "" && alertFlag)
    Auction::AddCallNo(n, "!");
  else
    Auction::AddCallNo(n, alert);
  return true;
}


bool Auction::AddAlert(
  const unsigned alertNo,
  const string& alert)
{
  // We could keep track of alerts as they happen, including
  // their bid number.  But that takes more storage and it only
  // occurs in some formats, we do the inefficient search.

  const string lookFor = "[" + STR(alertNo) + "]";
  for (unsigned b = 0; b < len; b++)
  {
    if (sequence[b].alert == lookFor)
    {
      sequence[b].alert = alert;
      return true;
    }
  }
  return false;
}

void Auction::AddPasses()
{
  unsigned add = (activeCNo == 0 ? 4 - numPasses : 3 - numPasses);
  numPasses += add;
  for (unsigned p = 0; p < add; p++)
    Auction::AddCallNo(0);
}


bool Auction::UndoLastCall()
{
  if (len == 0)
  {
    LOG("Can't undo before any bidding");
    return false;
  }

  if (Auction::IsOver())
  {
    LOG("Can't undo after complete bidding");
    return false;
  }

  if (sequence[len-1].no == 0)
  {
    sequence[len-1].alert = "";
    len--;
    numPasses--;
    return true;
  }

  sequence[len-1].alert = "";
  len--;
  numPasses = 0;
  while (sequence[len-1-numPasses].no == 0)
    numPasses++;

  unsigned p = len - numPasses;
  if (sequence[p].no > 2)
    multiplier = BRIDGE_MULT_UNDOUBLED;
  else
  {
    multiplier = (sequence[p].no == 1 ? 
      BRIDGE_MULT_DOUBLED : BRIDGE_MULT_REDOUBLED);
   while (sequence[p].no <= 2)
     p--;
  }

  activeCNo = sequence[p].no;
  activeBNo = p;
  return true;
}


bool Auction::ParseRBNDealer(const char c)
{
  switch(c)
  {
    case 'N':
      dealer = BRIDGE_NORTH;
      break;
    case 'E':
      dealer = BRIDGE_EAST;
      break;
    case 'S':
      dealer = BRIDGE_SOUTH;
      break;
    case 'W':
      dealer = BRIDGE_WEST;
      break;
    default:
      return false;
  }
  return true;
}


bool Auction::ParseRBNVul(const char c)
{
  switch(c)
  {
    case 'Z':
      vul = BRIDGE_VUL_NONE;
      break;
    case 'N':
      vul = BRIDGE_VUL_NORTH_SOUTH;
      break;
    case 'E':
      vul = BRIDGE_VUL_EAST_WEST;
      break;
    case 'B':
      vul = BRIDGE_VUL_BOTH;
      break;
    default:
      return false;
  }
  return true;
}


bool Auction::GetRBNAlertNo(
  const string& s,
  unsigned& pos,
  unsigned& aNo,
  const bool extendedFlag) const
{
  char c = s.at(pos);
  if (c < '0' || c > '9')
  {
    LOG("Bad alert number " + STR(c));
    return false;
  }
  aNo = static_cast<unsigned>(c - '0');
  pos++;

  if (extendedFlag)
  {
    // This is not standard RBN.
    c = s.at(pos);
    pos++;
    if (pos == s.length())
    {
      LOG("Missing end of alert");
      return false;
    }

    if (c < '0' || c > '9')
    {
      LOG("Bad alert number, second digit " + STR(c));
      return false;
    }
    aNo = 10*aNo + static_cast<unsigned>(c - '0');
  }
  return true;
}


bool Auction::AddAuctionRBN(const string& s)
{
  const size_t l = s.length();
  if (s.length() < 5)
  {
    LOG("String too short: " + s);
    return false;
  }

  if (s.at(0) != 'A' || s.at(1) != ' ' || s.at(4) != ':')
  {
    LOG("Must start with 'A xy:'");
    return false;
  }

  if (! Auction::ParseRBNDealer(s.at(2)))
  {
    LOG("Bad dealer " + STR(s.at(2)));
    return false;
  }

  if (! Auction::ParseRBNVul(s.at(3)))
  {
    LOG("Bad vulnerability " + STR(s.at(3)));
    return false;
  }

  size_t pos = 4;
  unsigned aNo = 0;
  while (1)
  {
    if (pos >= l)
      return true;

    const char c = s.at(pos);
    if (c == 'A')
    {
      Auction::AddPasses();
      if (pos == l-1)
        return true;
      else
      {
        LOG("Characters trailing all-pass");
	return false;
      }
    }
    else if (c == ':')
    {
      // In real RBN this is only permitted between groups of
      // four bids.
      pos++;
    }
    else if (c == '^')
    {
      unsigned aNoNew;
      pos++;
      if (pos >= l)
      {
        LOG("Missing end of alert");
	return false;
      }

      if (! Auction::GetRBNAlertNo(s, pos, aNoNew, aNo > 9))
      {
        LOG("Bad alert number");
	return false;
      }

      if (aNoNew <= aNo)
      {
        LOG("Alerts not in ascending order");
	return false;
      }
      aNo = aNoNew;
      sequence[activeBNo].alert = "[" + STR(aNo) + "]";
    }
    else if (c == '*') 
    {
      pos++;
      sequence[activeBNo].alert = '!';
    }
    else if (c == 'P' || c == 'X' || c == 'R')
    {
      Auction::AddCall(string(1, c), "");
      pos++;
    }
    else if (pos == l-1)
    {
      LOG("Missing end of bid");
      return false;
    }
    else
    {
      string t = s.substr(pos, 2);
      pos += 2;
      if (! Auction::AddCall(t, ""))
        return false;
    }
  }
}


bool Auction::AddAuction(
  const string& s,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("Auction LIN type not implemented");
      return false;
    
    case BRIDGE_FORMAT_PBN:
      LOG("Auction PBN type not implemented");
      return false;
    
    case BRIDGE_FORMAT_RBN:
      return Auction::AddAuctionRBN(s);
    
    case BRIDGE_FORMAT_TXT:
      LOG("Auction TXT type not implemented");
      return false;
    
    default:
      LOG("Unknown auction type");
      return false;
  }
}


bool Auction::operator == (const Auction& a2)
{
  if (setDVFlag != a2.setDVFlag ||
      len != a2.len || 
      activeCNo != a2.activeCNo ||
      activeBNo != a2.activeBNo)
    return false;
  
  for (unsigned b = 0; b < len; b++)
  {
    if (sequence[b].no != a2.sequence[b].no ||
        sequence[b].alert != a2.sequence[b].alert)
      return false;
  }

  return true;
}


bool Auction::operator != (const Auction& a2)
{
  return !(* this == a2);
}


bool Auction::ConsistentWith(const Contract& cref) const
{
  if (! Auction::IsOver())
    return false;

  Contract cown;
  if (activeCNo == 0)
    cown.SetContract(
      vul, 
      BRIDGE_NORTH, 
      0, 
      BRIDGE_NOTRUMP,
      BRIDGE_MULT_UNDOUBLED);
  else
  {
    const unsigned level = (activeCNo + 2) / 5;
    unsigned denom = (activeCNo - 5*level + 2);

    // Find the declaring side's earliest bid in denom.
    unsigned p;
    for (unsigned b = activeBNo % 2; b <= activeBNo; b += 2)
    {
      if (sequence[b].no > 2 && (sequence[b].no + 2) % 5 == denom)
      {
        p = b;
	break;
      }
    }
    const playerType declarer = static_cast<playerType>
      ((dealer + p) % 4);

    // Switch to DDS encoding.
    if (denom != 4)
      denom = 3 - denom;

    cown.SetContract(
      vul,
      declarer,
      level,
      static_cast<denomType>(denom),
      multiplier);
  }

  return (cown == cref);
}

string Auction::AsLIN() const
{
  if (len == 0)
    return "";

  stringstream s;
  for (unsigned b = 0; b < len; b++)
  {
    const Call& c = sequence[b];
    s << "mb|" << AUCTION_NO_TO_CALL_LIN[c.no];
    
    if (c.alert == "")
      s << "|";
    else if (c.alert == "!")
      s << "!|";
    else
      s << "|an|" << c.alert << "|";
  }
  s << "pg||";
  return s.str();
}


string Auction::AsPBN() const
{
  if (len == 0)
    return "";
  
  stringstream s, alerts;
  s << "[Auction \"" << PLAYER_NAMES_LONG[dealer] << "\"]\n";
  
  unsigned aNo = 1;
  for (unsigned b = 0; b < len; b++)
  {
    const Call& c = sequence[b];
    s << AUCTION_NO_TO_CALL_PBN[c.no] << " ";
    if (c.alert != "")
    {
      if (c.alert == "!")
        s << "$15 ";
      else
      {
        s << "=" << aNo << "= ";
	alerts << "[Note \"" << aNo << ":" << c.alert << "\"]\n";
	aNo++;
      }
    }
    if (b % 4 == 3)
      s << "\n";
  }
  if (len % 4 != 3)
    s << "\n";

  return s.str() + alerts.str();
}


string Auction::AsRBN() const
{
  if (len == 0)
    return "";
  
  stringstream s, alerts;
  s << "A " << 
    PLAYER_NAMES_SHORT[dealer] <<
    VUL_NAMES_SHORT_RBN[vul] << ":";
  
  if (Auction::IsPassedOut())
    return s.str();

  unsigned trailing = 0;
  unsigned end = len-1;
  while (sequence[end].no == 0 && sequence[end].alert == "")
    trailing++, end--;
    
  unsigned aNo = 1;
  for (unsigned b = 0; b <= end; b++)
  {
    const Call& c = sequence[b];
    if (c.no == 1)
      s << "X";
    else
      s << AUCTION_NO_TO_CALL_LIN[c.no];
    if (c.alert == "!")
    {
      s << "*";
    }
    else if (c.alert != "")
    {
      s << "^" << aNo;
      alerts << aNo << " " << c.alert << "\n";
      aNo++;
    }
  }

  if (trailing == 3)
    s << "A";
  return s.str() + "\n" + alerts.str();
}


string Auction::AsTXT(const string& names) const
{
  stringstream s, alerts;
  unsigned aNo = 1;
  s << left <<
       setw(15) << "West" << 
       setw(15) << "North" << 
       setw(15) << "East" << 
       setw(15) << "South" << "\n";
  s << names;

  if (len == 0)
    return s.str();

  const unsigned numSkips = static_cast<unsigned>
    ((dealer + 4 - BRIDGE_WEST) % 4);
  const unsigned wrap = 3 - numSkips;
  for (unsigned i = 0; i < numSkips; i++)
    s << setw(15) << "";
  
  for (unsigned b = 0; b < len; b++)
  {
    const Call& c = sequence[b];
    stringstream bid;
    bid << AUCTION_NO_TO_CALL_TXT[c.no];
    if (c.alert != "")
    {
      if (c.alert == "!")
        bid << " (*)";
      else
      {
        bid << " (" << aNo << ")";
	alerts << "(" << aNo << ") " << c.alert << "\n";
	aNo++;
      }
    }
    s << setw(15) << bid.str();
    if (b % 4 == wrap)
      s << "\n";
  }

  if (len % 4 != wrap)
    s << "\n";
  return s.str() + alerts.str();
}


string Auction::AsString(
  const formatType f,
  const string& names) const
{
  if (! setDVFlag)
  {
    LOG("Dealer/vul not set");
    return "";
  }

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Auction::AsLIN();
    
    case BRIDGE_FORMAT_PBN:
      return Auction::AsPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Auction::AsRBN();
    
    case BRIDGE_FORMAT_TXT:
      return Auction::AsTXT(names);
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}

