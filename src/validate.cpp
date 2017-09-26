/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <regex>
#pragma warning(pop)

#include "Chunk.h"
#include "ValStats.h"
#include "ValProfile.h"
#include "valint.h"
#include "RefLines.h"

#include "filePBN.h"
#include "validateLIN.h"
#include "validatePBN.h"
#include "validateRBN.h"
#include "validateTXT.h"
#include "validateEML.h"
#include "validateREC.h"
#include "parse.h"

#define PLOG(x) prof.log(x, valState.dataOut, valState.dataRef)


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

  setValidateLINTables();
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
      PLOG(BRIDGE_VAL_RECORD_NUMBER);
      continue;
    }
    else if ((* valPtr[formatRef])(valState, prof))
      continue;

    PLOG(BRIDGE_VAL_ERROR);
  }

  if (valState.bufferOut.next(valState.dataOut))
    PLOG(BRIDGE_VAL_REF_SHORT);

  if (valState.bufferRef.next(valState.dataRef))
  {
    if (formatRef == BRIDGE_FORMAT_LIN_VG &&
        validateLINTrailingNoise(valState))
    {
      // Kludge: Accept.
    }
    else
      PLOG(BRIDGE_VAL_OUT_SHORT);
  }

  if (options.verboseValDetails)
    prof.print(cout);

  vstats.add(formatOrig, formatRef, prof);
  return ! prof.hasError();
}


static bool validateCorePBN(
  ValState& valState,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats)
{
  ValProfile prof;
  Chunk chunkRef, chunkOut, prevRef, prevOut;
  bool newSegRef = false, newSegOut = false;
  bool doneRef, doneOut;

  while (true)
  {
    prevRef.copyFrom(chunkRef, CHUNK_PBN);
    prevOut.copyFrom(chunkOut, CHUNK_PBN);

    chunkRef.reset();
    chunkOut.reset();

    readPBNChunk(valState.bufferRef, chunkRef, newSegRef);
    readPBNChunk(valState.bufferOut, chunkOut, newSegOut);

    doneRef = chunkRef.seemsEmpty();
    doneOut = chunkOut.seemsEmpty();

    chunkRef.copyFrom(prevRef, CHUNK_PBN_SOFTLY);
    chunkOut.copyFrom(prevOut, CHUNK_PBN_SOFTLY);

    if (! doneRef && !doneOut)
    {
      if (validatePBNChunk(chunkRef, chunkOut, valState, prof))
        continue;

      PLOG(BRIDGE_VAL_ERROR);
    }

    if (doneRef || doneOut)
      break;
  }

  if (! doneOut)
    PLOG(BRIDGE_VAL_REF_SHORT);

  if (! doneRef)
    PLOG(BRIDGE_VAL_OUT_SHORT);

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
  RefLines refLines;
  valState.bufferOut.read(fileOut, formatRef, refLines);
  refLines.reset();
  valState.bufferRef.read(fileRef, formatRef, refLines);

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

  RefLines refLines;
  valState.bufferOut.fix(fileOut, refLines);

  valState.bufferRef.read(fileRef, formatRef, refLines);

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


bool refContainsOut(
  const LineData& dataOut,
  const LineData& dataRef)
{
  if (dataOut.len >= dataRef.len)
    return false;

  if (dataOut.line == dataRef.line.substr(0, dataOut.len))
    return true;
  else
    return false;
}


bool refContainsOutValue(
  const LineData& dataOut,
  const LineData& dataRef)
{
  if (dataOut.len >= dataRef.len)
    return false;

  if (dataOut.value == dataRef.value.substr(0, dataOut.value.length()))
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

