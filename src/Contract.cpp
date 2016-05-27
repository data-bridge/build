/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Contract.h"
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
  1, 1, 2, 2, 3
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
  { { 110,  470,  640}, { 110,  470,  840} }, // 2M
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
  20, 20, 30, 30, 30 
};

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

	  unsigned lo = 7 - e.contract.level;
	  for (unsigned i = lo; i <= lo+13; i++)
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

  for (unsigned i = 0; i < 20; i++)
    LEVEL_TAG_TO_RELATIVE[LEVEL_SHIFT_TO_TAG[i]] = i-13;
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
  {
    LOG("level must be > 0");
    return false;
  }
  else
  {
    setContractFlag = true;
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
  const string cstring)
{
  if (cstring == "P")
    return Contract::SetPassedOut();
  else
  {
    map<string, entryType>::iterator it = 
      CONTRACT_STRING_TO_PARTS.find(cstring);
    if (it == CONTRACT_STRING_TO_PARTS.end())
    {
      LOG("Invalid string: '" + cstring + "'");
      return false;
    }
    else
    {
      setContractFlag = true;
      vul = vulIn;
      entryType entry = CONTRACT_STRING_TO_PARTS[cstring];
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
}


bool Contract::SetTricks(
  const unsigned tricksIn)
{
  int trel = static_cast<int>(tricksIn) -
    static_cast<int>(contract.level + 6);

  if (setResultFlag)
  {
    return (tricksRelative == trel);
  }
  else if (! setContractFlag)
  {
    LOG("Cannot set tricks before a contract is entered");
    return false;
  }
  else
  {
    setResultFlag = true;
    tricksRelative = trel;
    Contract::CalculateScore();
    return true;
  }
}


void Contract::CalculateScore()
{
  if (! setContractFlag || ! setResultFlag)
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
    if (vulSide)
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


bool Contract::operator == (const Contract& c2) const
{
  if (setContractFlag)
  {
    if (! c2.setContractFlag)
    {
      LOG("First contract is set, second one is not");
      return false;
    }
    else if (contract.level == 0)
    {
      if (c2.contract.level == 0)
        return true;
      else
      {
        LOG("First contract is passed out, second one is not");
	return false;
      }
    }
    else if (vul != c2.vul)
    {
      LOG("Vulnerabilities differ");
      return false;
    }
    else if (contract.declarer != c2.contract.declarer)
    {
      LOG("Declarers differ");
      return false;
    }
    else if (contract.level != c2.contract.level)
    {
      LOG("Levels differ");
      return false;
    }
    else if (contract.denom != c2.contract.denom)
    {
      LOG("Denominations differ");
      return false;
    }
    else if (contract.mult != c2.contract.mult)
    {
      LOG("Multipliers differ");
      return false;
    }
    else if (setResultFlag && ! c2.setResultFlag)
    {
      LOG("First result is set, second one is not");
      return false;
    }
    else if (! setResultFlag && c2.setResultFlag)
    {
      LOG("First result is not set, second one is");
      return false;
    }
    else if (setResultFlag && tricksRelative != c2.tricksRelative)
    {
      LOG("Results differ");
      return false;
    }
    else
      return true;
  }
  else if (! c2.setContractFlag)
    return true;
  else
  {
    LOG("First constract is not set, second one is");
    return false;
  }
}


bool Contract::operator != (const Contract& c2) const
{
  return ! (* this == c2);
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
    return "[Contract \"P\"]\n";

  stringstream s;
  s << "[Contract \"" <<
    contract.level <<
    DENOM_NAMES_SHORT[contract.denom] <<
    MULT_NUM_TO_LIN_TAG[contract.mult] <<
    "\"]\n";

  return s.str();
}


string Contract::AsRBN() const
{
  if (! setContractFlag)
    return "";
  
  if (contract.level == 0)
    return "";

  stringstream s;
  s << "C " <<
    contract.level <<
    DENOM_NAMES_SHORT[contract.denom] <<
    MULT_NUM_TO_RBN_TAG[contract.mult] << 
    ":" <<
    PLAYER_NAMES_SHORT[contract.declarer] << 
    "\n";

  return s.str();
}


string Contract::AsTXT() const
{
  return Contract::AsLIN();
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
      return Contract::AsLIN();

    case BRIDGE_FORMAT_PBN:
      return Contract::AsPBN();

    case BRIDGE_FORMAT_RBN:
      return Contract::AsRBN();

    case BRIDGE_FORMAT_TXT:
      return Contract::AsTXT();

    case BRIDGE_FORMAT_PAR:
      return Contract::AsPar();

    default:
      return "";
  }
}


string Contract::AsTXTBlock() const
{
  if (! setContractFlag)
    return "";

  stringstream s;
  s << "Contract: " << Contract::AsTXT() << "\n";
  if (setResultFlag)
    s << "Score   : " << score << "\n";
  
  return s.str();
}


string Contract::DeclarerAsPBN() const
{
  if (! setResultFlag || contract.level == 0)
    return "";

  stringstream s;
  s << 
    "[Declarer \"" << 
    PLAYER_NAMES_SHORT[contract.declarer] << 
    "\"]\n";
  
  return s.str();
}


string Contract::VulAsRBN() const
{
  return VUL_RBN_TAG[vul];
}


string Contract::TricksAsPBN() const
{
  if (! setResultFlag || contract.level == 0)
    return "";

  stringstream s;
  s << "[Result \"" << contract.level + 6 + tricksRelative << "\"]\n";
  
  return s.str();
}


string Contract::ScoreAsPBN() const
{
  if (! setResultFlag || contract.level == 0)
    return "";

  stringstream s;
  s << "[Score \"" << score << "\"]\n";
  
  return s.str();
}

