/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <algorithm>
#include <regex>

#include "Deal.h"
#include "validateLIN.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


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
    prof.log(BRIDGE_VAL_TITLE, valState);

  if (vOut[1] != vRef[1])
    prof.log(BRIDGE_VAL_SESSION, valState);

  if (vOut[2] != vRef[2])
    prof.log(BRIDGE_VAL_SCORING, valState);

  if (vOut[3] != vRef[3] || vOut[4] != vRef[4])
    prof.log(BRIDGE_VAL_SCORING, valState);

  if (vOut[5] != vRef[5] || vOut[6] != vRef[6] ||
      vOut[7] != vRef[7] || vOut[8] != vRef[8])
    prof.log(BRIDGE_VAL_TEAMS, valState);

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
      prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
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
    prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
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
      prof.log(BRIDGE_VAL_LIN_PLAY_NL, valState);
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
      prof.log(BRIDGE_VAL_LIN_PLAY_NL, valState);
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
    prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
    return true;
  }
  else
    return false;
}


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


bool validateLIN(
  ValState& valState,
  ValProfile& prof)
{
  // TODO: Can write a function to streamline these calls.
  // TODO: Error names not updated.

  if (valState.dataRef.label == "an" &&
      valState.dataRef.value == "!" &&
      valState.dataOut.label == "mb")
  {
    // Probably the previous values were mb|bid!|, but ref has a 
    // trailing an|!| which we must get rid of.
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "mb")
      return false;

    prof.log(BRIDGE_VAL_LIN_EXCLAIM, valState);
  }
  if (valState.dataRef.label == "pf" &&
      valState.dataOut.label == "vg")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "vg")
      return false;

    prof.log(BRIDGE_VAL_BOARDS_HEADER, valState);
  }
  if (valState.dataRef.label == "pn" &&
      valState.dataOut.label == "st")
  {
    // Could be pn|...| embedded in qx|o1|, which we choose to
    // disregard, as it is hopefully consistent with the pn header.
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "st" &&
        valState.dataRef.label != "md")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "pn" &&
      valState.dataOut.label == "mb")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "mb")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "pn" &&
      valState.dataOut.label == "md")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "md")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "bn" &&
      valState.dataOut.label == "qx")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "qx" &&
        valState.dataRef.label != "mp")
      return false;

    prof.log(BRIDGE_VAL_BOARDS_HEADER, valState);
  }
  if (valState.dataRef.label == "mp" &&
      valState.dataOut.label == "qx")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "qx" &&
        valState.dataRef.label != "bn" &&
        valState.dataRef.label != "pn")
      return false;

    prof.log(BRIDGE_VAL_SCORES_HEADER, valState);
  }
  if (valState.dataRef.label == "bn" &&
      valState.dataOut.label == "qx")
  {
    // Duplicated on purpose.
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "qx" &&
        valState.dataRef.label != "mp" &&
        valState.dataRef.label != "pn")
      return false;

    prof.log(BRIDGE_VAL_BOARDS_HEADER, valState);
  }
  if (valState.dataRef.label == "mc" &&
      valState.dataOut.label == "pn")
  {
    if (! valState.bufferOut.next(valState.dataOut))
      return false;

    if (valState.dataOut.label != "qx")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "mc" &&
      valState.dataOut.label == "qx")
  {
    // Assume the global pw takes care of this.

    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "qx" &&
        valState.dataRef.label != "pn")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "pn" &&
      valState.dataOut.label == "qx")
  {
    // Assume the global pw takes care of this.

    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "qx" &&
        valState.dataRef.label != "mp")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "rh" &&
      valState.dataOut.label == "st")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "ah" &&
        valState.dataRef.label != "st")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "ah" &&
      valState.dataOut.label == "st")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "st" &&
        valState.dataRef.label != "md")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "st" &&
      valState.dataOut.label == "mb")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "mb")
      return false;

    prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
  }
  if (valState.dataRef.label == "md" &&
      valState.dataOut.label == "st")
  {
    // Could be missing st before md in ref.
    if (! valState.bufferOut.next(valState.dataOut))
      return false;

    if (valState.dataOut.label != "md")
      return false;

    prof.log(BRIDGE_VAL_LIN_ST_MISSING, valState);
  }
  if (valState.dataRef.label == "mb" &&
      valState.dataOut.label == "sv")
  {
    if (! valState.bufferOut.next(valState.dataOut))
      return false;

    if (valState.dataOut.label != "mb")
      return false;

    prof.log(BRIDGE_VAL_LIN_SV_MISSING, valState);
  }
  if (valState.dataRef.label == "qx" &&
      valState.dataOut.label == "sv")
  {
    if (! valState.bufferOut.next(valState.dataOut))
      return false;

    if (valState.dataOut.label != "qx")
      return false;

    prof.log(BRIDGE_VAL_LIN_SV_MISSING, valState);
  }
  if (valState.dataRef.label == "mp" &&
      valState.dataOut.label == "qx")
  {
    // Duplicated on purpose.  For now.
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != "qx")
      return false;

    prof.log(BRIDGE_VAL_SCORES_HEADER, valState);
  }

  if (valState.dataRef.label == "pw" &&
      valState.dataOut.label == "pn")
  {
    // Different syntax in a few old VG files.
    if (valState.dataRef.len < valState.dataOut.len &&
        valState.dataOut.value.substr(0, valState.dataRef.value.length()) ==
        valState.dataRef.value)
    {
      prof.log(BRIDGE_VAL_LIN_PN_EMBEDDED, valState);
      return true;
    }
  }

  if (valState.dataRef.label == valState.dataOut.label)
  {
    if (valState.dataRef.label == "vg")
    {
      if (isTitleBoards(valState.dataRef.value, valState.dataOut.value))
      {
        prof.log(BRIDGE_VAL_BOARDS_HEADER, valState);
        return true;
      }
      else
        return false;
    }
    else if (valState.dataRef.label == "pw" ||
        valState.dataOut.label == "pn")
    {
      if (isPlayersList(valState.dataRef.value, valState.dataOut.value))
      {
        prof.log(BRIDGE_VAL_PLAYERS_HEADER, valState);
        return true;
      }
      else
        return false;
    }
    else if (valState.dataRef.label == "rs")
    {
      if (isContracts(valState.dataRef.value, valState.dataOut.value))
      {
        prof.log(BRIDGE_VAL_BOARDS_HEADER, valState);
        return true;
      }
      else
        return false;
    }
    else if (valState.dataRef.label == "qx")
    {
      if (isBoard(valState.dataRef.value, valState.dataOut.value))
      {
        // Should we log for completeness?
        return true;
      }
      else
        return false;
    }
    else if (valState.dataRef.label == "md")
    {
      const unsigned lr = static_cast<unsigned>
        (valState.dataRef.value.length());
      if (lr == 54 &&
          valState.dataOut.value.length() == 72 &&
          valState.dataOut.value.substr(0, lr) == valState.dataRef.value)
      {
        prof.log(BRIDGE_VAL_VG_MD, valState);
        return true;
      }
      else if ((lr == 54 || lr == 55 || (lr >= 69 && lr <= 72)) && 
          valState.dataOut.value.length() == 72)
      {
        Deal dealRef, dealOut;
        try
        {
          dealRef.set(valState.dataRef.value, BRIDGE_FORMAT_LIN);
          dealOut.set(valState.dataOut.value, BRIDGE_FORMAT_LIN);

          if (dealRef == dealOut)
          {
            prof.log(BRIDGE_VAL_VG_MD, valState);
            return true;
          }
          else
            return false;
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
      else if (valState.dataRef.value == valState.dataOut.value)
        return true;
      else
        return false;
    }
    else if (valState.dataRef.label == "sv")
    {
      if ((valState.dataRef.value == "0" &&
          valState.dataOut.value == "o") ||
          (valState.dataRef.value == "B" &&
          valState.dataOut.value == "b") ||
          (valState.dataRef.value == "N" &&
          valState.dataOut.value == "n") ||
          (valState.dataRef.value == "E" &&
          valState.dataOut.value == "e"))
        return true;
      else
        return false;
    }
    else if (valState.dataRef.len != valState.dataOut.len)
    {
      const unsigned lr = valState.dataRef.len;
      const unsigned lo = valState.dataOut.len;

      if (valState.dataRef.label == "mb")
      {
        if ((lr == 6 || lr == 7) &&
            lo+1 == lr &&
            valState.dataRef.value.at(lr-5) == '!')
        {
          // Output is already in upper case, as we made it.
          string ur = valState.dataRef.value.substr(0, lr-5);
          toUpper(ur);
          if (ur != valState.dataOut.value)
            return false;

          // ref mb|2C!| vs out mb|2C|.
          if (! valState.bufferRef.next(valState.dataRef))
            return false;
          if (! valState.bufferOut.next(valState.dataOut))
            return false;
          if (valState.dataRef.label != "an" ||
              valState.dataOut.label != "an" ||
              valState.dataRef.value != valState.dataOut.value)
            return false;

          prof.log(BRIDGE_VAL_LIN_EXCLAIM, valState);
          return true;
        }
        else if (lr+1 == lo &&
            (lo == 6 || lo == 7) &&
            valState.dataOut.value.at(lo-5) == '!')
        {
          string ur = valState.dataRef.value;
          toUpper(ur);
          if (valState.dataOut.value.substr(0, lo-5) != ur)
            return false;

          // ref mb|2C|an|!| vs out mb|2C!|
          if (! valState.bufferRef.next(valState.dataRef))
            return false;
          if (valState.dataRef.label != "an" ||
              valState.dataRef.value != "!")
            return false;

          prof.log(BRIDGE_VAL_LIN_EXCLAIM, valState);
          return true;
        }
        else if (lr > 8)
        {
          // Might be a single-entry bidding sequence.
          if (isCompactSequence(valState))
          {
            prof.log(BRIDGE_VAL_LIN_EXCLAIM, valState);
            if (valState.dataRef.label == valState.dataOut.label &&
                valState.dataRef.value == valState.dataOut.value)
              return true;
            else
              return false;
          }
          else
            return false;
        }
        else
          return false;
      }
      else
        return false;
    }
    else
      return isDifferentCase(valState.dataRef.value,
        valState.dataOut.value);
    // Could maybe consider equality an error here, but only w.r.t. case
  }

  if (valState.dataRef.label != valState.dataOut.label)
  {
    // Normal LIN has rh|| and ah|Board|, not in LIN_VG.
    if (valState.dataOut.label == "sv" &&
        valState.dataRef.label == "rh" &&
        valState.dataRef.value == "")
    {
      if (! valState.bufferRef.next(valState.dataRef))
        return false;
      if (valState.dataRef.label != "ah")
        return false;
      if (! valState.bufferRef.next(valState.dataRef))
        return false;
      if (valState.dataRef.label != "sv")
        return false;
      if (valState.dataRef.value != valState.dataOut.value)
        return false;

      prof.log(BRIDGE_VAL_LIN_RH_AH, valState);
      return true;
    }

    return false;
  }
  else if (valState.dataRef.len != valState.dataOut.len)
    return false;
  else
    return isDifferentCase(valState.dataRef.value,
      valState.dataOut.value);
    // Could maybe consider equality an error here, but only w.r.t. case
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

