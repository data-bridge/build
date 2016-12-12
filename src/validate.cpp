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

typedef bool (* ValPtr)(ValState&, ValProfile&);
ValPtr valPtr[BRIDGE_FORMAT_LABELS_SIZE];


static bool validateCore(
  ValState& valState,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats);


void setValidateTables()
{
  valPtr[BRIDGE_FORMAT_LIN] = &validateLIN_RP;
  valPtr[BRIDGE_FORMAT_LIN_RP] = &validateLIN_RP;
  valPtr[BRIDGE_FORMAT_LIN_VG] = &validateLIN;
  valPtr[BRIDGE_FORMAT_LIN_TRN] = &validateLIN_RP;
  valPtr[BRIDGE_FORMAT_PBN] = &validatePBN;
  valPtr[BRIDGE_FORMAT_RBN] = &validateRBN;
  valPtr[BRIDGE_FORMAT_RBX] = &validateRBN;
  valPtr[BRIDGE_FORMAT_TXT] = &validateTXT;
  valPtr[BRIDGE_FORMAT_EML] = &validateEML;
  valPtr[BRIDGE_FORMAT_REC] = &validateREC;
}


static bool isRecordComment(
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


static bool validateCore(
  ValState& valState,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats)
{
  ValProfile prof;

  while (valState.bufferRef.next(valState.dataRef))
  {
    if (! valState.bufferOut.next(valState.dataOut))
      break;

    if (valState.dataRef.line == valState.dataOut.line)
      continue;

    // General: % line numbers (Pavlicek error).
    if (valState.dataOut.type == BRIDGE_BUFFER_COMMENT &&
        valState.dataRef.type == BRIDGE_BUFFER_COMMENT &&
        isRecordComment(valState.dataOut.line, valState.dataRef.line))
    {
      prof.log(BRIDGE_VAL_RECORD_NUMBER, valState);
      continue;
    }
    else if ((* valPtr[formatRef])(valState, prof))
      continue;

    prof.log(BRIDGE_VAL_ERROR, valState);
  }

  if (valState.bufferOut.next(valState.dataOut))
    prof.log(BRIDGE_VAL_REF_SHORT, valState);

  if (valState.bufferRef.next(valState.dataRef))
    prof.log(BRIDGE_VAL_OUT_SHORT, valState);

  if (options.verboseValDetails)
    prof.print(cout);

  vstats.add(formatOrig, formatRef, prof);
  return ! prof.hasError();
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
  valState.bufferOut.fix(fileOut);

  valState.bufferRef.read(fileRef, formatRef);
  valState.bufferRef.fix(fileRef);

  if (! validateCore(valState, formatOrig, formatRef, options, vstats))
    cout << "Error came from " << fileOut << " -> " << fileRef << "\n";
}


void validate(
  const string& strOut,
  const string& fileOut,
  const string& fileRef,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats)
{
  ValState valState;
  valState.bufferOut.split(strOut, formatRef);
  valState.bufferOut.fix(fileOut);

  valState.bufferRef.read(fileRef, formatRef);
  valState.bufferRef.fix(fileRef);

  if (! validateCore(valState, formatOrig, formatRef, options, vstats))
    cout << "Error came from " << fileOut << " -> " << fileRef << "\n";
}


bool refContainsOut(const ValState& valState)
{
  if (valState.dataOut.len >= valState.dataRef.len)
    return false;

  if (valState.dataOut.line == 
      valState.dataRef.line.substr(0, valState.dataOut.len))
    return true;
  else
    return false;
}


bool refContainsOutValue(const ValState& valState)
{
  if (valState.dataOut.len >= valState.dataRef.len)
    return false;

  if (valState.dataOut.value == 
      valState.dataRef.value.substr(0, valState.dataOut.value.length()))
    return true;
  else
    return false;
}


bool firstContainsSecond(
  const LineData& first,
  const LineData& second)
{
  if (second.len >= first.len)
    return false;

  if (second.line == first.line.substr(0, second.len))
    return true;
  else
    return false;
}


bool firstContainsSecond(
  const string& first,
  const string& second)
{
  if (second.length() >= first.length())
    return false;

  if (second == first.substr(0, second.length()))
    return true;
  else
    return false;
}

