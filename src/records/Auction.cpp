/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <regex>
#include <mutex>

#include "Auction.h"

#include "Contract.h"

#include "../util/parse.h"
#include "../handling/Bexcept.h"
#include "../handling/Bdiff.h"

static mutex mtx;


#define AUCTION_SEQ_INIT 12
#define AUCTION_SEQ_INCR 8

#define AUCTION_NUM_CALLS 38

static string AUCTION_NO_TO_CALL_LIN[AUCTION_NUM_CALLS]; // And RBN
static string AUCTION_NO_TO_CALL_PBN[AUCTION_NUM_CALLS];
static string AUCTION_NO_TO_CALL_TXT[AUCTION_NUM_CALLS];
static string AUCTION_NO_TO_CALL_EML[AUCTION_NUM_CALLS];


static const string AUCTION_DENOM_LIN[BRIDGE_DENOMS] =
{
  "C", "D", "H", "S", "N"
};

static const string AUCTION_DENOM_PBN[BRIDGE_DENOMS] =
{
  "C", "D", "H", "S", "NT"
};

static map<string, unsigned> AUCTION_CALL_TO_NO; // All syntaxes

static bool setAuctionTables = false;


Auction::Auction()
{
  Auction::reset();
  if (! setAuctionTables)
  {
    mtx.lock();
    if (! setAuctionTables)
      Auction::setTables();
    setAuctionTables = true;
    mtx.unlock();
  }
}


void Auction::reset()
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


void Auction::setTables()
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


void Auction::setDealerLIN(const string& text)
{
  unsigned u;
  if (! str2upos(text, u))
    THROW("Not a LIN dealer");
  if (u > 4)
    THROW("LIN dealer out of range");

  dealer = PLAYER_LIN_DEALER_TO_DDS[u];
}


void Auction::setDealerPBN(const string& text)
{
  // This is a "too permissive" version that accepts all formats.
  dealer = str2player(text);
  if (dealer == BRIDGE_PLAYER_SIZE) 
    THROW("Invalid dealer");
}


void Auction::setDealer(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      Auction::setDealerLIN(text);
      break;
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      Auction::setDealerPBN(text);
      break;
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
  setDVFlag = true;
}


void Auction::setVul(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      // This is a "too permissive" version that accepts all formats.
      vul = str2vul(text);
      if (vul == BRIDGE_VUL_SIZE)
        THROW("Invalid LIN vulnerability");
      break;
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
  setDVFlag = true;
}


bool Auction::hasDealerVul() const
{
  return setDVFlag;
}


Player Auction::getDealer() const
{
  if (! setDVFlag)
    THROW("Dealer not set");

  return dealer;
}


Vul Auction::getVul() const
{
  if (! setDVFlag)
    THROW("Vulnerability not set");

  return vul;
}


void Auction::copyDealerVul(const Auction& auction2)
{
  dealer = auction2.dealer;
  vul = auction2.vul;
  setDVFlag = true;
}


unsigned Auction::length() const
{
  return len;
}


void Auction::extend()
{
  lenMax += AUCTION_SEQ_INCR;
  sequence.resize(lenMax);
}


void Auction::addCallNo(
  const unsigned no,
  const string& alert)
{
  if (len == lenMax)
    Auction::extend();

  sequence[len].no = no;
  sequence[len].alert = alert;
  len++;
}


void Auction::addCall(
  const string& call,
  const string& alert)
{
  if (Auction::isOver())
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

  if (len == lenMax)
    Auction::extend();

  auto it = AUCTION_CALL_TO_NO.find(c);
  if (it == AUCTION_CALL_TO_NO.end())
    THROW("Illegal call: " + call);

  unsigned n = it->second;
  if (n == 0)
    numPasses++;
  else if (n == 1)
  {
    if (multiplier != BRIDGE_MULT_UNDOUBLED || numPasses == 1)
      THROW("Illegal double");

    multiplier = BRIDGE_MULT_DOUBLED;
    numPasses = 0;
  }
  else if (n == 2)
  {
    if (multiplier != BRIDGE_MULT_DOUBLED || numPasses == 1)
      THROW("Illegal redouble");

    multiplier = BRIDGE_MULT_REDOUBLED;
    numPasses = 0;
  }
  else if (n <= activeCNo)
    THROW("Call " + to_string(n) + " too low");
  else
  {
    numPasses = 0;
    multiplier = BRIDGE_MULT_UNDOUBLED;
    activeCNo = n;
    activeBNo = len;
  }

  if (alert == "" && alertFlag)
    Auction::addCallNo(n, "!");
  else
    Auction::addCallNo(n, alert);
}


void Auction::addAlert(
  const unsigned alertNo,
  const string& alert)
{
  // We could keep track of alerts as they happen, including
  // their bid number.  But as that takes more storage and it only
  // occurs in some formats, we do the inefficient search.

  const string lookFor = "[" + to_string(alertNo) + "]";
  for (unsigned b = 0; b < len; b++)
  {
    if (sequence[b].alert == lookFor)
    {
      sequence[b].alert = alert;
      return;
    }
  }

  THROW("Alert number not given: " + to_string(alertNo));
}


void Auction::addPasses()
{
  unsigned add = (activeCNo == 0 ? 4 - numPasses : 3 - numPasses);
  numPasses += add;
  for (unsigned p = 0; p < add; p++)
    Auction::addCallNo(0);
}


unsigned Auction::getRBNAlertNo(
  const string& s,
  size_t& pos,
  const bool extendedFlag) const
{
  char c = s.at(pos);
  if (c < '0' || c > '9')
    THROW("Bad alert number " + to_string(c));

  unsigned aNo = static_cast<unsigned>(c - '0');
  pos++;

  if (extendedFlag)
  {
    // This is not standard RBN.
    c = s.at(pos);
    pos++;
    if (pos == s.length())
      THROW("Missing end of alert");

    if (c < '0' || c > '9')
      THROW("Bad alert number, second digit " + to_string(c));

    aNo = 10*aNo + static_cast<unsigned>(c - '0');
  }
  return aNo;
}


void Auction::undoLastCall()
{
  if (len == 0)
    THROW("Can't undo before any bidding");

  if (Auction::isOver())
    THROW("Can't undo after complete bidding");

  if (sequence[len-1].no == 0)
  {
    sequence[len-1].alert = "";
    len--;
    numPasses--;
    return;
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
}


void Auction::addAlertsRBN(const vector<string>& lines)
{
  for (unsigned i = 1; i < lines.size(); i++)
  {
    string beg;
    string line = lines[i];
    if (! getNextWord(line, beg))
      THROW("Not a valid alert line");

    unsigned u;
    if (! str2upos(beg, u))
      THROW("Not a valid alert number");

    if (i != u)
      THROW("Alert numbers not in strict sequence");

    Auction::addAlert(i, line);
  }
}


void Auction::addAuctionLIN(const string& text)
{
  // This only occurs in RP-style LIN where alerts are not given.
  const size_t l = text.length();
  size_t pos = 0;

  while (1)
  {
    if (pos >= l)
      return;

    const char c = text.at(pos);
    if (c == 'P' || c == 'D' || c == 'R')
    {
      Auction::addCall(string(1, c), "");
      pos++;
    }
    else if (pos == l-1)
      THROW("Missing end of bid");
    else
    {
      Auction::addCall(text.substr(pos, 2), "");
      pos += 2;
    }
  }
}


bool Auction::isPBNNote(
  const string& text,
  int& no,
  string& alert) const
{
  regex re("^\\[Note \"(\\d+):(.*)\"\\]$");
  smatch match;
  if (regex_search(text, match, re) && match.size() >= 2)
  {
    if (! str2int(match.str(1), no))
      return false;

    alert = match.str(2);
    if (alert == "")
      alert = "!";
    return true;
  }
  else
    return false;
}


void Auction::addAuctionPBN(const vector<string>& list)
{
  if (! setDVFlag)
    THROW("Dealer and vul should be set by now");

  Player dlr = str2player(list[0]);
  if (dlr == BRIDGE_PLAYER_SIZE)
    THROW("Not a PBN dealer");

  if (dealer != dlr)
    THROW("Auction has different dealer");

  // Get the alerts from the back.
  unsigned end = static_cast<unsigned>(list.size()) - 1;
  vector<string> alerts;
  alerts.clear();
  int no;
  string alert;
  while (end > 0 && Auction::isPBNNote(list[end], no, alert))
  {
    alerts.insert(alerts.begin(), alert);
    end--;
  }

  // Get the auction (all of it for convenience).
  string word;
  vector<string> words;
  words.clear();
  for (unsigned i = 1; i <= end; i++)
  {
    vector<string> ll;
    splitIntoWords(list[i], ll);
    for (auto &s: ll)
      words.push_back(s);
  }

  const size_t l = words.size();
  for (size_t i = 0; i < l; i++)
  {
    if (i < l-1 && words[i+1].at(0) == '=')
    {
      // Alert was spaced from bid.
      unsigned ano;
      words[i+1].erase(0, 1);
      if (! str2upos(words[i+1], ano))
        THROW("Not an alert number");

      if (ano == 0 || ano > alerts.size())
        THROW("Alert number out of range: " + to_string(ano));

      Auction::addCall(words[i], alerts[ano-1]);

      // Consume the alert.
      i++;
    }
    else if (i < l-1 && words[i+1] == "$15")
    {
      Auction::addCall(words[i] + "!", "");
      i++;
    }
    else if (words[i].length() >= 4 &&
        words[i].at(words[i].length()-1) == '=')
    {
      // Alert is with bid (1S=1=).
      
      size_t p = words[i].find("=");
      const unsigned ll = static_cast<unsigned>(words[i].length());
      if (p == string::npos || p == 0 || p > ll-3)
        THROW("Odd string: " + words[i]);
      const string a = words[i].substr(p+1);

      unsigned ano;
      if (! str2upos(a, ano))
        THROW("Not an alert number");

      if (ano == 0 || ano > alerts.size())
        THROW("Alert number out of range: " + to_string(ano));

      Auction::addCall(words[i].substr(0, p), alerts[ano-1]);
    }
    else if (i == l-1 && words[i] == "AP")
    {
      Auction::addPasses();
      return;
    }
    else
      Auction::addCall(words[i]);
  }
}


void Auction::addAuctionRBN(const string& text)
{
  const size_t l = text.length();
  if (l < 4)
    THROW("String too short: " + text);

  if (text.at(2) != ':')
    THROW("Must start with 'xy:'");

  Auction::setDealerPBN(text.substr(0, 1));
  Auction::setVul(text.substr(1, 1), BRIDGE_FORMAT_RBN);
  setDVFlag = true;

  Auction::addAuctionRBNCore(text, 2);
}


void Auction::addAuctionRBNCore(
  const string& text,
  const unsigned startPos)
{
  const size_t l = text.length();
  string s = text;
  toUpper(s);
  size_t pos = startPos;
  unsigned aNo = 0;
  while (1)
  {
    if (pos >= l)
      return;

    const char c = s.at(pos);
    if (c == 'A')
    {
      Auction::addPasses();
      if (pos == l-1)
        return;
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

      aNoNew = Auction::getRBNAlertNo(s, pos, aNo >= 9);

      if (aNoNew <= aNo)
        THROW("Alerts not in ascending order");

      aNo = aNoNew;
      sequence[len-1].alert = "[" + to_string(aNo) + "]";
    }
    else if (c == '*' || c == '!') 
    {
      if (len == 0)
        THROW("Alerting before any bid");

      pos++;
      sequence[len-1].alert = '!';
    }
    else if (c == 'P' || c == 'X' || c == 'D' || c == 'R')
    {
      Auction::addCall(string(1, c), "");
      pos++;
    }
    else if (pos == l-1)
      THROW("Missing end of bid");
    else
    {
      string tt = s.substr(pos, 2);
      pos += 2;
      Auction::addCall(tt, "");
    }
  }
}


void Auction::addAuction(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      {
        vector<string> lines;
        str2lines(text, lines);
        Auction::addAuctionRBNCore(lines[0]);
        Auction::addAlertsRBN(lines);
      }
      break;
    
    case BRIDGE_FORMAT_PBN:
      {
        vector<string> lines;
        str2lines(text, lines);
        Auction::addAuctionPBN(lines);
      }
      break;
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      Auction::addAuctionRBN(text);
      break;

    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      Auction::addAuctionRBNCore(text);
      break;
    
    default:
      THROW("Unknown format: " + to_string(format));
  }
}


bool Auction::getContract(Contract& contract) const
{
  if (! Auction::isOver())
    return false;

  if (activeCNo == 0)
    contract.passOut();
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
    const Player declarer = static_cast<Player>
      ((static_cast<unsigned>(dealer) + p) % 4);

    // Switch to DDS encoding.
    if (denom != 4)
      denom = 3 - denom;

    contract.setContract(
      vul,
      declarer,
      level,
      static_cast<Denom>(denom),
      multiplier);
  }
  return true;
}


bool Auction::isPassedOut() const
{
  return (numPasses == 4);
}


bool Auction::isOver() const
{
  return (Auction::isPassedOut() ||
    (numPasses == 3 && activeCNo > 0));
}


bool Auction::isEmpty() const
{
  return (len == 0);
}


bool Auction::lateAlerts() const
{
  if (len < 3 || numPasses < 3)
    return false;

  for (unsigned b = len-3; b < len; b++)
  {
    if (sequence[b].alert != "")
      return true;
  }
  return false;
}


bool Auction::operator == (const Auction& auction2) const
{
  if (setDVFlag != auction2.setDVFlag)
    DIFF("Different DV status");
  else if (len != auction2.len)
    DIFF("Different lengths");
  else if (activeCNo != auction2.activeCNo || 
      activeBNo != auction2.activeBNo)
    DIFF("Different active numbers");
  
  for (unsigned b = 0; b < len; b++)
  {
    if (sequence[b].no != auction2.sequence[b].no ||
        sequence[b].alert != auction2.sequence[b].alert)
      DIFF("Different sequences");
  }

  return true;
}


bool Auction::operator != (const Auction& a2) const
{
  return !(* this == a2);
}


string Auction::strTXTHeader(const int * lengths) const
{
  if (lengths == nullptr)
    THROW("lengths is nullptr");

  stringstream ss;
  ss << left <<
       setw(lengths[0]) << "West" << 
       setw(lengths[1]) << "North" << 
       setw(lengths[2]) << "East" << 
       "South" << "\n";
  return ss.str();
}


string Auction::strEMLHeader() const
{
  stringstream ss;
  ss << setw(9) << left << "west" <<
      setw(9) << left << "north" <<
      setw(9) << left << "east" <<
      setw(9) << left << "south" << "\n\n\n";
  return ss.str();
}


string Auction::strLIN() const
{
  stringstream ss;
  for (unsigned b = 0; b < len; b++)
  {
    const Call& c = sequence[b];
    ss << "mb|" << AUCTION_NO_TO_CALL_LIN[c.no];
    
    if (c.alert == "")
      ss << "|";
    else if (c.alert == "!")
      ss << "!|";
    else
      ss << "|an|" << c.alert << "|";
  }
  ss << "pg||";
  return ss.str();
}


string Auction::strLIN_RP() const
{
  stringstream ss;
  ss << "\nmb|";
  for (unsigned b = 0; b < len; b++)
    ss << AUCTION_NO_TO_CALL_LIN[sequence[b].no];
  ss << "|pg||\n";
  return ss.str();
}


string Auction::strPBN() const
{
  stringstream ss, alerts;
  ss << "[Auction \"" << PLAYER_NAMES_SHORT[dealer] << "\"]\n";

  if (Auction::isPassedOut() && ! lateAlerts())
    return ss.str() + "AP\n";

  const bool shorten = (numPasses == 3 && ! lateAlerts());
  unsigned end = (shorten ? len-3 : len);
  unsigned aNo = 1;
  for (unsigned b = 0; b < end; b++)
  {
    const Call& c = sequence[b];
    if (b % 4 > 0)
      ss << " ";
    ss << AUCTION_NO_TO_CALL_PBN[c.no];
    if (c.alert != "")
    {
      if (c.alert == "!")
        ss << " $15";
      else
      {
        ss << " =" << aNo << "=";
	alerts << "[Note \"" << aNo << ":" << c.alert << "\"]\n";
	aNo++;
      }
    }
    if (b != end-1 && b % 4 == 3)
      ss << "\n";
  }

  string st = trimTrailing(ss.str());
  if (shorten)
  {
    st += (end % 4 == 0 ? "\n" : " ");
    st += "AP";
  }

  return st + "\n" + alerts.str();
}


string Auction::strRBNCore(const bool RBNflag) const
{
  stringstream s, alerts;
  s << PLAYER_NAMES_SHORT[dealer] << VUL_NAMES_RBN[vul] << ":";
  
  if (Auction::isPassedOut())
    return s.str() + "A";

  unsigned end = (numPasses == 3 ? len-3 : len);
  unsigned aNo = 1;
  for (unsigned b = 0; b < end; b++)
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
    if (b != end-1 && b % 4 == 3)
      s << ":";
  }

  if (numPasses == 3)
  {
    if (end % 4 == 0)
      s << ":";
    s << "A";
  }

  string astr = alerts.str();
  string sep;
  if (astr == "")
  {
    sep = "";
  }
  else
  {
    sep = (RBNflag ? "\n" : "");
    astr.pop_back(); // Remove last newline
  }
  return s.str() + sep + astr;
}


string Auction::strRBN() const
{
  return "A " + Auction::strRBNCore(true) + "\n";
}


string Auction::strRBX() const
{
  return "A{" + Auction::strRBNCore(false) + "}";
}


string Auction::strTXT(const int * lengths) const
{
  const unsigned numSkips = static_cast<unsigned>
    ((dealer + 4 - BRIDGE_WEST) % 4);
  const unsigned wrap = 3 - numSkips;

  stringstream ss;
  for (unsigned i = 0; i < numSkips; i++)
    ss << setw(lengths[i]) << "";
  
  if (Auction::isPassedOut())
    return ss.str() + "All Pass\n";

  unsigned end = (numPasses == 3 ? len-3 : len);
  for (unsigned b = 0; b < end; b++)
  {
    const string& call = AUCTION_NO_TO_CALL_TXT[sequence[b].no];
    if (b % 4 == wrap)
      ss << call << "\n";
    else
      ss << setw(lengths[(b+numSkips) % 4]) << left << 
        AUCTION_NO_TO_CALL_TXT[sequence[b].no];
  }

  if (numPasses == 3)
    return ss.str() + "All Pass\n";

  string st = trimTrailing(ss.str());

  if ((end-1) % 4 != wrap)
    st += "\n";

  return st;
}


string Auction::strEML() const
{
  if (Auction::isPassedOut())
    return "Passed out\n";

  const unsigned numSkips = static_cast<unsigned>
    ((dealer + 4 - BRIDGE_WEST) % 4);
  const unsigned wrap = 3 - numSkips;

  stringstream ss;
  for (unsigned i = 0; i < numSkips; i++)
    ss << setw(9) << "";
  
  unsigned end = (numPasses == 3 ? len-3 : len);
  for (unsigned b = 0; b < end; b++)
  {
    ss << setw(9) << left << AUCTION_NO_TO_CALL_EML[sequence[b].no];
    if (b % 4 == wrap)
      ss << "\n";
  }

  if (numPasses == 3)
    return ss.str() + "(all pass)\n";

  string st = trimTrailing(ss.str());

  if ((end-1) % 4 != wrap)
    st += "\n";

  return st;
}


string Auction::strREC() const
{
  const unsigned numSkips = static_cast<unsigned>
    ((dealer + 4 - BRIDGE_WEST) % 4);
  const unsigned wrap = 3 - numSkips;

  stringstream ss;
  for (unsigned i = 0; i < numSkips; i++)
    ss << setw(9) << "";
  
  if (Auction::isPassedOut())
    return ss.str() + "All Pass\n\n";

  unsigned end = (numPasses == 3 ? len-3 : len);
  for (unsigned b = 0; b < end; b++)
  {
    const string& call = AUCTION_NO_TO_CALL_TXT[sequence[b].no];
    if (b % 4 == wrap)
      ss << call << "\n";
    else
      ss << setw(9) << left << call;
  }

  if (numPasses == 3)
    return ss.str() + "All Pass\n\n";

  string st = trimTrailing(ss.str());

  if ((end-1) % 4 != wrap)
    st += "\n";

  return st + "\n";
}


string Auction::str(
  const Format format,
  const int * lengths) const
{
  if (! setDVFlag)
    DIFF("Dealer/vul not set");

  string header;
  if (format == BRIDGE_FORMAT_TXT)
  {
    header = Auction::strTXTHeader(lengths);
    if (len == 0)
      return header;
  }
  else if (format == BRIDGE_FORMAT_EML)
    header = Auction::strEMLHeader();
  else
    header = "";

  if (len == 0)
    return header;

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_TRN:
      return Auction::strLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Auction::strLIN_RP();
    
    case BRIDGE_FORMAT_LIN_VG:
      return Auction::strLIN() + "\n";

    case BRIDGE_FORMAT_PBN:
      return Auction::strPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Auction::strRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Auction::strRBX();
    
    case BRIDGE_FORMAT_TXT:
      if (lengths == nullptr)
        THROW("TXT needs lengths");
      return header + Auction::strTXT(lengths);
    
    case BRIDGE_FORMAT_EML:
      return header + Auction::strEML();
    
    case BRIDGE_FORMAT_REC:
      return Auction::strREC();
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
}


string Auction::strDealer(const Format format) const
{
  if (! setDVFlag)
    THROW("Dealer/vul not set");

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return to_string(PLAYER_DDS_TO_LIN_DEALER[dealer]);
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
      return "[Dealer \"" + PLAYER_NAMES_SHORT[dealer] + "\"]\n";
    
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      return "Dlr: " + PLAYER_NAMES_LONG[dealer];

    case BRIDGE_FORMAT_TXT:
      return PLAYER_NAMES_LONG[dealer] + " Dlr";
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
}


string Auction::strVul(const Format format) const
{
  if (! setDVFlag)
    THROW("Dealer/vul not set");

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_TRN:
      return "sv|" + VUL_NAMES_LIN[vul] + "|";

    case BRIDGE_FORMAT_LIN_VG:
      return "sv|" + VUL_NAMES_LIN[vul] + "|\n";

    case BRIDGE_FORMAT_LIN_RP:
      return "sv|" + VUL_NAMES_LIN_RP[vul] + "|\npf|y|";
    
    case BRIDGE_FORMAT_PBN:
      return "[Vulnerable \"" + VUL_NAMES_PBN[vul] + "\"]\n";
    
    case BRIDGE_FORMAT_RBN:
      return VUL_NAMES_RBN[vul];
    
    case BRIDGE_FORMAT_TXT:
      return VUL_NAMES_TXT[vul] + " Vul";
    
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      return "Vul: " + VUL_NAMES_TXT[vul];

    case BRIDGE_FORMAT_PAR:
      return VUL_NAMES_LIN[vul];
    
    default:
      THROW("Invalid format: " + to_string(format));
  }
}

