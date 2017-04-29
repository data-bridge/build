/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif

#include "Contract.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"

static mutex mtx;


static const bool VUL_LOOKUP[BRIDGE_VUL_SIZE][BRIDGE_PLAYER_SIZE] =
{
  {false, false, false, false, false, false, false},
  {true, true, true, true, true, true, false},
  {true, false, true, false, true, false, false},
  {false, true, false, true, false, true, false}
};

static const string VUL_RBN_TAG[BRIDGE_VUL_SIZE] =
{
  "Z", "B", "N", "E"
};

static const int DECLARER_DDS_TO_SIGN[BRIDGE_PLAYER_SIZE] =
{
  1, -1, 1, -1, 1, -1, 0
};

static const string LEVEL_SHIFT_TO_TAG[20] =
{
  "-13", "-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", 
  "-3", "-2", "-1", "=", "+1", "+2", "+3", "+4", "+5", "+6"
};

static map<string, int> LEVEL_TAG_TO_RELATIVE;

static const unsigned DENOM_DDS_TO_CATEGORY[BRIDGE_DENOMS] =
{
  2, 2, 1, 1, 3
};

static const string DENOM_DDS_TO_PBN_TAG[BRIDGE_DENOMS] =
{
  "C", "D", "H", "S", "NT"
};

static const string DENOM_SUPERSET_TAG[6] =
{
  "C", "D", "H", "S", "N", "NT"
};

static const Denom DENOM_SUPERSET_NUM[6] =
{
  BRIDGE_CLUBS, BRIDGE_DIAMONDS, BRIDGE_HEARTS, BRIDGE_SPADES,
  BRIDGE_NOTRUMP, BRIDGE_NOTRUMP
};

static const string MULT_NUM_TO_LIN_TAG[3] =
{
  "", "x", "xx"
};

static const string MULT_NUM_TO_PBN_TAG[3] =
{
  "", "X", "XX"
};

static const string MULT_NUM_TO_RBN_TAG[3] =
{
  "", "X", "R"
};

static const string MULT_NUM_TO_PAR_TAG[3] =
{
  "", "*", "**"
};

static const string MULT_SUPERSET_TAG[6] =
{
  "", "x", "xx", "X", "XX", "R"
};

static const Multiplier MULT_SUPERSET_NUM[6] =
{
  BRIDGE_MULT_UNDOUBLED,
  BRIDGE_MULT_DOUBLED,
  BRIDGE_MULT_REDOUBLED,
  BRIDGE_MULT_DOUBLED,
  BRIDGE_MULT_REDOUBLED,
  BRIDGE_MULT_REDOUBLED
};

static const int CONTRACT_SCORES[22][2][3] =
{
  { {   0,   -1,   -1}, {   0,   -1,   -1} }, // passed out
  { {  70,  140,  230}, {  70,  140,  230} }, // 1m
  { {  80,  160,  520}, {  80,  160,  720} }, // 1M
  { {  90,  180,  560}, {  90,  180,  760} }, // 1N
  { {  90,  180,  560}, {  90,  180,  760} }, // 2m
  { { 110,  470,  640}, { 110,  670,  840} }, // 2M
  { { 120,  490,  680}, { 120,  690,  880} }, // 2N
  { { 110,  470,  640}, { 110,  670,  840} }, // 3m
  { { 140,  530,  760}, { 140,  730,  960} }, // 3M
  { { 400,  550,  800}, { 600,  750, 1000} }, // 3N
  { { 130,  510,  720}, { 130,  710,  920} }, // 4m
  { { 420,  590,  880}, { 620,  790, 1080} }, // 4M
  { { 430,  610,  920}, { 630,  810, 1120} }, // 4N
  { { 400,  550,  800}, { 600,  750, 1000} }, // 5m
  { { 450,  650, 1000}, { 650,  850, 1200} }, // 5M
  { { 460,  670, 1040}, { 660,  870, 1240} }, // 5N
  { { 920, 1090, 1380}, {1370, 1540, 1830} }, // 6m
  { { 980, 1210, 1620}, {1430, 1660, 2070} }, // 6M
  { { 990, 1230, 1660}, {1440, 1680, 2110} }, // 6N
  { {1440, 1630, 1960}, {2140, 2330, 2660} }, // 7m
  { {1510, 1770, 2240}, {2210, 2470, 2940} }, // 7M
  { {1520, 1790, 2280}, {2220, 2490, 2980} }, // 7N
};

static const int DENOM_TO_OVERTRICKS_UNDOUBLED[BRIDGE_DENOMS] =
{
  30, 30, 20, 20, 30 
};

static const unsigned IMPscale[26] =
{
     0,   10,   40,   80,  120,  160,  210,  260,
   310,  360,  420,  490,  590,  740,  890, 1090,
  1290, 1490, 1740, 1990, 2240, 2490, 2990, 3490,
  3990, 9990
};

static unsigned IMPlookup[501];

static bool setContractTables = false;

struct Entry
{
  ContractInternal contract;
  int tricksRelative;
};

static map<string, Entry> CONTRACT_STRING_TO_PARTS;


Contract::Contract()
{
  Contract::reset();
  if (! setContractTables)
  {
    mtx.lock();
    if (! setContractTables)
      Contract::setTables();
    setContractTables = true;
    mtx.unlock();
  }
}


Contract::~Contract()
{
}


void Contract::reset()
{
  setContractFlag = false;
  setVulFlag = false;
  setResultFlag = false;
}


void Contract::setTables()
{
  Entry e;
  for (e.contract.level = 1; e.contract.level <= 7; e.contract.level++)
  {
    stringstream s1;
    s1 << e.contract.level;

    for (unsigned dno = 0; dno < 6; dno++)
    {
      e.contract.denom = DENOM_SUPERSET_NUM[dno];
      stringstream s2;
      s2 << s1.str() << DENOM_SUPERSET_TAG[dno];

      for (unsigned mno = 0; mno < 6; mno++)
      {
        e.contract.mult = MULT_SUPERSET_NUM[mno];
	stringstream s3;
	s3 << s2.str() << MULT_SUPERSET_TAG[mno];

        for (unsigned decl = 0; decl < BRIDGE_PLAYERS; decl++)
	{
	  e.contract.declarer = static_cast<Player>(decl);
	  stringstream s4;
	  s4 << s3.str() << PLAYER_NAMES_SHORT[decl];

          stringstream s4b;
	  s4b << s2.str() << PLAYER_NAMES_SHORT[decl] << 
            MULT_SUPERSET_TAG[mno];

	  // No trick number given, e.g "4HE".
	  e.tricksRelative = 7;
	  CONTRACT_STRING_TO_PARTS[s4.str()] = e;
	  CONTRACT_STRING_TO_PARTS[s4b.str()] = e;

          // RBN string with ":" and without result.
          stringstream s4a;
          s4a << s3.str() << ":" << PLAYER_NAMES_SHORT[decl];
          CONTRACT_STRING_TO_PARTS[s4a.str()] = e;

	  int lo = 7 - static_cast<int>(e.contract.level);
	  for (int i = lo; i <= lo+13; i++)
	  {
	    stringstream s5, s5b;
	    e.tricksRelative = i-13;
	    s5 << s4.str() << LEVEL_SHIFT_TO_TAG[i];
	    CONTRACT_STRING_TO_PARTS[s5.str()] = e;

	    s5b << s4b.str() << LEVEL_SHIFT_TO_TAG[i];
	    CONTRACT_STRING_TO_PARTS[s5b.str()] = e;
	  }
	}
      }
    }
  }

  for (int i = 0; i < 20; i++)
    LEVEL_TAG_TO_RELATIVE[LEVEL_SHIFT_TO_TAG[i]] = i-13;
  
  // IMPlookup  1,  2,  3,  4,  5,  6,  7, ...
  // value      0,  0,  1,  1,  1,  2,  2, ...
  // score      0, 10, 20, 30, 40, 50, 60, ...
  
  IMPlookup[0] = 0; // Unused
  for (unsigned i = 1; i < 501; i++)
  {
    unsigned j = 10 * (i-1);
    unsigned hit = 1;
    while (j > IMPscale[hit])
      hit++;

    IMPlookup[i] = hit-1;
  }
}


bool Contract::isSet() const
{
  return setContractFlag;
}


bool Contract::hasResult() const
{
  return setResultFlag;
}


void Contract::passOut()
{
  if (setContractFlag)
  {
    if (contract.level != 0)
    {
      if (tricksRelative < -13 || tricksRelative > 6)
      {
        THROW("Contract already set to " + 
            Contract::strRBNCore());
      }
      else
      {
        THROW("Contract already set to " + 
            Contract::strRBNCore() +
            LEVEL_SHIFT_TO_TAG[tricksRelative + 13]);
      }
    }
    return;
  }

  setContractFlag = true;
  contract.level = 0;
  setResultFlag = true;
  tricksRelative = 0;
  score = 0;
}


void Contract::setContractByString(const string& text)
{
  if (text == "P" || text == "p" || text == "Pass" || text == "PASS")
  {
    Contract::passOut();
    return;
  }

  auto it = CONTRACT_STRING_TO_PARTS.find(text);
  if (it == CONTRACT_STRING_TO_PARTS.end())
    THROW("Invalid string: '" + text + "'");

  setContractFlag = true;
  Entry entry = it->second;
  contract = entry.contract;
  if (entry.tricksRelative != 7)
  {
    setResultFlag = true;
    tricksRelative = entry.tricksRelative;
    Contract::calculateScore();
  }
}


void Contract::setContract(
  const Vul vulIn,
  const Player declarer,
  const unsigned level,
  const Denom denom,
  const Multiplier mult)
{
  if (setContractFlag)
  {
    if (contract.level == 0)
    {
      if (contract.level != level)
        THROW("Contract already passed out");
    }
    else if (vul != vulIn ||
        contract.declarer != declarer ||
        contract.level != level ||
	contract.denom != denom ||
	contract.mult != mult)
      THROW("Contract already set differently");
  }
  else if (level == 0)
    THROW("level must be > 0");
  else
  {
    setContractFlag = true;
    setVulFlag = true;
    vul = vulIn;
    contract.declarer = declarer;
    contract.level = level;
    contract.denom = denom;
    contract.mult = mult;
  }
}


void Contract::setContract(
  const Vul vulIn,
  const string& cstring)
{
  Contract::setContractByString(cstring);
  setVulFlag = true;
  vul = vulIn;
}


void Contract::setContractTXT(const string& text)
{
  string mod, wd;
  if (text.find(" ") == string::npos)
  {
    Contract::setContractByString(text);
    return;
  }

  if (! readNextWord(text, 0, mod))
  {
    Contract::setContractByString(text);
    return;
  }

  if (! readLastWord(text, wd))
  {
    Contract::setContractByString(text);
    return;
  }

  if (wd == "North")
    mod += "N";
  else if (wd == "East")
    mod += "E";
  else if (wd == "South")
    mod += "S";
  else if (wd == "West")
    mod += "W";
  else 
  {
    Contract::setContractByString(text);
    return;
  }

  Contract::setContractByString(mod);
}


void Contract::setContract(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      Contract::setContractByString(text);
      break;

    case BRIDGE_FORMAT_TXT:
      Contract::setContractTXT(text);
      break;

    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Contract::setDeclarer(const string& text)
{
  contract.declarer = str2player(text);
  if (contract.declarer == BRIDGE_PLAYER_SIZE)
    THROW("Invalid declarer");
}


void Contract::setVul(const Vul v)
{
  if (setVulFlag && vul != v)
    THROW("Vulnerability already set differently: " +
      VUL_NAMES_LIN[vul] + " vs. " + VUL_NAMES_LIN[v]);

  vul = v;
  setVulFlag = true;
}


bool Contract::isPassedOut() const
{
  if (setResultFlag)
    return (contract.level == 0);
  else
    return false;
}


void Contract::setTricks(const unsigned tricksIn)
{
  int trel = static_cast<int>(tricksIn) -
    static_cast<int>(contract.level + 6);

  if (setResultFlag)
  {
    if (tricksRelative != trel)
    {
      if (! Contract::isPassedOut() || tricksIn != 0)
        THROW("Tricks already set to " + STR(Contract::getTricks()));
    }
  }
  else if (! setContractFlag)
    THROW("Cannot set tricks before a contract is entered");
  else
  {
    setResultFlag = true;
    tricksRelative = trel;
    Contract::calculateScore();
  }
}


unsigned Contract::getTricks() const
{
  // No checking.
  return static_cast<unsigned>
    (tricksRelative + static_cast<int>(contract.level) + 6);
}


void Contract::textToTricks(const string& text)
{
  unsigned u;
  if (! str2unsigned(text, u))
    THROW("Not an unsigned result: " + text);
  Contract::setTricks(u);
}


void Contract::setResultTXT(const string& text)
{
  unsigned u;
  string wd1, wd2;

  if (! readNextWord(text, 0, wd1))
    THROW("No first word");

  if (! readNextWord(text, 5, wd2))
    THROW("No second word");

  if (! str2upos(wd2, u))
    THROW("Second word is not a valid number");

  if (wd1 == "Down")
    u = static_cast<unsigned>(static_cast<int>(contract.level + 6 - u));
  else if (wd1 == "Made")
    u += 6;
  else
    THROW("First word is invalid");

  Contract::setTricks(u);
}


void Contract::setResultEML(const string& text)
{
  int i;
  unsigned u;

  if (! str2int(text, i))
    THROW("Not an integer result");

  if (i > 0)
    u = static_cast<unsigned>(i + 6); // Possible Pavlicek error?
  else
    u = static_cast<unsigned>(i + static_cast<int>(contract.level + 6));

  Contract::setTricks(u);
}


void Contract::setResult(
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
      Contract::textToTricks(text);
      break;

    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      if ((text == "P" || text.substr(0, 2) == "P:"))
      {
        if (! Contract::isPassedOut())
          THROW("Contract is not passed out");
        break;
      }

      Contract::textToTricks(text);
      break;
      
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_REC:
      Contract::setResultTXT(text);
      break;

    case BRIDGE_FORMAT_EML:
      Contract::setResultEML(text);
      break;

    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Contract::setScore(
  const string& text,
  const Format format)
{
  int s;
  if (format == BRIDGE_FORMAT_PBN)
  {
    // Recognize "NS 50" and "EW 50".
    string pp = text.substr(0, 3);
    int sign = 0;
    if (pp == "NS ")
      sign = 1;
    else if (pp == "EW ")
      sign = -1;

    if (sign)
    {
      pp = text.substr(3, string::npos);
      if (! str2int(pp, s))
        THROW("Invalid score");

      score = sign * s;
      return;
    }
  }

  if (! str2int(text, s))
    THROW("Invalid score");

  score = s;
}


void Contract::calculateScore()
{
  if (! setContractFlag)
    return;
    // THROW("No contract set");

  if (! setVulFlag)
    return;
    // THROW("No vulnerability set");

  if (! setResultFlag)
    return;
    // THROW("No result set");

  if (contract.level == 0)
  {
    score = 0;
    return;
  }

  unsigned cno = 3 * (contract.level-1) + 
    DENOM_DDS_TO_CATEGORY[contract.denom];
  bool vulSide = VUL_LOOKUP[vul][contract.declarer];

  int baseScore = 0, adder = 0;
  if (tricksRelative > 0)
  {
    baseScore = CONTRACT_SCORES[cno][vulSide][contract.mult];
    adder = (contract.mult == BRIDGE_MULT_UNDOUBLED ?
      tricksRelative * DENOM_TO_OVERTRICKS_UNDOUBLED[contract.denom] :
      contract.mult * tricksRelative * (vulSide ? 200 : 100));
  }
  else if (tricksRelative == 0)
  {
    baseScore = CONTRACT_SCORES[cno][vulSide][contract.mult];
  }
  else if (contract.mult == BRIDGE_MULT_UNDOUBLED)
  {
    adder = tricksRelative * (vulSide ? 100 : 50);
  }
  else if (tricksRelative <= -3 || vulSide)
  {
    adder = 100 + 300 * tricksRelative;
    if (! vulSide)
      adder += 300;
    adder *= contract.mult;
  }
  else
  {
    adder = contract.mult * (100 + 200 * tricksRelative);
  }

  score = DECLARER_DDS_TO_SIGN[contract.declarer] * (baseScore + adder);
}


Player Contract::getDeclarer() const
{
  if (! setContractFlag || contract.level == 0)
    return BRIDGE_NORTH_SOUTH; // Error
  return contract.declarer;
}


Denom Contract::getDenom() const
{
  if (! setContractFlag || contract.level == 0)
    return BRIDGE_NOTRUMP; // Not recognizable as error!
  return contract.denom;
}


int Contract::getScore() const
{
  if (! setContractFlag || ! setVulFlag || ! setResultFlag)
    THROW("Can't get score");

  return score;
}


int Contract::diffToIMPs(const int d) const
{
  int v, sign;
  if (d < 0)
  {
    v = -d;
    sign = -1;
  }
  else
  {
    v = d;
    sign = 1;
  }

  return sign * static_cast<int>(IMPlookup[v/10 + 1]);
}


bool Contract::operator == (const Contract& c2) const
{
  if (setVulFlag != c2.setVulFlag)
    DIFF("Vulnerabilities are not equally set");

  if (setVulFlag && vul != c2.vul)
    DIFF("Vulnerabilities differ");

  if (setContractFlag)
  {
    if (! c2.setContractFlag)
      DIFF("First contract is set, second one is not");
    else if (contract.level == 0)
    {
      if (c2.contract.level == 0)
        return true;
      else
        DIFF("First contract is passed out, second one is not");
    }
    else if (contract.declarer != c2.contract.declarer)
      DIFF("Declarers differ");
    else if (contract.level != c2.contract.level)
      DIFF("Levels differ");
    else if (contract.denom != c2.contract.denom)
      DIFF("Denominations differ");
    else if (contract.mult != c2.contract.mult)
      DIFF("Multipliers differ");
    else if (setResultFlag && ! c2.setResultFlag)
      DIFF("First result is set, second one is not");
    else if (! setResultFlag && c2.setResultFlag)
      DIFF("First result is not set, second one is");
    else if (setResultFlag && tricksRelative != c2.tricksRelative)
      DIFF("Results differ");
    else
      return true;
  }
  else if (! c2.setContractFlag)
    return true;
  else
    DIFF("First constract is not set, second one is");
}


bool Contract::operator != (const Contract& c2) const
{
  return ! (* this == c2);
}


string Contract::strLIN(const Format format) const
{
  if (! setContractFlag)
    return "";
  
  if (contract.level == 0)
    return (format == BRIDGE_FORMAT_LIN_VG ? "PASS" : "P");

  stringstream ss;
  ss << 
    contract.level <<
    DENOM_NAMES_SHORT[contract.denom] <<
    PLAYER_NAMES_SHORT[contract.declarer] << 
    MULT_NUM_TO_LIN_TAG[contract.mult];

  if (setResultFlag)
    ss << LEVEL_SHIFT_TO_TAG[tricksRelative + 13];

  return ss.str();
}


string Contract::strPBN() const
{
  if (! setContractFlag)
    return "";
  
  if (contract.level == 0)
    return "[Contract \"Pass\"]\n";

  stringstream ss;
  ss << "[Contract \"" <<
    contract.level <<
    DENOM_NAMES_SHORT_PBN[contract.denom] <<
    MULT_NUM_TO_PBN_TAG[contract.mult] <<
    "\"]\n";

  return ss.str();
}


string Contract::strRBNCore() const
{
  if (! setContractFlag)
    return "";
  
  if (contract.level == 0)
    return "";

  stringstream s;
  s << 
    contract.level <<
    DENOM_NAMES_SHORT[contract.denom] <<
    MULT_NUM_TO_RBN_TAG[contract.mult] << 
    ":" <<
    PLAYER_NAMES_SHORT[contract.declarer];

  return s.str();
}


string Contract::strRBN() const
{
  const string st = Contract::strRBNCore();
  if (st == "")
    return "";
  return "C " + st + "\n";
}


string Contract::strRBX() const
{
  const string st = Contract::strRBNCore();
  if (st == "")
    return "";
  return "C{" + st + "}";
}


string Contract::strTXT() const
{
  if (! setContractFlag)
    return "";
  
  if (contract.level == 0)
    return "";

  stringstream ss;
  ss << contract.level << 
    DENOM_NAMES_SHORT_PBN[contract.denom] << 
    MULT_NUM_TO_LIN_TAG[contract.mult] << " " <<
    PLAYER_NAMES_LONG[contract.declarer] << "\n";

  return ss.str();
}


string Contract::strPar() const
{
  if (! setContractFlag || ! setResultFlag)
    return "";
  
  if (contract.level == 0)
    return "pass";

  stringstream ss;
  ss << 
    contract.level <<
    DENOM_NAMES_SHORT[contract.denom] <<
    MULT_NUM_TO_PAR_TAG[contract.mult] << 
    "-" <<
    PLAYER_NAMES_SHORT[contract.declarer] << 
    LEVEL_SHIFT_TO_TAG[tricksRelative + 13];

  return ss.str();
}


string Contract::str(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return Contract::strLIN(format);

    case BRIDGE_FORMAT_PBN:
      return Contract::strPBN();

    case BRIDGE_FORMAT_RBN:
      return Contract::strRBN();

    case BRIDGE_FORMAT_RBX:
      return Contract::strRBX();

    case BRIDGE_FORMAT_TXT:
      return Contract::strTXT();

    case BRIDGE_FORMAT_PAR:
      return Contract::strPar();

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strDeclarer(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      if (! setResultFlag)
        return "";
      else if (contract.level == 0)
        return "[Declarer \"\"]\n";

      return "[Declarer \"" + 
          PLAYER_NAMES_SHORT[contract.declarer] + "\"]\n";

    case BRIDGE_FORMAT_PAR:
      return PLAYER_NAMES_SHORT[contract.declarer];

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strDenom(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PAR:
      if (! setResultFlag)
        return "";
      else 
        return STR(DENOM_NAMES_SHORT[contract.denom]);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strVul(const Format format) const
{
  if (! setVulFlag)
    THROW("Vulnerability not set");

  switch(format)
  {
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return VUL_RBN_TAG[vul];

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strTricks(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      if (! setResultFlag || contract.level == 0)
        return "";
      else
        return "[Result \"" + STR(Contract::getTricks()) + "\"]\n";

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strScorePBN() const
{
  if (! setResultFlag || score == 0)
    return "";

  if (score > 0)
    return "[Score \"NS " + STR(score) + "\"]\n";
  else
    return "[Score \"EW " + STR(-score) + "\"]\n";
}


string Contract::strScorePBN(const int refScore) const
{
  if (! setResultFlag)
    return "";

  stringstream ss;
  ss << Contract::strScorePBN();

  int IMPs = Contract::diffToIMPs(score - refScore);
  if (IMPs > 0)
    ss << "[ScoreIMP \"NS " << IMPs << "\"]\n";
  else if (IMPs == 0)
    ss << "[ScoreIMP \"0\"]\n";
  else
    ss << "[ScoreIMP \"EW " << -IMPs << "\"]\n";
  
  return ss.str();
}


string Contract::strScoreREC() const
{
  stringstream ss;
  if (Contract::isPassedOut())
    ss << "Score: " << setw(13) << "";
  else
    ss << "Score: " << setw(13) << left << showpos << score;
  return ss.str();
}


string Contract::strScoreEML() const
{
  return "Score: " + STR(score) + ",  IMPs:";
}


string Contract::strScoreEML(const int refScore) const
{
  stringstream ss;
  ss << Contract::strScoreEML();

  int IMPs = Contract::diffToIMPs(score - refScore);
  ss << setw(7) << setprecision(2) << fixed << static_cast<double>(IMPs);
  return ss.str();
}


string Contract::strScoreTXT() const
{
  if (! setContractFlag || ! setResultFlag)
    return "";

  return "Score   : " + STR(score) + "\n";
}


string Contract::strScore(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      return Contract::strScorePBN();

    case BRIDGE_FORMAT_TXT:
      return Contract::strScoreTXT();

    case BRIDGE_FORMAT_EML:
      return Contract::strScoreEML();

    case BRIDGE_FORMAT_REC:
      return Contract::strScoreREC();

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strScore(
  const Format format,
  const int refScore) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      return Contract::strScorePBN(refScore);

    case BRIDGE_FORMAT_EML:
      return Contract::strScoreEML(refScore);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strScoreIMP(
  const Format format,
  const int refScore) const
{
  stringstream ss;
  int IMPs;
  switch(format)
  {
    case BRIDGE_FORMAT_REC:
      ss << "Points:";
      IMPs = Contract::diffToIMPs(score - refScore);
      if (Contract::isPassedOut())
        ss << setw(7) << "";
      else if (IMPs == 0)
        ss << setw(7) << right << "=";
      else
        ss << setw(7) << right << showpos << IMPs;
      return ss.str(); 

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strResultPBN() const
{
  stringstream ss;
  if (Contract::isPassedOut())
    ss << "[Result \"\"]\n";
  else
    ss << "[Result \"" << Contract::getTricks() << "\"]\n";

  return ss.str();
}


string Contract::strResultRBNCore() const
{
  stringstream ss;
  if (Contract::isPassedOut())
    return "P";

  ss << Contract::getTricks();
  if (score > 0)
    ss << "+";
  ss << score;
  return ss.str();
}


string Contract::strResultRBNCore(const int refScore) const
{
  stringstream ss;
  ss << Contract::strResultRBNCore() << ":";
  int IMPs = Contract::diffToIMPs(score - refScore);
  if (IMPs > 0)
    ss << "+" << IMPs;
  else if (IMPs == 0)
    ss << "=";
  else
    ss << IMPs;
  return ss.str();
}


string Contract::strResultRBN() const
{
  return "R " + Contract::strResultRBNCore();
}


string Contract::strResultRBN(const int refScore) const
{
  return "R " + Contract::strResultRBNCore(refScore) + "\n";
}


string Contract::strResultRBX() const
{
  return "R{" + Contract::strResultRBNCore() + "}";
}


string Contract::strResultRBX(const int refScore) const
{
  return "R{" + Contract::strResultRBNCore(refScore) + "}";
}


string Contract::strResultTXT() const
{
  if (contract.level == 0)
    return "Passed out";

  stringstream ss;
  if (tricksRelative < 0)
    ss << "Down " << -tricksRelative << " -- ";
  else
    ss << "Made " << 
      static_cast<int>(contract.level) + tricksRelative << " -- ";

  if (score > 0)
    ss << "NS +" << score;
  else
    ss << "EW +" << -score;
  return ss.str();
}


string Contract::strResultTXT(
  const int refScore,
  const string& team) const
{
  stringstream ss;
  ss << Contract::strResultTXT();

  int IMPs = Contract::diffToIMPs(score - refScore);
  if (IMPs == 0)
    ss << " -- Tie";
  else if (IMPs > 0)
    ss << " -- " << team << " +" << IMPs << " IMP";
  else
    ss << " -- " << team << " +" << -IMPs << " IMP";

  if (IMPs != 0 && IMPs != 1 && IMPs != -1)
    ss << "s";
  return ss.str() + "\n";
}


string Contract::strResultEML() const
{
  stringstream ss;
  // Pavlicek bug?
  if (tricksRelative < 0)
    ss << "Result: " << tricksRelative;
  else
    ss << "Result: +" << static_cast<int>(contract.level) + tricksRelative;
  return ss.str();
}


string Contract::strResultREC() const
{
  stringstream ss;
  if (Contract::isPassedOut())
    ss << "Result: Won 32"; // Pavlicek bug
  else if (tricksRelative < 0)
    ss << "Result: Down " << -tricksRelative;
  else
    ss << "Result: Made " << 
      static_cast<int>(contract.level) + tricksRelative;
  return ss.str();
}


string Contract::strResult(const Format format) const
{
  if (! setResultFlag)
    return "";

  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      return Contract::strResultPBN();

    case BRIDGE_FORMAT_RBN:
      return Contract::strResultRBN() + "\n";

    case BRIDGE_FORMAT_RBX:
      return Contract::strResultRBX();

    case BRIDGE_FORMAT_TXT:
      return Contract::strResultTXT() + "\n";

    case BRIDGE_FORMAT_EML:
      return Contract::strResultEML();

    case BRIDGE_FORMAT_REC:
      return Contract::strResultREC();

    case BRIDGE_FORMAT_PAR:
      return STR(Contract::getTricks());

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strResult(
  const Format format,
  const int refScore) const
{
  if (! setResultFlag)
    return "";

  switch(format)
  {
    case BRIDGE_FORMAT_RBN:
      return Contract::strResultRBN(refScore);

    case BRIDGE_FORMAT_RBX:
      return Contract::strResultRBX(refScore);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Contract::strResult(
  const Format format,
  const int refScore,
  const string& team) const
{
  if (! setResultFlag)
    return "";

  switch(format)
  {
    case BRIDGE_FORMAT_TXT:
      return Contract::strResultTXT(refScore, team);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


int Contract::IMPScore(const int refScore) const
{
  return Contract::diffToIMPs(score - refScore);
}

