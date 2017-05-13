/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_REFLINELIN_H
#define BRIDGE_REFLINELIN_H

#include <string>
#include <regex>

#include "referr.h"

using namespace std;


void parseReplaceLIN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void parseInsertLIN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void parseDeleteLIN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void modifyReplaceLIN(
 const string& line,
 const unsigned start,
 const bool interiorFlag,
 const RefFix& rf,
 const Refline& rl,
 vector<string>& vLIN,
 vector<string>& fields,
 string& llNew);

void modifyInsertLIN(
 const string& line,
 const unsigned start,
 const bool interiorFlag,
 const RefFix& rf,
 const Refline& rl,
 vector<string>& vLIN,
 vector<string>& fields,
 string& llNew);

void modifyDeleteLIN(
 const string& line,
 const unsigned start,
 const bool interiorFlag,
 const RefFix& rf,
 const Refline& rl,
 vector<string>& vLIN,
 vector<string>& fields,
 string& llNew);

#endif
