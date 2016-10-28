/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <map>
#include <regex>
#include <assert.h>

#include "Auction.h"
#include "Contract.h"
#include "parse.h"
#include "portab.h"
#include "Bexcept.h"
#include "Bdiff.h"


#define AUCTION_SEQ_INIT 12
#define AUCTION_SEQ_INCR 8

#define AUCTION_NUM_CALLS 38

string AUCTION_NO_TO_CALL_LIN[AUCTION_NUM_CALLS]; // And RBN
string AUCTION_NO_TO_CALL_PBN[AUCTION_NUM_CALLS];
string AUCTION_NO_TO_CALL_EML[AUCTION_NUM_CALLS];
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

  AUCTION_NO_TO_CALL_EML[0] = "pass";
  AUCTION_NO_TO_CALL_EML[1] = "X";
  AUCTION_NO_TO_CALL_EML[2] = "XX";

  AUCTION_NO_TO_CALL_TXT[0] = "Pass";
  AUCTION_NO_TO_CALL_TXT[1] = "Dbl";
  AUCTION_NO_TO_CALL_TXT[2] = "Rdbl";

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
      AUCTION_NO_TO_CALL_EML[p] = s2.str();
      AUCTION_NO_TO_CALL_TXT[p] = s2.str();

      p++;
    }
  }

  for (p = 0; p < AUCTION_NUM_CALLS; p++)
  {
    AUCTION_CALL_TO_NO[AUCTION_NO_TO_CALL_LIN[p]] = p;
    AUCTION_CALL_TO_NO[AUCTION_NO_TO_CALL_PBN[p]] = p;
    AUCTION_CALL_TO_NO[AUCTION_NO_TO_CALL_EML[p]] = p;
    AUCTION_CALL_TO_NO[AUCTION_NO_TO_CALL_TXT[p]] = p;
  }

  AUCTION_CALL_TO_NO["PASS"] = 0;
}


bool Auction::SetDealerLIN(
  const string& d,
  playerType& p) const
{
  unsigned u;
  if (! StringToNonzeroUnsigned(d, u))
    THROW("Not a LIN dealer");

  if (u > 4)
    THROW("LIN dealer out of range");

  p = static_cast<playerType>((u+1) % 4);
  return true;
}


bool Auction::SetDealerPBN(
  const string& d,
  playerType& p) const
{
  if (d == "N")
    p = BRIDGE_NORTH;
  else if (d == "E")
    p = BRIDGE_EAST;
  else if (d == "S")
    p = BRIDGE_SOUTH;
  else if (d == "W")
    p = BRIDGE_WEST;
  else
    THROW("Invalid PBN dealer");

  return true;
}


bool Auction::SetDealerTXT(
  const string& d,
  playerType& p) const
{
  if (d == "North")
    p = BRIDGE_NORTH;
  else if (d == "East")
    p = BRIDGE_EAST;
  else if (d == "South")
    p = BRIDGE_SOUTH;
  else if (d == "West")
    p = BRIDGE_WEST;
  else
    THROW("Invalid PBN dealer");

  return true;
}


bool Auction::SetVulLIN(
  const string& v,
  vulType& vOut) const
{
  if (v == "o" || v == "O" || v == "0") // Pavlicek uses 0 -- wrong?
    vOut = BRIDGE_VUL_NONE;
  else if (v == "e" || v == "E")
    vOut = BRIDGE_VUL_EAST_WEST;
  else if (v == "n" || v == "N")
    vOut = BRIDGE_VUL_NORTH_SOUTH;
  else if (v == "b" || v == "B")
    vOut = BRIDGE_VUL_BOTH;
  else
    THROW("Invalid LIN vulnerability");

  return true;
}

bool Auction::SetVulPBN(
  const string& v,
  vulType& vOut) const
{
  if (v == "None")
    vOut = BRIDGE_VUL_NONE;
  else if (v == "NS")
    vOut = BRIDGE_VUL_NORTH_SOUTH;
  else if (v == "EW")
    vOut = BRIDGE_VUL_EAST_WEST;
  else if (v == "All")
    vOut = BRIDGE_VUL_BOTH;
  else
    THROW("Invalid PBN vulnerability");

  return true;
}

bool Auction::SetVulRBN(
  const string& v,
  vulType& vOut) const
{
  if (v == "Z")
    vOut = BRIDGE_VUL_NONE;
  else if (v == "E")
    vOut = BRIDGE_VUL_EAST_WEST;
  else if (v == "N")
    vOut = BRIDGE_VUL_NORTH_SOUTH;
  else if (v == "B")
    vOut = BRIDGE_VUL_BOTH;
  else
    THROW("Invalid RBN vulnerability");

  return true;
}

bool Auction::SetVulTXT(
  const string& v,
  vulType& vOut) const
{
  if (v == "None")
    vOut = BRIDGE_VUL_NONE;
  else if (v == "E-W")
    vOut = BRIDGE_VUL_EAST_WEST;
  else if (v == "N-S")
    vOut = BRIDGE_VUL_NORTH_SOUTH;
  else if (v == "Both")
    vOut = BRIDGE_VUL_BOTH;
  else
    THROW("Invalid TXT vulnerability");

  return true;
}


bool Auction::ParseDealerVul(
  const string& d,
  const string& v,
  const formatType f,
  playerType& dOut,
  vulType& vOut) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      if (! Auction::SetDealerLIN(d, dOut))
        return false;
      if (! Auction::SetVulLIN(v, vOut))
        return false;
      break;
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if (! Auction::SetDealerPBN(d, dOut))
        return false;
      if (! Auction::SetVulPBN(v, vOut))
        return false;
      break;
    
    case BRIDGE_FORMAT_TXT:
      if (! Auction::SetDealerTXT(d, dOut))
        return false;
      if (! Auction::SetVulTXT(v, vOut))
        return false;
      break;
    
    default:
      THROW("Invalid format " + STR(f));
  }

  return true;
}


bool Auction::SetDealerVul(
  const string& d,
  const string& v,
  const formatType f)
{
  if (setDVFlag)
    THROW("Dealer and vulnerability already set");

  playerType dOut;
  vulType vOut;
  if (! Auction::ParseDealerVul(d, v, f, dOut, vOut))
    return false;
  setDVFlag = true;
  dealer = dOut;
  vul = vOut;
  return true;
}


bool Auction::SetDealer(
  const string& d,
  const formatType f)
{
  playerType dOut;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      if (! Auction::SetDealerLIN(d, dOut))
        return false;
      setDVFlag = true;
      dealer = dOut;
      break;
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if (! Auction::SetDealerPBN(d, dOut))
        return false;
      setDVFlag = true;
      dealer = dOut;
      break;
    
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_REC:
      if (! Auction::SetDealerTXT(d, dOut))
        return false;
      setDVFlag = true;
      dealer = dOut;
      break;
    
    default:
      THROW("Invalid format " + STR(f));
  }

  return true;
}


bool Auction::SetVul(
  const string& v,
  const formatType f)
{
  vulType vOut;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      if (! Auction::SetVulLIN(v, vOut))
        return false;
      setDVFlag = true;
      vul = vOut;
      break;
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if (! Auction::SetVulPBN(v, vOut))
        return false;
      setDVFlag = true;
      vul = vOut;
      break;
    
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_REC:
      if (! Auction::SetVulTXT(v, vOut))
        return false;
      setDVFlag = true;
      vul = vOut;
      break;
    
    default:
      THROW("Invalid format " + STR(f));
  }

  return true;
}


void Auction::CopyDealerVulFrom(const Auction& a2)
{
  setDVFlag = true;
  dealer = a2.dealer;
  vul = a2.vul;
}


bool Auction::CheckDealerVul(
  const string& d,
  const string& v,
  const formatType f) const
{
  if (! setDVFlag)
    THROW("Dealer and vulnerability not set");

  playerType dOut;
  vulType vOut;
  if (! Auction::ParseDealerVul(d, v, f, dOut, vOut))
    return false;

  return (dealer == dOut && vul == vOut);
}


bool Auction::IsOver() const
{
  return (Auction::IsPassedOut() ||
    (numPasses == 3 && activeCNo > 0));
}


bool Auction::IsEmpty() const
{
  return (len == 0);
}


bool Auction::DVIsSet() const
{
  return setDVFlag;
}


vulType Auction::GetVul() const
{
  return vul;
}


bool Auction::IsPassedOut() const
{
  return (numPasses == 4);
}


playerType Auction::GetDealer() const
{
  return dealer;
}


void Auction::AddCallNo(
  const unsigned no,
  const string& alert)
{
  if (len == lenMax)
  {
    lenMax += AUCTION_SEQ_INCR;
    sequence.resize(lenMax);
  }

  sequence[len].no = no;
  sequence[len].alert = alert;
  len++;
}


bool Auction::AddCall(
  const string& call,
  const string& alert)
{
  if (Auction::IsOver())
    THROW("Call after auction is over");

  if (call == "")
    THROW("Empty call");

  string c = call;
  bool alertFlag = false;
  if (c.back() == '!')
  {
    alertFlag = true;
    c.pop_back();
  }

  map<string, unsigned>::iterator it = AUCTION_CALL_TO_NO.find(c);
  if (it == AUCTION_CALL_TO_NO.end())
    THROW("Illegal call: " + call);

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
      THROW("Illegal double");

    multiplier = BRIDGE_MULT_DOUBLED;
    numPasses = 0;
  }
  else if (n == 2)
  {
    if (multiplier != BRIDGE_MULT_DOUBLED)
      THROW("Illegal redouble");

    multiplier = BRIDGE_MULT_REDOUBLED;
    numPasses = 0;
  }
  else if (n <= activeCNo)
    THROW("Call " + STR(n) + " too low");
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
  // their bid number.  But as that takes more storage and it only
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


bool Auction::AddAlertsRBN(const vector<string>& lines)
{
  for (unsigned i = 1; i < lines.size(); i++)
  {
    string beg;
    string line = lines[i];
    if (! GetNextWord(line, beg))
      THROW("Not a valid alert line");

    unsigned u;
    if (! StringToNonzeroUnsigned(beg, u))
      THROW("Not a valid alert number");

    if (i != u)
      THROW("Alert numbers not in strict sequence");

    if (! Auction::AddAlert(i, line))
      return false;
  }
  return true;
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
    THROW("Can't undo before any bidding");

  if (Auction::IsOver())
    THROW("Can't undo after complete bidding");

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
      THROW("Unknown RBN dealer");
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
      THROW("Unknown RBN vulnerability");
  }
  return true;
}


bool Auction::GetRBNAlertNo(
  const string& s,
  size_t& pos,
  unsigned& aNo,
  const bool extendedFlag) const
{
  char c = s.at(pos);
  if (c < '0' || c > '9')
    THROW("Bad alert number " + STR(c));

  aNo = static_cast<unsigned>(c - '0');
  pos++;

  if (extendedFlag)
  {
    // This is not standard RBN.
    c = s.at(pos);
    pos++;
    if (pos == s.length())
      THROW("Missing end of alert");

    if (c < '0' || c > '9')
      THROW("Bad alert number, second digit " + STR(c));

    aNo = 10*aNo + static_cast<unsigned>(c - '0');
  }
  return true;
}


bool Auction::AddAuctionLIN(const string& s)
{
  // This only occurs in RP-style LIN where alerts are not given.
  const size_t l = s.length();
  size_t pos = 0;

  while (1)
  {
    if (pos >= l)
      return true;

    const char c = s.at(pos);
    if (c == 'P' || c == 'D' || c == 'R')
    {
      Auction::AddCall(string(1, c), "");
      pos++;
    }
    else if (pos == l-1)
      THROW("Missing end of bid");
    else
    {
      string t = s.substr(pos, 2);
      pos += 2;
      if (! Auction::AddCall(t, ""))
        return false;
    }
  }
  return true;
}


bool Auction::AddAuctionRBN(const string& s)
{
  const size_t l = s.length();
  if (l < 4)
    THROW("String too short: " + s);

  if (s.at(2) != ':')
    THROW("Must start with 'xy:'");

  if (! Auction::ParseRBNDealer(s.at(0)))
    THROW("Bad dealer " + STR(s.at(0)));

  if (! Auction::ParseRBNVul(s.at(1)))
    THROW("Bad vulnerability " + STR(s.at(1)));

  setDVFlag = true;

  return Auction::AddAuctionEML(s, 2);

}


bool Auction::AddAuctionEML(
  const string& t,
  const unsigned startPos)
{
  const size_t l = t.length();
  string s = t;
  toUpper(s);
  size_t pos = startPos;
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
        THROW("Characters trailing all-pass");
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
        THROW("Missing end of alert");

      if (! Auction::GetRBNAlertNo(s, pos, aNoNew, aNo > 9))
        THROW("Bad alert number");

      if (aNoNew <= aNo)
        THROW("Alerts not in ascending order");

      aNo = aNoNew;
      sequence[len-1].alert = "[" + STR(aNo) + "]";
    }
    else if (c == '*' || c == '!') 
    {
      pos++;
      sequence[activeBNo].alert = '!';
    }
    else if (c == 'P' || c == 'X' || c == 'D' || c == 'R')
    {
      Auction::AddCall(string(1, c), "");
      pos++;
    }
    else if (pos == l-1)
      THROW("Missing end of bid");
    else
    {
      string tt = s.substr(pos, 2);
      pos += 2;
      if (! Auction::AddCall(tt, ""))
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
      // return Auction::AddAuctionLIN(s);
      {
        vector<string> lines;
        ConvertMultilineToVector(s, lines);
        if (! Auction::AddAuctionEML(lines[0]))
          return false;

        return Auction::AddAlertsRBN(lines);
      }
    
    case BRIDGE_FORMAT_PBN:
      {
        vector<string> lines;
        ConvertMultilineToVector(s, lines);
        return Auction::AddAuctionPBN(lines);
      }
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return Auction::AddAuctionRBN(s);

    case BRIDGE_FORMAT_REC:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_TXT:
      return Auction::AddAuctionEML(s);
    
    default:
      THROW("Unknown auction type");
  }
}


bool Auction::IsPBNNote(
  const string& s,
  int& no,
  string& alert) const
{
  regex re("^\\[Note \"(\\d+):(.*)\"\\]$");
  smatch match;
  if (regex_search(s, match, re) && match.size() >= 2)
  {
    if (! StringToInt(match.str(1), no))
      return false;

    alert = match.str(2);
    return true;
  }
  else
    return false;
}


bool Auction::AddAuctionPBN(const vector<string>& list)
{
  if (! setDVFlag)
    THROW("Dealer and vul should be set by now");

  playerType dlr;
  if (! Auction::SetDealerPBN(list[0], dlr))
    THROW("Not a PBN dealer");

  if (dealer != dlr)
    THROW("Auction has different dealer");

  // Get the alerts from the back.
  unsigned end = list.size() - 1;
  vector<string> alerts;
  alerts.clear();
  int no;
  string alert;
  while (end > 0 && Auction::IsPBNNote(list[end], no, alert))
  {
    alerts.insert(alerts.begin() + no, alert);
    end--;
  }

  // Get the auction (all of it for convenience).
  string word;
  vector<string> words;
  words.clear();
  for (unsigned i = 1; i <= end; i++)
  {
    string s = list[i];
    while (GetNextWord(s, word))
      words.push_back(word);
  }

  const size_t l = words.size();
  for (size_t i = 0; i < l; i++)
  {
    if (i == l-1 || words[i+1].at(0) != '=')
    {
      if (i == l-1 && words[i] == "AP")
      {
        Auction::AddPasses();
        return true;
      }
      if (! Auction::AddCall(words[i]))
        return false;
    }
    else
    {
      unsigned ano;
      words[i+1].erase(0, 1);
      if (! StringToNonzeroUnsigned(words[i+1], ano))
        THROW("Not an alert number");

      if (ano > alerts.size()-1)
        THROW("Alert too high");

      if (! Auction::AddCall(words[i], alerts[ano]))
        return false;

      // Already consumed the alert.
      i--;
    }
  }

  return true;
}


bool Auction::AddAuction(
  const vector<string>& list,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      THROW("Auction LIN type not implemented");
    
    case BRIDGE_FORMAT_PBN:
      return Auction::AddAuctionPBN(list);
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      THROW("Auction RBN type not implemented");
    
    case BRIDGE_FORMAT_TXT:
      THROW("Auction TXT type not implemented");
    
    default:
      THROW("Unknown auction type");
  }
}


bool Auction::operator == (const Auction& a2) const
{
  if (setDVFlag != a2.setDVFlag)
    DIFF("Different DV status");
  else if (len != a2.len)
    DIFF("Different lengths");
  else if (activeCNo != a2.activeCNo || activeBNo != a2.activeBNo)
    DIFF("Different active numbers");
  
  for (unsigned b = 0; b < len; b++)
  {
    if (sequence[b].no != a2.sequence[b].no ||
        sequence[b].alert != a2.sequence[b].alert)
      DIFF("Different sequences");
  }

  return true;
}


bool Auction::operator != (const Auction& a2) const
{
  return !(* this == a2);
}


bool Auction::ExtractContract(Contract& contract) const
{
  if (! Auction::IsOver())
    return false;

  if (activeCNo == 0)
    contract.SetPassedOut();
  else
  {
    const unsigned level = (activeCNo + 2) / 5;
    unsigned denom = (activeCNo - 5*level + 2);

    // Find the declaring side's earliest bid in denom.
    unsigned p = 0;
    for (unsigned b = activeBNo % 2; b <= activeBNo; b += 2)
    {
      if (sequence[b].no > 2 && (sequence[b].no + 2) % 5 == denom)
      {
        p = b;
	break;
      }
    }
    const playerType declarer = static_cast<playerType>
      ((static_cast<unsigned>(dealer) + p) % 4);

    // Switch to DDS encoding.
    if (denom != 4)
      denom = 3 - denom;

    contract.SetContract(
      vul,
      declarer,
      level,
      static_cast<denomType>(denom),
      multiplier);
  }
  return true;
}


bool Auction::ConsistentWith(const Contract& cref) const
{
  Contract cown;
  if (! Auction::ExtractContract(cown))
    return false;

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


string Auction::AsLIN_RP() const
{
  if (len == 0)
    return "";

  stringstream s;
  s << "\nmb|";
  for (unsigned b = 0; b < len; b++)
  {
    const Call& c = sequence[b];
    s << AUCTION_NO_TO_CALL_LIN[c.no];
  }
  s << "|pg||\n";
  return s.str();
}


string Auction::AsPBN() const
{
  if (len == 0)
    return "";
  
  stringstream s, alerts;
  if (Auction::IsPassedOut())
  {
    s << "[Auction \"" << PLAYER_NAMES_SHORT[dealer] << "\"]\nAP\n";
    return s.str();
  }

  s << "[Auction \"" << PLAYER_NAMES_SHORT[dealer] << "\"]\n";
  
  unsigned trailing = 0;
  unsigned end = len-1;
  while (sequence[end].no == 0 && sequence[end].alert == "")
    trailing++, end--;

  if (trailing > 0 && trailing < 3)
    end = len-1;
    
  unsigned aNo = 1;
  for (unsigned b = 0; b <= end; b++)
  {
    const Call& c = sequence[b];
    if (b % 4 > 0)
      s << " ";
    s << AUCTION_NO_TO_CALL_PBN[c.no];
    if (c.alert != "")
    {
      if (c.alert == "!")
        s << " $15";
      else
      {
        s << " =" << aNo << "=";
	alerts << "[Note \"" << aNo << ":" << c.alert << "\"]\n";
	aNo++;
      }
    }
    if (b % 4 == 3)
      s << "\n";
  }

  string st = s.str();
  if (trailing == 3)
  {
    if (end % 4 != 3)
      st += " ";
    st += "AP\n";
  }
  else
  {
    // TODO: General function to trim trailing spaces
    int pos = static_cast<int>(st.length()) - 1;
    while (pos >= 0 && st.at(static_cast<unsigned>(pos)) == ' ')
      pos--;
    st = st.substr(0, static_cast<unsigned>(pos)+1);
    if (end % 4 != 3)
      st += "\n";
  }

  return st + alerts.str();
}


string Auction::AsRBNCore(const bool RBNflag) const
{
  if (len == 0)
    return "";
  
  stringstream s, alerts;
  s << PLAYER_NAMES_SHORT[dealer] << VUL_NAMES_RBN[vul] << ":";
  
  if (Auction::IsPassedOut())
    return s.str() + "A";

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
      if (RBNflag)
        alerts << aNo << " " << c.alert << "\n";
      else
        alerts << aNo << "{" << c.alert << "}";
      aNo++;
    }
    if (b != end && b % 4 == 3)
      s << ":";
  }

  if (trailing == 3)
  {
    if (end % 4 == 3)
      s << ":";
    s << "A";
  }
  else if (trailing > 0)
  {
    // Incomplete bidding
    for (unsigned t = 0; t < trailing; t++)
    {
      if ((end+t) % 4 == 3)
        s << ":";
      s << "P";
    }
  }

  if (RBNflag)
  {
    if (alerts.str() == "")
      return s.str();
    else
      return s.str() + "\n" + alerts.str();
  }
  else
    return s.str() + alerts.str();
}


string Auction::AsRBN() const
{
  const string s = Auction::AsRBNCore(true);
  if (s == "")
    return "";

  return "A " + s + "\n";
}


string Auction::AsRBX() const
{
  const string s = Auction::AsRBNCore(false);
  if (s == "")
    return "";

  return "A{" + s + "}";
}


string Auction::AsEML() const
{
  stringstream s;
  s << setw(9) << left << "west" <<
      setw(9) << left << "north" <<
      setw(9) << left << "east" <<
      setw(9) << left << "south" << "\n\n\n";

  if (len == 0)
    return s.str();

  if (Auction::IsPassedOut())
  {
    s << "Passed out\n";
    return s.str();
  }

  const unsigned numSkips = static_cast<unsigned>
    ((dealer + 4 - BRIDGE_WEST) % 4);
  const unsigned wrap = 3 - numSkips;
  for (unsigned i = 0; i < numSkips; i++)
    s << setw(9) << "";
  
  unsigned trailing = 0;
  unsigned end = len-1;
  while (sequence[end].no == 0 && sequence[end].alert == "")
    trailing++, end--;
    
  if (trailing > 0 && trailing < 3)
    end = len-1;
    
  for (unsigned b = 0; b <= end; b++)
  {
    const Call& c = sequence[b];
    stringstream bid;
    bid << AUCTION_NO_TO_CALL_EML[c.no];
    s << setw(9) << bid.str();
    if (b % 4 == wrap)
      s << "\n";
  }

  if (trailing == 3)
  {
    s << "(all pass)\n";
    return s.str();
  }

  string st = s.str();
  if (trailing > 0)
  {
    // TODO: General function to trim trailing spaces
    int pos = static_cast<int>(st.length()) - 1;
    while (pos >= 0 && st.at(static_cast<unsigned>(pos)) == ' ')
      pos--;
    st = st.substr(0, static_cast<unsigned>(pos)+1);
  }

  if (end % 4 != wrap)
    st += "\n";

  return st;
}


string Auction::AsTXT(const unsigned lengths[]) const
{
  int adjLen[BRIDGE_PLAYERS];
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    adjLen[p] = Max(12, static_cast<int>(lengths[p])+1);

  stringstream s;
  s << left <<
       setw(adjLen[0]) << "West" << 
       setw(adjLen[1]) << "North" << 
       setw(adjLen[2]) << "East" << 
       "South" << "\n";

  if (len == 0)
    return s.str();

  const unsigned numSkips = static_cast<unsigned>
    ((dealer + 4 - BRIDGE_WEST) % 4);
  const unsigned wrap = 3 - numSkips;
  for (unsigned i = 0; i < numSkips; i++)
    s << setw(adjLen[i]) << "";
  
  if (Auction::IsPassedOut())
  {
    s << "All Pass\n";
    return s.str();
  }

  unsigned trailing = 0;
  unsigned end = len-1;
  while (sequence[end].no == 0 && sequence[end].alert == "")
    trailing++, end--;

  if (trailing > 0 && trailing < 3)
    end = len-1;
    
  for (unsigned b = 0; b <= end; b++)
  {
    const Call& c = sequence[b];
    stringstream bid;
    bid << AUCTION_NO_TO_CALL_TXT[c.no];
    if (b % 4 == wrap)
      s << bid.str() << "\n";
    else
      s << setw(adjLen[(b+numSkips) % 4]) << bid.str();
  }

  if (trailing == 3)
  {
    s << "All Pass\n";
    return s.str();
  }

  string st = s.str();
  // TODO: General function to trim trailing spaces
  int pos = static_cast<int>(st.length()) - 1;
  while (pos >= 0 && st.at(static_cast<unsigned>(pos)) == ' ')
    pos--;
  st = st.substr(0, static_cast<unsigned>(pos)+1);

  if (end % 4 != wrap)
    st += "\n";

  return st;
}


string Auction::AsREC() const
{

  if (len == 0)
    return "";

  stringstream s;
  const unsigned numSkips = static_cast<unsigned>
    ((dealer + 4 - BRIDGE_WEST) % 4);
  const unsigned wrap = 3 - numSkips;
  for (unsigned i = 0; i < numSkips; i++)
    s << setw(9) << "";
  
  if (Auction::IsPassedOut())
  {
    s << "All Pass\n\n";
    return s.str();
  }

  unsigned trailing = 0;
  unsigned end = len-1;
  while (sequence[end].no == 0 && sequence[end].alert == "")
    trailing++, end--;
    
  if (trailing > 0 && trailing < 3)
    end = len-1;
    
  for (unsigned b = 0; b <= end; b++)
  {
    const Call& c = sequence[b];
    stringstream bid;
    bid << AUCTION_NO_TO_CALL_TXT[c.no];
    if (b % 4 == wrap)
      s << bid.str() << "\n";
    else
      s << setw(9) << left << bid.str();
  }

  if (trailing == 3)
  {
    s << "All Pass\n";
    return s.str() + "\n";
  }

  string st = s.str();
  if (st.at(st.length()-1) == ' ')
  {
    // TODO: General function to trim trailing spaces
    int pos = static_cast<int>(st.length()) - 1;
    while (pos >= 0 && st.at(static_cast<unsigned>(pos)) == ' ')
      pos--;
    st = st.substr(0, static_cast<unsigned>(pos)+1);
  }

  if (end % 4 != wrap)
    st += "\n";

  return st + "\n";
}


string Auction::AsString(
  const formatType f,
  const string& names) const
{
  UNUSED(names);
  if (! setDVFlag)
    DIFF("Dealer/vul not set");

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_TRN:
      return Auction::AsLIN();

    case BRIDGE_FORMAT_LIN_VG:
      return Auction::AsLIN() + "\n";

    case BRIDGE_FORMAT_LIN_RP:
      return Auction::AsLIN_RP();
    
    case BRIDGE_FORMAT_PBN:
      return Auction::AsPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Auction::AsRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Auction::AsRBX();
    
    case BRIDGE_FORMAT_EML:
      return Auction::AsEML();
    
    case BRIDGE_FORMAT_TXT:
      THROW("TXT needs lengths");
    
    case BRIDGE_FORMAT_REC:
      return Auction::AsREC();
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


string Auction::AsString(
  const formatType f,
  const unsigned lengths[BRIDGE_PLAYERS]) const
{
  if (! setDVFlag)
    THROW("Dealer/vul not set");

  switch(f)
  {
    case BRIDGE_FORMAT_TXT:
      return Auction::AsTXT(lengths);
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


string Auction::DealerAsLIN() const
{
  stringstream s;
  s << PLAYER_DDS_TO_LIN_DEALER[dealer];
  return s.str();
}


string Auction::DealerAsPBN() const
{
  return "[Dealer \"" + PLAYER_NAMES_SHORT[dealer] + "\"]\n";
}


string Auction::DealerAsEML() const
{
  return "Dlr: " + PLAYER_NAMES_LONG[dealer];
}


string Auction::DealerAsTXT() const
{
  return PLAYER_NAMES_LONG[dealer] + " Dlr";
}


string Auction::VulAsLIN() const
{
  return "sv|" + VUL_NAMES_LIN[vul] + "|";
}


string Auction::VulAsLIN_RP() const
{
  return "sv|" + VUL_NAMES_LIN_RP[vul] + "|\npf|y|";
}


string Auction::VulAsPBN() const
{
  return "[Vulnerable \"" + VUL_NAMES_PBN[vul] + "\"]\n";
}


string Auction::VulAsRBN() const
{
  return VUL_NAMES_RBN[vul];
}


string Auction::VulAsEML() const
{
  return "Vul: " + VUL_NAMES_TXT[vul];
}


string Auction::VulAsTXT() const
{
  return VUL_NAMES_TXT[vul] + " Vul";
}


string Auction::DealerAsString(
  const formatType f) const
{
  if (! setDVFlag)
    THROW("Dealer/vul not set");

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Auction::DealerAsLIN();
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
      return Auction::DealerAsPBN();
    
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      return Auction::DealerAsEML();

    case BRIDGE_FORMAT_TXT:
      return Auction::DealerAsTXT();
    
    default:
      THROW("Invalid format " + STR(f));
  }
}


string Auction::VulAsString(
  const formatType f) const
{
  if (! setDVFlag)
    THROW("Dealer/vul not set");

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return Auction::VulAsLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Auction::VulAsLIN_RP();
    
    case BRIDGE_FORMAT_PBN:
      return Auction::VulAsPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Auction::VulAsRBN();
    
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      return Auction::VulAsEML();
    
    case BRIDGE_FORMAT_TXT:
      return Auction::VulAsTXT();
    
    default:
      THROW("Invalid format " + STR(f));
  }
}

