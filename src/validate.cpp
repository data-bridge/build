/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <regex>

#include "ValStats.h"
#include "ValProfile.h"

#include "filePBN.h"
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


static bool isOfRPOrigin(const string& fileRef);

static bool chunkIsEmpty(const vector<string>& chunk);

static void copyChunkHeader(
  const vector<string>& chunk,
  vector<string>& prev);

static void restoreChunkHeader(
  vector<string>& chunk,
  vector<string>& prev);

static bool validateCore(
  ValState& valState,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats);

static bool validateCorePBN(
  ValState& valState,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats);


void setValidateTables()
{
  valPtr[BRIDGE_FORMAT_LIN] = &validateLIN;
  valPtr[BRIDGE_FORMAT_LIN_RP] = &validateLIN_RP;
  valPtr[BRIDGE_FORMAT_LIN_VG] = &validateLIN;
  valPtr[BRIDGE_FORMAT_LIN_TRN] = &validateLIN;
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


static bool isOfRPOrigin(const string& fileRef)
{
  regex re("[SUVW]\\d\\d\\w\\w\\w.PBN");
  smatch match;
  return regex_search(fileRef, match, re);
}


static bool chunkIsEmpty(const vector<string>& chunk)
{
  return (chunk[BRIDGE_FORMAT_BOARD_NO] == "" &&
      chunk[BRIDGE_FORMAT_RESULT] == "" &&
      chunk[BRIDGE_FORMAT_AUCTION] == "");
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
  {
    if (formatRef == BRIDGE_FORMAT_LIN_VG &&
        validateLINTrailingNoise(valState))
    {
      // Kludge: Accept.
    }
    else
    prof.log(BRIDGE_VAL_OUT_SHORT, valState);
  }

  if (options.verboseValDetails)
    prof.print(cout);

  vstats.add(formatOrig, formatRef, prof);
  return ! prof.hasError();
}


const vector<Label> PBNFields =
{
  BRIDGE_FORMAT_TITLE,
  BRIDGE_FORMAT_EVENT,
  BRIDGE_FORMAT_DATE,
  BRIDGE_FORMAT_LOCATION,
  BRIDGE_FORMAT_EVENT,
  BRIDGE_FORMAT_SESSION,
  BRIDGE_FORMAT_SCORING,
  BRIDGE_FORMAT_HOMETEAM,
  BRIDGE_FORMAT_VISITTEAM,
  BRIDGE_FORMAT_WEST,
  BRIDGE_FORMAT_NORTH,
  BRIDGE_FORMAT_EAST,
  BRIDGE_FORMAT_SOUTH,
  BRIDGE_FORMAT_BOARD_NO,
  BRIDGE_FORMAT_ROOM
};

static void copyChunkHeader(
  const vector<string>& chunk,
  vector<string>& prev)
{
  for (auto &lb: PBNFields)
    prev[lb] = chunk[lb];
}


static void restoreChunkHeader(
  vector<string>& chunk,
  vector<string>& prev)
{
  for (auto &lb: PBNFields)
  {
    if (chunk[lb] == "")
      chunk[lb] = prev[lb];
  }
}


static bool validateCorePBN(
  ValState& valState,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats)
{
  ValProfile prof;

  vector<string> chunkRef(BRIDGE_FORMAT_LABELS_SIZE),
    chunkOut(BRIDGE_FORMAT_LABELS_SIZE);
  vector<string> prevRef(BRIDGE_FORMAT_LABELS_SIZE),
    prevOut(BRIDGE_FORMAT_LABELS_SIZE);
  vector<unsigned> lnoRef(BRIDGE_FORMAT_LABELS_SIZE), 
    lnoOut(BRIDGE_FORMAT_LABELS_SIZE);
  bool newSegRef = false, newSegOut = false;
  bool doneRef, doneOut;

  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
  {
    chunkRef[i].reserve(128);
    chunkOut[i].reserve(128);
    chunkRef[i] = "";
    chunkOut[i] = "";

    prevRef[i].reserve(128);
    prevOut[i].reserve(128);
  }

  while (true)
  {
    copyChunkHeader(chunkRef, prevRef);
    copyChunkHeader(chunkOut, prevOut);

    for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    {
      chunkRef[i] = "";
      chunkOut[i] = "";
    }

    readPBNChunk(valState.bufferRef, lnoRef, chunkRef, newSegRef);
    readPBNChunk(valState.bufferOut, lnoOut, chunkOut, newSegOut);

    doneRef = chunkIsEmpty(chunkRef);
    doneOut = chunkIsEmpty(chunkOut);

    restoreChunkHeader(chunkRef, prevRef);
    restoreChunkHeader(chunkOut, prevOut);

    if (! doneRef && !doneOut)
    {
      if (validatePBNChunk(chunkRef, chunkOut, valState, prof))
        continue;

      prof.log(BRIDGE_VAL_ERROR, valState);
    }

    if (doneRef || doneOut)
      break;
  }

  if (! doneOut)
    prof.log(BRIDGE_VAL_REF_SHORT, valState);

  if (! doneRef)
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

  if (formatRef == BRIDGE_FORMAT_PBN && ! isOfRPOrigin(fileRef))
  {
    // The order of PBN tags is all over the map in general,
    // except in Pavlicek files.  We only compare chunks for these.
    if (! validateCorePBN(valState, formatOrig, formatRef, options, vstats))
      cout << "Error came from " << fileOut << " -> " << fileRef << "\n";
  }
  else
  {
    if (! validateCore(valState, formatOrig, formatRef, options, vstats))
      cout << "Error came from " << fileOut << " -> " << fileRef << "\n";
  }
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

  if (formatRef == BRIDGE_FORMAT_PBN && ! isOfRPOrigin(fileRef))
  {
    // The order of PBN tags is all over the map in general,
    // except in Pavlicek files.  We only compare chunks for these.
    if (! validateCorePBN(valState, formatOrig, formatRef, options, vstats))
      cout << "Error came from " << fileOut << " -> " << fileRef << "\n";
  }
  else
  {
    if (! validateCore(valState, formatOrig, formatRef, options, vstats))
      cout << "Error came from " << fileOut << " -> " << fileRef << "\n";
  }
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

