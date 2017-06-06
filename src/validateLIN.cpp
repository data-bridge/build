/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <regex>

#include "Deal.h"
#include "valint.h"
#include "ValProfile.h"
#include "validate.h"
#include "validateLIN.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"

#define PLOG(x) prof.log(x, valState.dataOut, valState.dataRef)


enum FixEntry
{
  VAL_LIN_NONE = 0,
  VAL_LIN_REF = 1,
  VAL_LIN_OUT = 2
};

enum FixTag
{
  VAL_LIN_AH = 0,
  VAL_LIN_AN = 1,
  VAL_LIN_BN = 2,
  VAL_LIN_MB = 3,
  VAL_LIN_MC = 4,
  VAL_LIN_MD = 5,
  VAL_LIN_MP = 6,
  VAL_LIN_PC = 7,
  VAL_LIN_PN = 8,
  VAL_LIN_PW = 9,
  VAL_LIN_QX = 10,
  VAL_LIN_RH = 11,
  VAL_LIN_RS = 12,
  VAL_LIN_ST = 13,
  VAL_LIN_SV = 14,
  VAL_LIN_VG = 15,
  VAL_LIN_FIX_SIZE = 16
};

struct FixInfo
{
  bool valFlag;
  string val;
  FixEntry advancer;
  ValError error;
};

static map<string, FixTag> tagMap;

FixInfo fixTable[VAL_LIN_FIX_SIZE][VAL_LIN_FIX_SIZE];

typedef bool (*ValFncPtr)(const string&, const string&);

struct ValLINEntry
{
  bool set;
  ValError error;
  ValFncPtr fptr;
};

ValLINEntry valLINPtr[VAL_LIN_FIX_SIZE];

static bool isDeal(const string& valueRef, const string& valueOut);
static bool isPlayersList(const string& valueRef, const string& valueOut);
static bool isBoard(const string& valueRef, const string& valueOut);
static bool isContracts(const string& valueRef, const string& valueOut);
static bool isVul(const string& valueRef, const string& valueOut);
static bool isTitleBoards(const string& valueRef, const string& valueOut);

// Initialization functions

void setValidateLINTables()
{
  for (size_t i = 0; i < VAL_LIN_FIX_SIZE; i++)
    valLINPtr[i].set = false;

  valLINPtr[VAL_LIN_MD].set = true;
  valLINPtr[VAL_LIN_MD].error = BRIDGE_VAL_LIN_MD;
  valLINPtr[VAL_LIN_MD].fptr = &isDeal;

  valLINPtr[VAL_LIN_PN].set = true;
  valLINPtr[VAL_LIN_PN].error = BRIDGE_VAL_PLAYERS_HEADER;
  valLINPtr[VAL_LIN_PN].fptr = &isPlayersList;

  valLINPtr[VAL_LIN_PW].set = true;
  valLINPtr[VAL_LIN_PW].error = BRIDGE_VAL_PLAYERS_HEADER;
  valLINPtr[VAL_LIN_PW].fptr = &isPlayersList;

  valLINPtr[VAL_LIN_QX].set = true;
  valLINPtr[VAL_LIN_QX].error = BRIDGE_VAL_LIN_QX;
  valLINPtr[VAL_LIN_QX].fptr = &isBoard;

  valLINPtr[VAL_LIN_RS].set = true;
  valLINPtr[VAL_LIN_RS].error = BRIDGE_VAL_BOARDS_HEADER;
  valLINPtr[VAL_LIN_RS].fptr = &isContracts;

  valLINPtr[VAL_LIN_SV].set = true;
  valLINPtr[VAL_LIN_SV].error = BRIDGE_VAL_VUL;
  valLINPtr[VAL_LIN_SV].fptr = &isVul;

  valLINPtr[VAL_LIN_VG].set = true;
  valLINPtr[VAL_LIN_VG].error = BRIDGE_VAL_TITLE;
  valLINPtr[VAL_LIN_VG].fptr = &isTitleBoards;

  tagMap["ah"] = VAL_LIN_AH;
  tagMap["an"] = VAL_LIN_AN;
  tagMap["bn"] = VAL_LIN_BN;
  tagMap["mb"] = VAL_LIN_MB;
  tagMap["mc"] = VAL_LIN_MC;
  tagMap["md"] = VAL_LIN_MD;
  tagMap["mp"] = VAL_LIN_MP;
  tagMap["pc"] = VAL_LIN_PC;
  tagMap["pn"] = VAL_LIN_PN;
  tagMap["pw"] = VAL_LIN_PW;
  tagMap["qx"] = VAL_LIN_QX;
  tagMap["rh"] = VAL_LIN_RH;
  tagMap["rs"] = VAL_LIN_RS;
  tagMap["st"] = VAL_LIN_ST;
  tagMap["sv"] = VAL_LIN_SV;
  tagMap["vg"] = VAL_LIN_VG;

  for (size_t i = 0; i < VAL_LIN_FIX_SIZE; i++)
  {
    for (size_t j = 0; j < VAL_LIN_FIX_SIZE; j++)
    {
      fixTable[i][j].valFlag = false;
      fixTable[i][j].advancer = VAL_LIN_NONE;
      fixTable[i][j].error = BRIDGE_VAL_SIZE;
    }
  }

  fixTable[VAL_LIN_AH][VAL_LIN_ST].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_AH][VAL_LIN_ST].error = BRIDGE_VAL_LIN_AH_EXTRA;

  fixTable[VAL_LIN_AH][VAL_LIN_SV].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_AH][VAL_LIN_SV].error = BRIDGE_VAL_LIN_AH_EXTRA;

  // Probably the previous values were mb|bid!|, but ref has a 
  // trailing an|!| which we must get rid of.
  fixTable[VAL_LIN_AN][VAL_LIN_MB].valFlag = true;
  fixTable[VAL_LIN_AN][VAL_LIN_MB].val = "!";
  fixTable[VAL_LIN_AN][VAL_LIN_MB].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_AN][VAL_LIN_MB].error = BRIDGE_VAL_LIN_AN_EXTRA;

  fixTable[VAL_LIN_BN][VAL_LIN_QX].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_BN][VAL_LIN_QX].error = BRIDGE_VAL_BOARDS_HEADER;

  fixTable[VAL_LIN_MB][VAL_LIN_SV].advancer = VAL_LIN_OUT;
  fixTable[VAL_LIN_MB][VAL_LIN_SV].error = BRIDGE_VAL_LIN_SV_MISSING;

  // Assume the global pw takes care of this.
  fixTable[VAL_LIN_MC][VAL_LIN_PN].advancer = VAL_LIN_OUT;
  fixTable[VAL_LIN_MC][VAL_LIN_PN].error = BRIDGE_VAL_LIN_PN_MISSING;

  // Assume the global pw takes care of this.
  fixTable[VAL_LIN_MC][VAL_LIN_QX].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_MC][VAL_LIN_QX].error = BRIDGE_VAL_LIN_MC_EXTRA;

  fixTable[VAL_LIN_MD][VAL_LIN_ST].advancer = VAL_LIN_OUT;
  fixTable[VAL_LIN_MD][VAL_LIN_ST].error = BRIDGE_VAL_LIN_ST_MISSING;

  fixTable[VAL_LIN_MP][VAL_LIN_QX].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_MP][VAL_LIN_QX].error = BRIDGE_VAL_SCORES_HEADER;

  fixTable[VAL_LIN_QX][VAL_LIN_SV].advancer = VAL_LIN_OUT;
  fixTable[VAL_LIN_QX][VAL_LIN_SV].error = BRIDGE_VAL_LIN_SV_MISSING;

  fixTable[VAL_LIN_PN][VAL_LIN_MB].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_PN][VAL_LIN_MB].error = BRIDGE_VAL_LIN_PN_EXTRA;

  fixTable[VAL_LIN_PN][VAL_LIN_MD].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_PN][VAL_LIN_MD].error = BRIDGE_VAL_LIN_PN_EXTRA;

  fixTable[VAL_LIN_PN][VAL_LIN_QX].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_PN][VAL_LIN_QX].error = BRIDGE_VAL_LIN_PN_EXTRA;

  // Could be pn|...| embedded in qx|o1|, which we choose to
  // disregard, as it is hopefully consistent with the pn header.
  fixTable[VAL_LIN_PN][VAL_LIN_ST].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_PN][VAL_LIN_ST].error = BRIDGE_VAL_LIN_ST_EXTRA;

  fixTable[VAL_LIN_RH][VAL_LIN_ST].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_RH][VAL_LIN_ST].error = BRIDGE_VAL_LIN_RH_EXTRA;

  fixTable[VAL_LIN_RH][VAL_LIN_SV].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_RH][VAL_LIN_SV].error = BRIDGE_VAL_LIN_RH_EXTRA;

  fixTable[VAL_LIN_ST][VAL_LIN_MB].advancer = VAL_LIN_REF;
  fixTable[VAL_LIN_ST][VAL_LIN_MB].error = BRIDGE_VAL_LIN_ST_EXTRA;
}


// General functions

static FixTag str2tag(const string& str)
{
  auto it = tagMap.find(str);
  if (it == tagMap.end())
  {
    string strl = str;
    toLower(strl);
    it = tagMap.find(strl);
    if (it == tagMap.end())
      return VAL_LIN_FIX_SIZE;
  }
  return it->second;
}


static bool firstContainsSecondLIN(
  const LineData& first,
  const LineData& second,
  string& expectLine)
{
  if ((first.label != "mb" && first.label != "pf") || 
      first.label != second.label)
    return false;

  if (firstContainsSecond(first, second))
  {
    expectLine = first.line.substr(second.len);
    return true;
  }

  // If we can lop off "pg||" at the end, try again.
  if (second.line.substr(second.len-4) != "pg||")
    return false;

  if (second.line.substr(0, second.len-4) == 
      first.line.substr(0, second.len-4))
  {
    expectLine = first.line.substr(second.len-4);
    return true;
  }
  else
    return false;
}


static bool LINtoList(
  const string& line,
  vector<string>& list,
  const int numFields)
{
  // A LIN vg line must have exactly 8 commas.
  // For a pn line it is 7.
  if (count(line.begin(), line.end(), ',') != numFields)
    return false;

  list.clear();
  if (line.length() >= 5 && line.substr(line.length()-5) == "|pg||")
    tokenize(line.substr(0, line.length()-5), list, ",");
  else
    tokenize(line, list, ",");
  return true;
}


static string pruneCommas(
  const string& text,
  unsigned& p,
  unsigned& q)
{
  p = 0;
  const unsigned l = static_cast<unsigned>(text.length());
  while (p < l && text.at(p) == ',')
    p++;

  if (p >= l)
    return "";

  q = l;
  while (q >= 1 && text.at(q-1) == ',')
    q--;

  // Trailing commas.
  if (q <= p)
    return "";

  return text.substr(p, q-p);
}


static void collapseList(
  const vector<string>& in,
  vector<string>& out)
{
  for (unsigned i = 0; i < 4; i++)
    out.push_back(in[i]);

  const unsigned l = in.size();
  for (unsigned i = 4; i+3 < l; i += 4)
  {
    if (in[i] != in[i-4] ||
        in[i+1] != in[i-3] ||
        in[i+2] != in[i-2] ||
        in[i+3] != in[i-1])
    {
      out.push_back(in[i]);
      out.push_back(in[i+1]);
      out.push_back(in[i+2]);
      out.push_back(in[i+3]);
    }
  }

  // Stragglers.
  for (unsigned i = (l & 0xfffc); i < l; i++)
    out.push_back(in[i]);
}


static void despaceList(
  const vector<string>& in,
  vector<string>& out)
{
  const unsigned l = in.size();
  unsigned start;
  for (unsigned i = 0; i+3 < l; i += 4)
  {
    if (in[i] != "South" || in[i+1] != "West" || 
        in[i+2] != "North" || in[i+3] != "East")
    {
      out.push_back(in[i]);
      out.push_back(in[i+1]);
      out.push_back(in[i+2]);
      out.push_back(in[i+3]);
      start = i+4;
      break;
    }
  }

  if (out.size() == 0)
    return;

  unsigned ol = 4;
  for (unsigned i = start; i+3 < l; i += 4)
  {
    if (in[i] != out[ol-4] ||
        in[i+1] != out[ol-3] ||
        in[i+2] != out[ol-2] ||
        in[i+3] != out[ol-1])
    {
      if (in[i] == "" && in[i+1] == "" && in[i+2] == "" && in[i+3] == "")
        continue;

      if (in[i] == "South" && in[i+1] == "West" && 
          in[i+2] == "North" && in[i+3] == "East")
        continue;

      for (unsigned j = 0; j < 4; j++)
      {
        if (in[i+j] == "")
          out.push_back(out[ol+j-4]);
        else
          out.push_back(in[i+j]);
      }
      ol += 4;
    }
  }

  // Stragglers.
  for (unsigned i = (l & 0xfffc); i < l; i++)
    out.push_back(in[i]);
}


static bool isCompactSequence(ValState& valState)
{
  string refSeq = valState.dataRef.value;
  trimLeading(refSeq, '-');
  refSeq = trimTrailing(refSeq, '-');
  toUpper(refSeq);

  if (! valState.bufferRef.next(valState.dataRef))
    return false;

  string refOut = valState.dataOut.value;
  while (valState.bufferOut.next(valState.dataOut) &&
      valState.dataOut.label == "mb")
  {
    refOut += valState.dataOut.value;
  }

  if (valState.bufferOut.peek() == 0x00)
    return false;

  toUpper(refOut);
  return (refSeq == refOut);
}


static bool isDifferentCase(
  const string& value1,
  const string& value2)
{
  string v1 = value1;
  string v2 = value2;
  toUpper(v1);
  toUpper(v2);
  return (v1 == v2);
}


// RP functions

static bool isLINHeaderLine(
  const ValState& valState,
  ValProfile& prof)
{
  if (valState.dataRef.label != "vg" ||
      valState.dataOut.label != "vg")
    return false;

  vector<string> vOut(9), vRef(9);
  if (! LINtoList(valState.dataOut.value, vOut, 8))
    return false;
  if (! LINtoList(valState.dataRef.value, vRef, 8))
    return false;

  if (vOut[0] != vRef[0])
    PLOG(BRIDGE_VAL_TITLE);

  if (vOut[1] != vRef[1])
    PLOG(BRIDGE_VAL_SESSION);

  if (vOut[2] != vRef[2])
    PLOG(BRIDGE_VAL_SCORING);

  if (vOut[3] != vRef[3] || vOut[4] != vRef[4])
    PLOG(BRIDGE_VAL_SCORING);

  if (vOut[5] != vRef[5] || vOut[6] != vRef[6] ||
      vOut[7] != vRef[7] || vOut[8] != vRef[8])
    PLOG(BRIDGE_VAL_TEAMS);

  return true;
}


static bool isLINPlayerLine(
  const ValState& valState,
  ValProfile& prof)
{
  if (valState.dataRef.label != "pn" ||
      valState.dataOut.label != "pn")
    return false;

  vector<string> vOut(8), vRef(8);
  if (! LINtoList(valState.dataOut.value, vOut, 7))
    return false;
  if (! LINtoList(valState.dataRef.value, vRef, 7))
    return false;

  for (unsigned i = 0; i < 8; i++)
  {
    if (vOut[i] == vRef[i])
      continue;

    if (firstContainsSecond(vRef[i], vOut[i]))
      PLOG(BRIDGE_VAL_NAMES_SHORT);
    else
      return false;
  }
  return true;
}


static bool isLINPlayLine(
  const ValState& valState,
  ValProfile& prof)
{
  if (valState.dataRef.label != "pc" ||
      valState.dataOut.label != "pc")
    return false;

  vector<string> vOut(1), vRef(1);
  if (! LINtoList(valState.dataOut.value, vOut, 0))
    return false;
  if (! LINtoList(valState.dataRef.value, vRef, 0))
    return false;

  if (firstContainsSecond(vRef[0], vOut[0]))
  {
    PLOG(BRIDGE_VAL_PLAY_SHORT);
    return true;
  }
  else
    return false;
}


bool validateLIN_RP(
  ValState& valState,
  ValProfile& prof)
{
  if (valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED ||
      valState.dataOut.type != BRIDGE_BUFFER_STRUCTURED)
    return false;

  string expectLine;
  if (firstContainsSecondLIN(valState.dataRef, valState.dataOut, 
      expectLine))
  {
    if (! valState.bufferOut.next(valState.dataOut))
      return false;

    if (valState.dataOut.line == expectLine)
    {
      // No newline when no play (Pavlicek error).
      PLOG(BRIDGE_VAL_LIN_PLAY_NL);
      return true;
    }
    else
      return false;
  }

  if (firstContainsSecondLIN(valState.dataOut, valState.dataRef, 
      expectLine))
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.line == expectLine)
    {
      // No newline when no play (Pavlicek error).
      PLOG(BRIDGE_VAL_LIN_PLAY_NL);
      return true;
    }
    else
      return false;
  }

  if (isLINHeaderLine(valState, prof))
    return true;

  if (isLINPlayerLine(valState, prof))
    return true;

  if (isLINPlayLine(valState, prof))
    return true;

  if (valState.dataOut.label == "pc")
    return false;

  while (valState.dataRef.label == "pc")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;
  }

  if (valState.dataOut.line == valState.dataRef.line)
  {
    PLOG(BRIDGE_VAL_PLAY_SHORT);
    return true;
  }
  else
    return false;
}


// Other LIN-type functions

static bool isTitleBoards(
  const string& valueRef,
  const string& valueOut)
{
  vector<string> listRef(9), listOut(9);
  if (! LINtoList(valueRef, listRef, 8))
    return false;
  if (! LINtoList(valueOut, listOut, 8))
    return false;

  for (unsigned i = 0; i < 9; i++)
  {
    if (i == 3 || i == 4)
      continue;
    if (listRef[i] != listOut[i])
    {
      if ((i == 6 || i == 8) && listRef[i] == "0" && listOut[i] == "")
      {
        // Permit.
      }
      else
        return false;
    }
  }
  return true;
}


static bool isPlayersList(
  const string& valueRef,
  const string& valueOut)
{
  unsigned p, q;
  const string refPruned = pruneCommas(valueRef, p, q);
  if (refPruned == "")
    return false;

  if (valueOut == refPruned)
    return true;

  unsigned p1, q1;
  const string outPruned = pruneCommas(valueOut, p1, q1);

  if (outPruned == refPruned)
    return true;

  const int refCommas = count(refPruned.begin(), refPruned.end(), ',');
  const int outCommas = count(outPruned.begin(), outPruned.end(), ',');

  if (refCommas < 3 || outCommas < 3)
    return false;

  vector<string> refList(static_cast<unsigned>(refCommas+1));
  vector<string> outList(static_cast<unsigned>(outCommas+1));
  if (! LINtoList(refPruned, refList, refCommas))
    return false;

  if (! LINtoList(outPruned, outList, outCommas))
    return false;

  vector<string> refCollapsed;
  vector<string> outCollapsed;

  collapseList(refList, refCollapsed);
  collapseList(outList, outCollapsed);

  const unsigned lRef = refCollapsed.size();
  const unsigned lOut = outCollapsed.size();
  if (lRef == lOut)
  {
    for (unsigned i = 0; i < lRef; i++)
    {
      if (refCollapsed[i] != outCollapsed[i] &&
          refCollapsed[i] != "" &&
          outCollapsed[i] != "")
        return false;
    }
    return true;
  }

  vector<string> refDespaced;
  vector<string> outDespaced;

  despaceList(refCollapsed, refDespaced);
  despaceList(outCollapsed, outDespaced);

  const unsigned nRef = refDespaced.size();
  const unsigned nOut = outDespaced.size();
  if (nRef != nOut)
    return false;

  for (unsigned i = 0; i < nRef; i++)
  {
    if (refDespaced[i] != outDespaced[i])
    {
      for (unsigned j = 0; j < nRef; j++)
      {
        cout << j << ": ref " << refDespaced[j] << " out " <<
          outDespaced[j] << "\n";
      }
      cout << "First diff: " << i << "\n";
      return false;
    }
  }
  return true;
}


static bool isContracts(
  const string& valueRef,
  const string& valueOut)
{
  unsigned p, q;
  const string refPruned = pruneCommas(valueRef, p, q);
  if (refPruned == "")
    return false;

  if (valueOut == refPruned)
    return true;

  unsigned p1, q1;
  const string outPruned = pruneCommas(valueOut, p1, q1);

  if (outPruned == refPruned)
    return true;

  regex reu("\\bP\\b");
  string refPass = regex_replace(refPruned, reu, string("PASS"));
  regex rel("\\bp\\b");
  refPass = regex_replace(refPass, rel, string("PASS"));
  regex rep("\\bPass\\b");
  refPass = regex_replace(refPass, rep, string("PASS"));

  if (outPruned == refPass)
    return true;

  // Tolerate single stray comma in reference.
  const unsigned lr = refPass.length();
  if (outPruned.length()+1 == lr &&
      refPass.at(lr-1) == ',' &&
      outPruned == refPass.substr(0, lr-1))
    return true;
  else
    return false;
}


static bool isBoard(
  const string& valueRef,
  const string& valueOut)
{
  if (valueRef == valueOut)
    return true;

  const unsigned lr = valueRef.length();
  const unsigned lo = valueOut.length();

  if (lo+8 > lr || valueRef.substr(0, lo) != valueOut)
    return false;

  if (valueRef.substr(lo, 7) != ",BOARD ")
    return false;
  else
    return true;
}


static bool isDeal(
  const string& valueRef,
  const string& valueOut)
{
  const unsigned lr = valueRef.length();
  const unsigned lo = valueOut.length();

  if (lr == 54 && lo == 72 && valueOut.substr(0, lr-4) == valueRef)
  {
    return true;
  }
  else if ((lr == 54 || lr == 55 || (lr >= 69 && lr <= 72)) && lo == 72)
  {
    Deal dealRef, dealOut;
    try
    {
      dealRef.set(valueRef, BRIDGE_FORMAT_LIN);
      dealOut.set(valueOut, BRIDGE_FORMAT_LIN);

      return (dealRef == dealOut);
    }
    catch (Bexcept& bex)
    {
      UNUSED(bex);
      return false;
    }
    catch (Bdiff& bdiff)
    {
      UNUSED(bdiff);
      return false;
    }
  }
  else
    return false;
}


static bool isVul(
  const string& valueRef,
  const string& valueOut)
{
  if ((valueRef == "0" && valueOut == "o") ||
      (valueRef == "B" && valueOut == "b") ||
      (valueRef == "N" && valueOut == "n") ||
      (valueRef == "E" && valueOut == "e"))
    return true;
  else
    return false;
}


static bool isRotatedPlay(
  ValState& valState,
  ValProfile& prof)
{
  UNUSED(prof);
  vector<string> playRef, playOut;

  do
  {
    playRef.clear();
    playOut.clear();

    bool doneRef = false, doneOut = false;
    do
    {
      playRef.push_back(valState.dataRef.value);
      if (! valState.bufferRef.next(valState.dataRef))
      {
        doneRef = true;
        break;
      }
    }
    while (valState.dataRef.label == "pc" && playRef.size() < 4);

    do
    {
      playOut.push_back(valState.dataOut.value);
      if (! valState.bufferOut.next(valState.dataOut))
      {
        doneOut = true;
        break;
      }
    }
    while (valState.dataOut.label == "pc" && playOut.size() < 4);

    if (playRef.size() != playOut.size())
      return false;

    const unsigned ps = playRef.size();
    unsigned offset = 0;
    while (offset < ps && ! isDifferentCase(playOut[offset], playRef[0]))
      offset++;

    if (offset == ps)
      return false;

    for (unsigned i = 1; i < ps; i++)
    {
      if (! isDifferentCase(playRef[i], playOut[(offset+i) % ps]))
        return false;
    }

    if (offset == 0 &&
        valState.dataRef.label == valState.dataOut.label &&
        isDifferentCase(valState.dataRef.value, valState.dataOut.value))
      return true;
    
    if (doneRef && doneOut)
      return true;
  }
  while (valState.dataRef.label == "pc" &&
      valState.dataOut.label == "pc");

  if (valState.dataRef.label == valState.dataOut.label &&
      isDifferentCase(valState.dataRef.value, valState.dataOut.value))
    return true;
  else if (valState.dataRef.label == "mc" &&
      valState.dataOut.label == "qx")
  {
    // Would be nice if this went back into the global loop.
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label == "qx")
    {
      PLOG(BRIDGE_VAL_LIN_MC_EXTRA);
      return (valState.dataRef.value == valState.dataOut.value);
    }
    else
      return false;
  }
  else
    return false;
}


static bool isXDouble(
  const string& valueRef,
  const string& valueOut)
{
  if (valueRef == "X" || valueRef == "x")
  {
    return (valueOut == "D");
  }
  else if (valueRef == "X!" || valueRef == "x!")
  {
    return (valueOut == "D!");
  }
  else
  {
    return isDifferentCase(valueRef, valueOut);
  }
}


static bool isCall(
  const string& valueRef,
  const string& valueOut)
{
  const unsigned lr = valueRef.length();
  const unsigned lo = valueOut.length();

  if ((lr == 2 || lr == 3) && lo+1 == lr && valueRef.at(lo) == '!')
  {
    // Output is already in upper case, as we made it.
    // ref mb|2C!| vs out mb|2C|?
    string ur = valueRef.substr(0, lo);
    toUpper(ur);
    return (ur == valueOut);
  }
  else if (lr+1 == lo && (lo == 2 || lo == 3) && valueOut.at(lr) == '!')
  {
    // ref mb|2C|an|!| vs out mb|2C!|?
    string ur = valueRef;
    toUpper(ur);
    return (valueOut.substr(0, lr) == ur);
  }
  else
    return false;
}


bool validateLINTrailingNoise(ValState& valState)
{
  // Accept a dangling st.
  if (valState.dataRef.label != "st")
    return false;

  if (! valState.bufferRef.next(valState.dataRef))
    return true;

  return false;
}


bool validateLIN(
  ValState& valState,
  ValProfile& prof)
{
  // Loop over combinations of different tags that we tolerate.

  while (true)
  {
    if (isDifferentCase(valState.dataRef.label, valState.dataOut.label))
    {
      if (isDifferentCase(valState.dataRef.value, valState.dataOut.value))
        return true;
      else
        break;
    }

    FixTag tRef = str2tag(valState.dataRef.label);
    FixTag tOut = str2tag(valState.dataOut.label);
    if (tRef == VAL_LIN_FIX_SIZE || tOut == VAL_LIN_FIX_SIZE)
      break;

    const FixInfo& fixInfo = fixTable[tRef][tOut];
    if (fixInfo.advancer == VAL_LIN_NONE)
    {
      if (valState.dataRef.label == "pw" &&
          valState.dataOut.label == "pn")
        break;
      else
        return false;
    }

    if (fixInfo.valFlag && fixInfo.val != valState.dataRef.value)
      return false;

    if (fixInfo.advancer == VAL_LIN_REF)
    {
      if (valState.bufferRef.next(valState.dataRef))
        PLOG(fixInfo.error);
      else
        return false;
    }
    else
    {
      if (valState.bufferOut.next(valState.dataOut))
        PLOG(fixInfo.error);
      else
        return false;
    }
  }

  if (valState.dataRef.label == "pw" &&
      valState.dataOut.label == "pn")
  {
    // Different syntax in a few old VG files.
    if (valState.dataRef.len < valState.dataOut.len &&
        valState.dataOut.value.substr(0, valState.dataRef.value.length()) 
          == valState.dataRef.value)
    {
      PLOG(BRIDGE_VAL_LIN_PN_EXTRA);
      return true;
    }
    else
      return false;
  }

  FixTag tOut = str2tag(valState.dataOut.label);
  ValLINEntry ventry = valLINPtr[tOut];
  if (ventry.set)
  {
    // Currently md, pn, pw, qx, rs, sv, vg.
    if ((* ventry.fptr)(valState.dataRef.value, valState.dataOut.value))
    {
      PLOG(ventry.error);
      return true;
    }
    else
      return false;
  }
  else if (valState.dataRef.label == "pc")
  {
    // Could be a play rotation from an early LIN_VG file.
    if (isRotatedPlay(valState, prof))
    {
      PLOG(BRIDGE_VAL_LIN_PC_ROTATED);
      return true;
    }
    else
      return false;
  }
  else if (valState.dataRef.label == "mb")
  {
    if (valState.dataRef.len == valState.dataOut.len)
    {
      if (isXDouble(valState.dataRef.value, valState.dataOut.value))
      {
        PLOG(BRIDGE_VAL_AUCTION);
        return true;
      }
      else
        return false;
    }
    else if (isCall(valState.dataRef.value, valState.dataOut.value))
    {
      PLOG(BRIDGE_VAL_LIN_AN_ERROR);
      return true;
    }
    else if (isCompactSequence(valState))
    {
      // Might be a single-entry bidding sequence.
      PLOG(BRIDGE_VAL_LIN_AN_ERROR);
      return (valState.dataRef.label == valState.dataOut.label &&
              valState.dataRef.value == valState.dataOut.value);
    }
    else
      return false;
  }
  else
    return false;
}

