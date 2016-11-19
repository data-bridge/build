/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include "validateRBN.h"
#include "parse.h"


static bool isRBNMissing(
  ValState& valState,
  ValProfile& prof)
{
  if (valState.dataRef.label == "P" && 
      valState.dataOut.label == "R")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != valState.dataOut.label)
      return false;

    prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
    return (valState.dataRef.line == valState.dataOut.line);
  }
  else if (valState.dataRef.label == "K")
  {
    if (! valState.bufferRef.next(valState.dataRef))
      return false;

    if (valState.dataRef.label != valState.dataOut.label)
      return false;

    prof.log(BRIDGE_VAL_TEAMS, valState);
    return (valState.dataRef.line == valState.dataOut.line);
  }

  return false;
}


static bool areRBNNames(
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
    if (! firstContainsSecond(listRef[i], listOut[i]))
      return false;
  }
  return true;
}


static bool isRBNShort(
  ValState& valState,
  ValProfile& prof)
{
  switch (valState.dataOut.label.at(0))
  {
    case 'T':
      if (valState.dataOut.len > 2)
        return false;
      prof.log(BRIDGE_VAL_TITLE, valState);
      return true;

    case 'D':
      if (valState.dataOut.len > 2)
        return false;
      prof.log(BRIDGE_VAL_DATE, valState);
      return true;
        
    case 'L':
      if (valState.dataOut.len > 2)
        return false;
      prof.log(BRIDGE_VAL_LOCATION, valState);
      return true;
        
    case 'E':
      if (valState.dataOut.len > 2)
        return false;
      prof.log(BRIDGE_VAL_EVENT, valState);
      return true;
        
    case 'S':
      if (! refContainsOut(valState))
        return false;
      prof.log(BRIDGE_VAL_SESSION, valState);
      return true;

    case 'N':
      if (! areRBNNames(valState.dataRef.value, valState.dataOut.value))
        return false;
      prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
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


bool validateRBN(
  ValState& valState,
  ValProfile& prof)
{
  if (valState.dataOut.type == BRIDGE_BUFFER_EMPTY ||
      valState.dataRef.type == BRIDGE_BUFFER_EMPTY)
    return false;

  if (valState.dataOut.label != valState.dataRef.label)
  {
    if (isRBNMissing(valState, prof))
      return true;
  }

  if (valState.dataOut.label == valState.dataRef.label)
    return isRBNShort(valState, prof);
  else
    return false;
}

