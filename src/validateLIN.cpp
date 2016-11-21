/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


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


static bool isLINDiscard(const string& label)
{
  if (label == "nt" || label == "pg")
    return true;
  else
    return false;
}


static bool skipDiscard(
  LineData& data,
  Buffer& buffer)
{
  while (data.type == BRIDGE_BUFFER_EMPTY || isLINDiscard(data.label))
    if (! buffer.next(data))
      return false;

  return true;
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
  UNUSED(prof);

  if (valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED ||
      valState.dataOut.type != BRIDGE_BUFFER_STRUCTURED)
    return false;

  if (valState.dataRef.label == valState.dataOut.label)
  {
    if (valState.dataRef.len != valState.dataOut.len)
    {
      if (valState.dataRef.label == "mb" &&
          valState.dataRef.len == 7 &&
          valState.dataOut.len == 6 &&
          valState.dataRef.value.at(2) == '!' &&
          valState.dataRef.value.substr(0, 2) == valState.dataOut.value)
      {
        // mb|2C| vs mb|2C!|.
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
      else
        return false;
    }
    else
      return isDifferentCase(valState.dataRef.value,
        valState.dataOut.value);
      // Could maybe consider equality an error here, but only w.r.t. case
  }

  if (! skipDiscard(valState.dataRef, valState.bufferRef))
  {
    if (! skipDiscard(valState.dataOut, valState.bufferOut))
      return true;
    else
      return false;
  }

  if (! skipDiscard(valState.dataOut, valState.bufferOut))
    return false;

  if (valState.dataRef.label != valState.dataOut.label)
    return false;
  else if (valState.dataRef.len != valState.dataOut.len)
    return false;
  else
    return isDifferentCase(valState.dataRef.value,
      valState.dataOut.value);
    // Could maybe consider equality an error here, but only w.r.t. case
}

