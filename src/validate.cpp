/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <regex>

#include "ValStats.h"
#include "ValProfile.h"

#include "validateLIN.h"
#include "validatePBN.h"
#include "validateRBN.h"
#include "validateTXT.h"
#include "validateEML.h"
#include "validateREC.h"
#include "parse.h"


using namespace std;


bool isRecordComment(
  const string& lineOut,
  const string& lineRef)
{
  // Detect Pavlicek bug with wrong record numbers.

  regex re("(\\d+) records (\\d+) deals");
  smatch match;
  unsigned r1, d1, r2, d2;

  if (! regex_search(lineOut, match, re))
    return false;

  if (! str2unsigned(match.str(1), r1)) return false;
  if (! str2unsigned(match.str(2), d1)) return false;

  if (! regex_search(lineRef, match, re))
    return false;

  if (! str2unsigned(match.str(1), r2)) return false;
  if (! str2unsigned(match.str(2), d2)) return false;

  if (r2 <= r1 || d2 <= d1)
    return false;

  return true;
}


void validate(
  const string& fileOut,
  const string& fileRef,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats)
{
  ValState valState;
  valState.bufferOut.read(fileOut, formatRef);
  valState.bufferOut.fix(fileOut, formatRef);

  valState.bufferRef.read(fileRef, formatRef);
  valState.bufferRef.fix(fileRef, formatRef);

  ValProfile prof;

  unsigned headerStartTXT = 4; // If comments and no dash line

  while (valState.bufferRef.next(valState.dataRef))
  {
    if (! valState.bufferOut.next(valState.dataOut))
    {
      prof.log(BRIDGE_VAL_OUT_SHORT, valState);
      break;
    }

    if (formatRef == BRIDGE_FORMAT_TXT &&
        valState.dataOut.type == BRIDGE_BUFFER_DASHES)
      headerStartTXT = valState.dataOut.no + 2;

    if (valState.dataRef.line == valState.dataOut.line)
      continue;

    // General: % line numbers (Pavlicek error).
    if (formatRef != BRIDGE_FORMAT_RBX &&
        valState.dataOut.type == BRIDGE_BUFFER_COMMENT &&
        valState.dataRef.type == BRIDGE_BUFFER_COMMENT &&
        isRecordComment(valState.dataOut.line, valState.dataRef.line))
    {
      prof.log(BRIDGE_VAL_RECORD_NUMBER, valState);
      continue;
    }
    else if (formatRef == BRIDGE_FORMAT_LIN_RP)
    {
      if (validateLIN_RP(valState, prof))
        continue;
    }
    else if (formatRef == BRIDGE_FORMAT_PBN)
    {
      if (validatePBN(valState, prof))
        continue;
    }
    else if (formatRef == BRIDGE_FORMAT_RBN)
    {
      if (validateRBN(valState, prof))
        continue;
    }
    else if (formatRef == BRIDGE_FORMAT_RBX)
    {
      if (validateRBX(valState, prof))
        continue;
    }
    else if (formatRef == BRIDGE_FORMAT_TXT)
    {
      if (validateTXT(valState, headerStartTXT, prof))
        continue;
    }
    else if (formatRef == BRIDGE_FORMAT_EML)
    {
      if (validateEML(valState, prof))
        continue;
    }
    else if (formatRef == BRIDGE_FORMAT_REC)
    {
      if (validateREC(valState, prof))
        continue;
    }

    prof.log(BRIDGE_VAL_ERROR, valState);
  }

  if (valState.bufferOut.next(valState.dataOut))
    prof.log(BRIDGE_VAL_REF_SHORT, valState);

  if (options.verboseValDetails)
    prof.print(cout);

  vstats.add(formatOrig, formatRef, prof);
}


bool refContainsOut(const ValState& valState)
{
  if (valState.dataOut.len > valState.dataRef.len)
    return false;

  if (valState.dataOut.line == 
      valState.dataRef.line.substr(0, valState.dataOut.len))
    return true;
  else
    return false;
}

