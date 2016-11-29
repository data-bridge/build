/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "validateLIN.h"
#include "parse.h"


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
      return false;
  }
  return true;
}


static bool isContracts(
  const string& valueRef,
  const string& valueOut,
  const string& bufRefName,
  const unsigned lineno)
{
  unsigned p = 0;
  const unsigned l = valueRef.length();
  while (p+1 < l && valueRef.substr(p, 2) == ",,")
    p += 2;

  if (p+1 >= l)
    return false;

  // Leading commas.
  if (valueOut == valueRef.substr(p))
    return true;

  unsigned q = l;
  while (q >= 2 && valueRef.substr(q-2, 2) == ",,")
    q -= 2;

  // Trailing commas.
  if (q <= p)
    return false;

  if (valueOut == valueRef.substr(p, q-p))
    return true;

  // Last-ditch effort: If the ref buffer has a fix file, it might
  // be a fix of a specific contract in the list.
  const string fixName = changeExt(bufRefName, ".fix");
  ifstream ff(fixName.c_str());
  if (! ff.good())
    return false;

  const int countRef = count(valueRef.begin(), valueRef.end(), ',');
  const int countOut = count(valueOut.begin(), valueOut.end(), ',');

  if (countRef != countOut)
    return false;
  
  vector<string> listRef, listOut;
  if (! LINtoList(valueRef, listRef, countRef))
    return false;
  if (! LINtoList(valueOut, listOut, countOut))
    return false;

  unsigned diffs = 0;
  for (unsigned i = 0; i < static_cast<unsigned>(countRef); i++)
    if (listRef[i] != listOut[i])
      diffs++;

  if (diffs >= 2)
    return false;

  string lnew = STR(lineno) + " replace \"rs|" + valueOut + "|\"";
  const string refName = changeExt(bufRefName, ".ref");
  appendFile(refName, lnew);
  return false;
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


static bool replaceTricks(
  string& target,
  const string& source)
{
  size_t tp0 = target.find("mc|");
  if (tp0 == string::npos || tp0+3 >= target.length())
    return false;

  size_t sp0 = source.find("mc|");
  if (sp0 == string::npos || sp0+3 >= source.length())
    return false;

  size_t tp1 = target.find("|", tp0+3);
  if (tp1 == string::npos)
    return false;

  size_t sp1 = source.find("|", sp0+3);
  if (sp1 == string::npos)
    return false;

  target.erase(tp0+3, tp1 - (tp0+3));
  target.insert(tp0+3, source, sp0+3, sp1 - (sp0+3));
  return true;
}


bool validateLIN(
  ValState& valState,
  ValProfile& prof)
{
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
    else if (valState.dataRef.label == "rs")
    {
      if (isContracts(valState.dataRef.value, valState.dataOut.value,
        valState.bufferRef.name(), valState.dataRef.no))
      {
        prof.log(BRIDGE_VAL_BOARDS_HEADER, valState);
        return true;
      }
      else
        return false;
    }
    else if (valState.dataRef.label == "mc")
    {
      // If the ref buffer has a fix file, we guess that it was for
      // this reason.
      const string fixName = changeExt(valState.bufferRef.name(), ".fix");
      ifstream ff(fixName.c_str());
      if (! ff.good())
        return false;
      
      string lref = STR(valState.dataRef.no) + " replace \"" +
        valState.bufferRef.getLine(valState.dataRef.no) + "\"";
      string lout = valState.bufferOut.getLine(valState.dataOut.no);

      if (! replaceTricks(lref, lout))
        return false;

      const string refName = changeExt(valState.bufferRef.name(), ".ref");
      appendFile(refName, lref);
      prof.log(BRIDGE_VAL_VG_MC, valState);
      return true;
    }
    else if (valState.dataRef.len != valState.dataOut.len)
    {
      const unsigned lr = valState.dataRef.len;
      const unsigned lo = valState.dataOut.len;

      if (valState.dataRef.label == "mb" &&
          (lr == 6 || lr == 7) &&
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
      else if (valState.dataRef.label == "mb" &&
          lr+1 == lo &&
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
      else
        return false;
    }
    else
      return isDifferentCase(valState.dataRef.value,
        valState.dataOut.value);
      // Could maybe consider equality an error here, but only w.r.t. case
  }

  if (valState.dataRef.label != valState.dataOut.label)
    return false;
  else if (valState.dataRef.len != valState.dataOut.len)
    return false;
  else
    return isDifferentCase(valState.dataRef.value,
      valState.dataOut.value);
    // Could maybe consider equality an error here, but only w.r.t. case
}

