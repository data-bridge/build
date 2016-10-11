/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Contract.h"
#include "parse.h"
#include "portab.h"
#include "Bexcept.h"
#include "Bdiff.h"
#include "Debug.h"
#include <map>

extern Debug debug;


const bool VUL_LOOKUP[BRIDGE_VUL_SIZE][BRIDGE_PLAYER_SIZE] =
{
  {false, false, false, false, false, false, false},
  {true, true, true, true, true, true, false},
  {true, false, true, false, true, false, false},
  {false, true, false, true, false, true, false}
};

const string VUL_RBN_TAG[BRIDGE_VUL_SIZE] =
{
  "Z", "B", "N", "E"
};

const int DECLARER_DDS_TO_SIGN[BRIDGE_PLAYER_SIZE] =
{
  1, -1, 1, -1, 1, -1, 0
};

const string LEVEL_SHIFT_TO_TAG[20] =
{
  "-13", "-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", 
  "-3", "-2", "-1", "=", "+1", "+2", "+3", "+4", "+5", "+6"
};

map<string, int> LEVEL_TAG_TO_RELATIVE;

const unsigned DENOM_DDS_TO_CATEGORY[BRIDGE_DENOMS] =
{
  2, 2, 1, 1, 3
};

const string DENOM_DDS_TO_PBN_TAG[BRIDGE_DENOMS] =
{
  "C", "D", "H", "S", "NT"
};

const string DENOM_SUPERSET_TAG[6] =
{
  "C", "D", "H", "S", "N", "NT"
};

const denomType DENOM_SUPERSET_NUM[6] =
{
  BRIDGE_CLUBS, BRIDGE_DIAMONDS, BRIDGE_HEARTS, BRIDGE_SPADES,
  BRIDGE_NOTRUMP, BRIDGE_NOTRUMP
};

const string MULT_NUM_TO_LIN_TAG[3] =
{
  "", "x", "xx"
};

const string MULT_NUM_TO_PBN_TAG[3] =
{
  "", "X", "XX"
};

const string MULT_NUM_TO_RBN_TAG[3] =
{
  "", "X", "R"
};

const string MULT_NUM_TO_PAR_TAG[3] =
{
  "", "*", "**"
};

const string MULT_SUPERSET_TAG[6] =
{
  "", "x", "xx", "X", "XX", "R"
};

const multiplierType MULT_SUPERSET_NUM[6] =
{
  BRIDGE_MULT_UNDOUBLED,
  BRIDGE_MULT_DOUBLED,
  BRIDGE_MULT_REDOUBLED,
  BRIDGE_MULT_DOUBLED,
  BRIDGE_MULT_REDOUBLED,
  BRIDGE_MULT_REDOUBLED
};

const int CONTRACT_SCORES[22][2][3] =
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

const int DENOM_TO_OVERTRICKS_UNDOUBLED[BRIDGE_DENOMS] =
{
  30, 30, 20, 20, 30 
};

const unsigned IMPscale[26] =
{
     0,   10,   40,   80,  120,  160,  210,  260,
   310,  360,  420,  490,  590,  740,  890, 1090,
  1290, 1490, 1740, 1990, 2240, 2490, 2990, 3490,
  3990, 9990
};

unsigned IMPlookup[501];

bool setContractTables = false;

struct entryType
{
  contractType contract;
  int tricksRelative;
};

map<string, entryType> CONTRACT_STRING_TO_PARTS;



Contract::Contract()
{
  Contract::Reset();
  if (! setContractTables)
  {
    setContractTables = true;
    Contract::SetTables();
  }
}


Contract::~Contract()
{
}


void Contract::Reset()
{
  setContractFlag = false;
  setVulFlag = false;
  setResultFlag = false;
}


void Contract::SetTables()
{
  entryType e;
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
	  e.contract.declarer = static_cast<playerType>(decl);
	  stringstream s4;
	  s4 << s3.str() << PLAYER_NAMES_SHORT[decl];

	  // No trick number given, e.g "4HE".
	  e.tricksRelative = 7;
	  CONTRACT_STRING_TO_PARTS[s4.str()] = e;

          // RBN string with ":" and without result.
          stringstream s4a;
          s4a << s3.str() << ":" << PLAYER_NAMES_SHORT[decl];
          CONTRACT_STRING_TO_PARTS[s4a.str()] = e;

	  int lo = 7 - static_cast<int>(e.contract.level);
	  for (int i = lo; i <= lo+13; i++)
	  {
	    stringstream s5;
	    e.tricksRelative = i-13;
	    s5 << s4.str() << LEVEL_SHIFT_TO_TAG[i];
	    CONTRACT_STRING_TO_PARTS[s5.str()] = e;
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


bool Contract::ContractIsSet() const
{
  return setContractFlag;
}


bool Contract::ResultIsSet() const
{
  return setResultFlag;
}


bool Contract::SetPassedOut()
{
  if (setContractFlag)
    return (contract.level == 0);
  else
  {
    setContractFlag = true;
    contract.level = 0;
    setResultFlag = true;
    tricksRelative = 0;
    score = 0;
    return true;
  }
}


bool Contract::SetContract(
  const vulType vulIn,
  const playerType declarer,
  const unsigned level,
  const denomType denom,
  const multiplierType mult)
{
  if (setContractFlag)
  {
    if (contract.level == 0)
      return (contract.level == level);
    else
      return (vul == vulIn &&
        contract.declarer == declarer &&
        contract.level == level &&
	contract.denom == denom &&
	contract.mult == mult);
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
    return true;
  }
}


bool Contract::SetContract(
  const vulType vulIn,
  const string& cstring)
{
  if (! Contract::SetContract(cstring))
    return false;

  setVulFlag = true;
  vul = vulIn;
  return true;
}


bool Contract::SetContract(const string& text)
{
  if (text == "P" || text == "Pass")
    return Contract::SetPassedOut();

  auto it = CONTRACT_STRING_TO_PARTS.find(text);
  if (it == CONTRACT_STRING_TO_PARTS.end())
  {
    THROW("Invalid string: '" + text + "'");
  }
  else
  {
    setContractFlag = true;
    entryType entry = CONTRACT_STRING_TO_PARTS[text];
    contract = entry.contract;
    if (entry.tricksRelative != 7)
    {
      setResultFlag = true;
      tricksRelative = entry.tricksRelative;
      Contract::CalculateScore();
    }

    return true;
  }
}


bool Contract::SetContract(
  const string& text,
  const formatType f)
{
  string mod, wd;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      return Contract::SetContract(text);

    case BRIDGE_FORMAT_TXT:
      if (text.find(" ") == string::npos)
        return Contract::SetContract(text);

      if (! ReadNextWord(text, 0, mod))
        return Contract::SetContract(text);

      if (! ReadLastWord(text, wd))
        return Contract::SetContract(text);

      if (wd == "North")
        mod += "N";
      else if (wd == "East")
        mod += "E";
      else if (wd == "South")
        mod += "S";
      else if (wd == "West")
        mod += "W";
      else 
        return Contract::SetContract(text);

      return Contract::SetContract(mod);

    default:
      THROW("Other score formats not implemented");
  }
}


bool Contract::SetDeclarer(
  const string& d,
  const formatType f)
{
  UNUSED(f);
  if (d == "N")
    contract.declarer = BRIDGE_NORTH;
  else if (d == "E")
    contract.declarer = BRIDGE_EAST;
  else if (d == "S")
    contract.declarer = BRIDGE_SOUTH;
  else if (d == "W")
    contract.declarer = BRIDGE_WEST;
  else
    THROW("Invalid declarer");

  return true;
}


bool Contract::SetVul(
  const vulType v)
{
  if (setVulFlag && vul != v)
    THROW("Vulnerability already set differently");

  vul = v;
  setVulFlag = true;
  return true;
}


bool Contract::SetTricks(
  const unsigned tricksIn)
{
  int trel = static_cast<int>(tricksIn) -
    static_cast<int>(contract.level + 6);

  if (setResultFlag)
    return (tricksRelative == trel);
  else if (! setContractFlag)
    THROW("Cannot set tricks before a contract is entered");
  else
  {
    setResultFlag = true;
    tricksRelative = trel;
    Contract::CalculateScore();
    return true;
  }
}


unsigned Contract::GetTricks() const
{
  // No checking.
  return static_cast<unsigned>
    (tricksRelative + static_cast<int>(contract.level) + 6);
}


bool Contract::SetScore(
  const string& text,
  const formatType f)
{
  int s;
  if (f == BRIDGE_FORMAT_PBN)
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
      if (! StringToInt(pp, s))
        THROW("Invalid score");

      score = sign * s;
      return true;
    }
  }

  if (! StringToInt(text, s))
    THROW("Invalid score");

  score = s;
  return true;
}


bool Contract::SetResult(
  const string& text,
  const formatType f)
{
  // Maybe the usual switch.
  //
  unsigned u;

  if (f == BRIDGE_FORMAT_EML)
  {
    int i;
    if (! StringToInt(text, i))
      THROW("Not an integer result");

    if (i > 0)
      u = static_cast<unsigned>(i + 6); // Possible Pavlicek error?
    else
      u = static_cast<unsigned>(i + static_cast<int>(contract.level + 6));
  }
  else if (f == BRIDGE_FORMAT_TXT ||
      f == BRIDGE_FORMAT_REC)
  {
    // Ignore everything but the number of tricks.
    string wd1, wd2;
    if (! ReadNextWord(text, 0, wd1))
      return false;

    if (! ReadNextWord(text, 5, wd2))
      return false;

    if (! StringToNonzeroUnsigned(wd2, u))
      return false;

    if (wd1 == "Down")
      u = static_cast<unsigned>( static_cast<int>(contract.level + 6 - u));
    else if (wd1 == "Made")
      u += 6;
    else
      return false;
      
  }
  else if ((f == BRIDGE_FORMAT_RBN || f == BRIDGE_FORMAT_RBX) && 
      (text == "P" || text.substr(0, 2) == "P:"))
  {
    return Contract::IsPassedOut();
  }
  else if (! StringToUnsigned(text, u))
    THROW("Not an unsigned result: " + text);

  return Contract::SetTricks(u);
}


void Contract::CalculateScore()
{
  if (! setContractFlag || ! setVulFlag || ! setResultFlag)
  {
    score = -1;
    return;
  }

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


bool Contract::IsPassedOut() const
{
  if (setResultFlag)
    return (contract.level == 0);
  else
    return false;
}


int Contract::GetScore() const
{
  return score;
}


int Contract::ConvertDiffToIMPs(const int d) const
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


playerType Contract::GetDeclarer() const
{
  if (! setContractFlag || contract.level == 0)
    return BRIDGE_NORTH_SOUTH; // Error
  return contract.declarer;
}


denomType Contract::GetDenom() const
{
  if (! setContractFlag || contract.level == 0)
    return BRIDGE_NOTRUMP; // Not recognizable as error!
  return contract.denom;
}


string Contract::AsLIN() const
{
  if (! setContractFlag)
    return "";
  
  if (contract.level == 0)
    return "P";

  stringstream s;
  s << 
    contract.level <<
    DENOM_NAMES_SHORT[contract.denom] <<
    PLAYER_NAMES_SHORT[contract.declarer] << 
    MULT_NUM_TO_LIN_TAG[contract.mult];

  if (setResultFlag)
    s << LEVEL_SHIFT_TO_TAG[tricksRelative + 13];

  return s.str();
}


string Contract::AsPBN() const
{
  if (! setContractFlag)
    return "";
  
  if (contract.level == 0)
    return "[Contract \"Pass\"]\n";

  stringstream s;
  s << "[Contract \"" <<
    contract.level <<
    DENOM_NAMES_SHORT_PBN[contract.denom] <<
    MULT_NUM_TO_PBN_TAG[contract.mult] <<
    "\"]\n";

  return s.str();
}


string Contract::AsRBNCore() const
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


string Contract::AsRBN() const
{
  const string s = Contract::AsRBNCore();
  if (s == "")
    return "";
  return "C " + s + "\n";
}


string Contract::AsRBX() const
{
  const string s = Contract::AsRBNCore();
  if (s == "")
    return "";
  return "C{" + s + "}";
}


string Contract::AsTXT() const
{
  if (! setContractFlag)
    return "";
  
  if (contract.level == 0)
    return "";

  stringstream s;
  s << contract.level << 
    DENOM_NAMES_SHORT_PBN[contract.denom] << 
    MULT_NUM_TO_LIN_TAG[contract.mult] << " " <<
    PLAYER_NAMES_LONG[contract.declarer] << "\n";

  return s.str();
}


string Contract::AsPar() const
{
  if (! setContractFlag || ! setResultFlag)
    return "";
  
  if (contract.level == 0)
    return "pass";

  stringstream s;
  s << 
    contract.level <<
    DENOM_NAMES_SHORT[contract.denom] <<
    MULT_NUM_TO_PAR_TAG[contract.mult] << 
    "-" <<
    PLAYER_NAMES_SHORT[contract.declarer] << 
    LEVEL_SHIFT_TO_TAG[tricksRelative + 13];

  return s.str();
}


string Contract::AsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return Contract::AsLIN();

    case BRIDGE_FORMAT_PBN:
      return Contract::AsPBN();

    case BRIDGE_FORMAT_RBN:
      return Contract::AsRBN();

    case BRIDGE_FORMAT_RBX:
      return Contract::AsRBX();

    case BRIDGE_FORMAT_TXT:
      return Contract::AsTXT();

    case BRIDGE_FORMAT_PAR:
      return Contract::AsPar();

    case BRIDGE_FORMAT_SIZE:
    default:
      return "";
  }
}


string Contract::DeclarerAsPBN() const
{
  if (! setResultFlag)
    return "";
  else if (contract.level == 0)
    return "[Declarer \"\"]\n";

  stringstream s;
  s << 
    "[Declarer \"" << 
    PLAYER_NAMES_SHORT[contract.declarer] << 
    "\"]\n";
  
  return s.str();
}


string Contract::DeclarerAsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_PBN:
      return Contract::DeclarerAsPBN();

    default:
      THROW("Other declarer formats not implemented");
      return "";
  }
}


string Contract::VulAsRBN() const
{
  return VUL_RBN_TAG[vul];
}


string Contract::VulAsString(const formatType f) const
{
  if (! setVulFlag)
    THROW("Vulnerability not yet");

  switch(f)
  {
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return Contract::VulAsRBN();

    default:
      THROW("Other vulnerability formats not implemented");
  }
}


string Contract::TricksAsPBN() const
{
  if (! setResultFlag || contract.level == 0)
    return "";

  stringstream s;
  s << "[Result \"" << Contract::GetTricks() << "\"]\n";
  
  return s.str();
}


string Contract::TricksAsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_PBN:
      return Contract::TricksAsPBN();

    default:
      THROW("Other tricks formats not implemented");
      return "";
  }
}


string Contract::ScoreAsPBN() const
{
  if (! setResultFlag || score == 0)
    return "";

  stringstream s;
  if (score > 0)
    s << "[Score \"NS " << score << "\"]\n";
  else
    s << "[Score \"EW " << -score << "\"]\n";
  
  return s.str();
}


string Contract::ScoreAsPBN(const int refScore) const
{
  stringstream s;
  s << Contract::ScoreAsPBN();

  int IMPs = Contract::ConvertDiffToIMPs(score - refScore);
  if (IMPs > 0)
    s << "[ScoreIMP \"NS " << IMPs << "\"]\n";
  else if (IMPs == 0)
    s << "[ScoreIMP \"0\"]\n";
  else
    s << "[ScoreIMP \"EW " << -IMPs << "\"]\n";
  
  return s.str();
}


string Contract::ScoreAsEML() const
{
  stringstream s;
  s << "Score: " << score << ",  IMPs:";
  return s.str();
}


string Contract::ScoreAsEML(const int refScore) const
{
  stringstream s;
  s << Contract::ScoreAsEML();

  int IMPs = Contract::ConvertDiffToIMPs(score - refScore);
  s << setw(7) << setprecision(2) << fixed << static_cast<double>(IMPs);
  return s.str();
}


string Contract::ScoreAsREC() const
{
  stringstream s;
  s << "Score: " << setw(13) << left << showpos << score;
  return s.str();
}


string Contract::ScoreAsTXT() const
{
  if (! setContractFlag || ! setResultFlag)
    return "";

  stringstream s;
  s << "Score   : " << score << "\n";
  return s.str();
}


string Contract::ScoreAsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_PBN:
      return Contract::ScoreAsPBN();

    case BRIDGE_FORMAT_EML:
      return Contract::ScoreAsEML();

    case BRIDGE_FORMAT_TXT:
      return Contract::ScoreAsTXT();

    case BRIDGE_FORMAT_REC:
      return Contract::ScoreAsREC();

    default:
      THROW("Other score formats not implemented");
  }
}


string Contract::ScoreIMPAsREC(const int refScore) const
{
  stringstream s;
  s << "Points:";
  int IMPs = Contract::ConvertDiffToIMPs(score - refScore);
  if (IMPs == 0)
    s << setw(7) << right << "=";
  else
    s << setw(7) << right << showpos << IMPs;
  return s.str(); 
}


string Contract::ScoreIMPAsString(
  const formatType f,
  const int refScore) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_REC:
      return Contract::ScoreIMPAsREC(refScore);

    default:
      THROW("Other score formats not implemented");
  }
}


int Contract::ScoreIMPAsInt(const int refScore) const
{
  return Contract::ConvertDiffToIMPs(score - refScore);
}


string Contract::ScoreAsString(
  const formatType f,
  const int refScore) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_PBN:
      return Contract::ScoreAsPBN(refScore);

    case BRIDGE_FORMAT_EML:
      return Contract::ScoreAsEML(refScore);

    default:
      THROW("Other score formats not implemented");
  }
}


string Contract::ResultAsStringPBN() const
{
  stringstream s;
  if (Contract::IsPassedOut())
    s << "[Result \"\"]\n";
  else
    s << "[Result \"" << Contract::GetTricks() << "\"]\n";

  return s.str();
}


string Contract::ResultAsStringRBNCore() const
{
  stringstream s;
  if (Contract::IsPassedOut())
    return "P";

  s << Contract::GetTricks();
  if (score > 0)
    s << "+";
  s << score;
  return s.str();
}


string Contract::ResultAsStringRBN() const
{
  return "R " + Contract::ResultAsStringRBNCore();
}


string Contract::ResultAsStringRBX() const
{
  return "R{" + Contract::ResultAsStringRBNCore() + "}";
}


string Contract::ResultAsStringRBNCore(const int refScore) const
{
  stringstream s;
  s << Contract::ResultAsStringRBNCore() << ":";
  int IMPs = Contract::ConvertDiffToIMPs(score - refScore);
  if (IMPs > 0)
    s << "+" << IMPs;
  else if (IMPs == 0)
    s << "=";
  else
    s << IMPs;
  return s.str();
}


string Contract::ResultAsStringRBN(const int refScore) const
{
  return "R " + Contract::ResultAsStringRBNCore(refScore) + "\n";
}


string Contract::ResultAsStringRBX(const int refScore) const
{
  return "R{" + Contract::ResultAsStringRBNCore(refScore) + "}";
}


string Contract::ResultAsStringEML() const
{
  stringstream s;
  // Pavlicek bug?
  if (tricksRelative < 0)
    s << "Result: " << tricksRelative;
  else
    s << "Result: +" << 
      static_cast<int>(contract.level) + tricksRelative;
  return s.str();
}


string Contract::ResultAsStringTXT() const
{
  if (contract.level == 0)
    return "Passed out";

  stringstream s;
  if (tricksRelative < 0)
    s << "Down " << -tricksRelative << " -- ";
  else
    s << "Made " << 
      static_cast<int>(contract.level) + tricksRelative << " -- ";

  if (score > 0)
    s << "NS +" << score;
  else
    s << "EW +" << -score;
  return s.str();
}


string Contract::ResultAsStringTXT(
  const int refScore,
  const string& team) const
{
  stringstream s;
  s << Contract::ResultAsStringTXT();

  int IMPs = Contract::ConvertDiffToIMPs(score - refScore);
  if (IMPs == 0)
    s << " -- Tie";
  else if (IMPs > 0)
    s << " -- " << team << " +" << IMPs << " IMP";
  else
    s << " -- " << team << " +" << -IMPs << " IMP";

  if (IMPs != 0 && IMPs != 1 && IMPs != -1)
    s << "s";
  return s.str() + "\n";
}


string Contract::ResultAsStringREC() const
{
  stringstream s;
  if (tricksRelative < 0)
    s << "Result: Down " << -tricksRelative;
  else
    s << "Result: Made " << 
      static_cast<int>(contract.level) + tricksRelative;
  return s.str();
}


string Contract::ResultAsString(const formatType f) const
{
  if (! setResultFlag)
    return "";

  switch(f)
  {
    case BRIDGE_FORMAT_PBN:
      return Contract::ResultAsStringPBN();

    case BRIDGE_FORMAT_RBN:
      return Contract::ResultAsStringRBN() + "\n";

    case BRIDGE_FORMAT_RBX:
      return Contract::ResultAsStringRBX();

    case BRIDGE_FORMAT_EML:
      return Contract::ResultAsStringEML();

    case BRIDGE_FORMAT_TXT:
      return Contract::ResultAsStringTXT() + "\n";

    case BRIDGE_FORMAT_REC:
      return Contract::ResultAsStringREC();

    default:
      THROW("Other score formats not implemented");
  }
}


string Contract::ResultAsString(
  const formatType f,
  const int refScore) const
{
  if (! setResultFlag)
    return "";

  switch(f)
  {
    case BRIDGE_FORMAT_RBN:
      return Contract::ResultAsStringRBN(refScore);

    case BRIDGE_FORMAT_RBX:
      return Contract::ResultAsStringRBX(refScore);

    default:
      THROW("Other score formats not implemented");
      return "";
  }
}


string Contract::ResultAsString(
  const formatType f,
  const int refScore,
  const string& team) const
{
  if (! setResultFlag)
    return "";

  switch(f)
  {
    case BRIDGE_FORMAT_TXT:
      return Contract::ResultAsStringTXT(refScore, team);

    default:
      THROW("Other score formats not implemented");
      return "";
  }
}

