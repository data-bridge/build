/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>

#include "ValProfile.h"
#include "validate.h"
#include "valint.h"
#include "validateRBN.h"
#include "parse.h"
#include "Bexcept.h"


bool isRBNMissing(
  ValState& valState,
  ValProfile& prof,
  char& rf);

bool areRBNNames(
  const string& lineRef,
  const string& lineOut);

bool splitRBXToVector(
  const string& line,
  vector<string>& list);


bool isRBNMissing(
  ValState& valState,
  ValProfile& prof,
  char& rf)
{
  char of;

  unsigned lOut = valState.dataOut.len;
  unsigned lRef = valState.dataRef.len;

  if (lOut == 0 || lRef == 0)
    return false;

  of = valState.dataOut.line.at(0);
  rf = valState.dataRef.line.at(0);

  if (of != rf)
  {
    if (rf == 'P' && of == 'R')
    {
      if (! valState.bufferRef.next(valState.dataRef))
      {
        prof.log(BRIDGE_VAL_REF_SHORT, valState);
        return false;
      }

      rf = valState.dataRef.line.at(0);
      if (rf != of)
      {
        prof.log(BRIDGE_VAL_ERROR, valState);
        return false;
      }

      return (valState.dataRef.line == valState.dataOut.line);
    }

    return false;
  }

  switch (of)
  {
    case 'T':
      if (lOut > 2)
        return false;
      prof.log(BRIDGE_VAL_TITLE, valState);
      return true;

    case 'D':
      if (lOut > 2)
        return false;
      prof.log(BRIDGE_VAL_DATE, valState);
      return true;
        
    case 'L':
      if (lOut > 2)
        return false;
      prof.log(BRIDGE_VAL_LOCATION, valState);
      return true;
        
    case 'E':
      if (lOut > 2)
        return false;
      prof.log(BRIDGE_VAL_EVENT, valState);
      return true;
        
    case 'S':
      if (! refContainsOut(valState))
        return false;
      prof.log(BRIDGE_VAL_SESSION, valState);
      return true;

    case 'P':
      if (! refContainsOut(valState))
        return false;
      prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
      return true;

    default:
      return false;
  }

}


bool areRBNNames(
  const string& lineRef,
  const string& lineOut)
{
  vector<string> listRef, listOut;
  listRef.clear();
  listOut.clear();
  tokenize(lineRef, listRef, "+:");
  tokenize(lineOut, listOut, "+:");

  if (listRef.size() != listOut.size())
    return false;

  for (unsigned i = 0; i < listRef.size(); i++)
  {
    if (listRef[i] == listOut[i])
      continue;
    unsigned lOut = listOut[i].length();
    if (lOut >= listRef[i].length() ||
        listRef[i].substr(0, lOut) != listOut[i])
      return false;
  }
  return true;
}


bool validateRBN(
  ValState& valState,
  ValProfile& prof)
{
  char rf = ' ';
  if (isRBNMissing(valState, prof, rf))
    return true;

  if (rf == 'K')
  {
    prof.log(BRIDGE_VAL_TEAMS, valState);

    if (! valState.bufferRef.next(valState.dataRef))
    {
      prof.log(BRIDGE_VAL_REF_SHORT, valState);
      return false;
    }
  }

  if (valState.dataRef.type != BRIDGE_BUFFER_EMPTY && 
      valState.dataOut.type != BRIDGE_BUFFER_EMPTY)
  {
    if (valState.dataRef.label == "N" && valState.dataOut.label == "N")
    {
      if (areRBNNames(valState.dataRef.line, valState.dataOut.line))
        return true;
      else
      {
        prof.log(BRIDGE_VAL_ERROR, valState);
        return false;
      }
    }
  }
 
  if (valState.dataOut.line == valState.dataRef.line)
    return true;
  else
  {
    prof.log(BRIDGE_VAL_ERROR, valState);
    return false;
  }
}


bool splitRBXToVector(
  const string& line,
  vector<string>& list)
{
  string s = line;
  regex re("^(.)\\{([^\\}]*)\\}");
  smatch match;
  list.clear();

  while (regex_search(s, match, re))
  {
    list.push_back(match.str(1));
    list.push_back(match.str(2));
    s = regex_replace(s, re, string(""));
  }

  return (s == "");
}


bool validateRBX(
  ValState& valState,
  ValProfile& prof)
{

  vector<string> vOut, vRef;
  if (! splitRBXToVector(valState.dataOut.line, vOut))
  {
    prof.log(BRIDGE_VAL_ERROR, valState);
    return false;
  }

  if (! splitRBXToVector(valState.dataRef.line, vRef))
  {
    prof.log(BRIDGE_VAL_ERROR, valState);
    return false;
  }

  unsigned j = 0;
  for (unsigned i = 0; i < vRef.size(); i += 2)
  {
    if (j >= vOut.size())
    {
      prof.log(BRIDGE_VAL_ERROR, valState);
      return false;
    }

    unsigned lOut = vOut[j+1].length();
    unsigned lRef = vRef[i+1].length();

    if (vRef[i] != vOut[j])
    {
      if (vRef[i] == "K")
      {
        // j stays unchanged
        prof.log(BRIDGE_VAL_TEAMS, valState);
        continue;
      }
      else if (vRef[i] == "P" && vOut[j] == "R" && vRef[i+2] == "R")
      {
        // Play may be missing completely (e.g., coming from EML).
        prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
        continue;
      }
      else
      {
        prof.log(BRIDGE_VAL_ERROR, valState);
        return false;
      }
    }
    else if (vRef[i+1] == vOut[j+1])
    {
      j += 2;
      continue;
    }
    else if (vRef[i] == "P")
    {
      if (lOut >= lRef || vRef[i+1].substr(0, lOut) != vOut[j+1])
        return false;
      prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
    }
    else if (vRef[i] == "%")
    {
      // Record numbers have not been checked yet.
      string llOut = "% " + vOut[j+1];
      string llRef = "% " + vRef[i+1];
      if (isRecordComment(llOut, llRef))
        prof.log(BRIDGE_VAL_RECORD_NUMBER, valState);
      else
      {
        prof.log(BRIDGE_VAL_ERROR, valState);
        return false;
      }
    }
    else if (vRef[i] == "N")
      return areRBNNames(vRef[i+1], vOut[j+1]);
    else if (vRef[i] == "S")
    {
      if (lOut >= lRef || 
          vRef[i+1].substr(0, lOut) != vOut[j+1])
        return false;
      prof.log(BRIDGE_VAL_SESSION, valState);
    }
    else if (vOut[j+1] != "")
      return false;
    else if (vRef[i] == "T")
    {
      if (lOut > 0)
        return false;
      prof.log(BRIDGE_VAL_TITLE, valState);
    }
    else if (vRef[i] == "D")
    {
      if (lOut > 0)
        return false;
      prof.log(BRIDGE_VAL_DATE, valState);
    }
    else if (vRef[i] == "L")
    {
      if (lOut > 0)
        return false;
      prof.log(BRIDGE_VAL_LOCATION, valState);
    }
    else if (vRef[i] == "E")
    {
      if (lOut > 0)
        return false;
      prof.log(BRIDGE_VAL_EVENT, valState);
    }
    else
      return false;

    j += 2;
  }
  return true;
}

