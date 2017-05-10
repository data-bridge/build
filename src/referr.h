/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFERR_H
#define BRIDGE_REFERR_H

#include <string>
#include <vector>

#include "refcodes.h"

using namespace std;


enum RefControl
{
  ERR_REF_STANDARD = 0,
  ERR_REF_SKIP = 1,
  ERR_REF_NOVAL = 2,
  ERR_REF_OUT_COCO = 3,
  ERR_REF_OUT_OOCC = 4
};


enum FixType
{
  BRIDGE_REF_INSERT = 0,
  BRIDGE_REF_REPLACE = 1,
  BRIDGE_REF_DELETE = 2,
  BRIDGE_REF_INSERT_LIN = 3,
  BRIDGE_REF_REPLACE_LIN = 4,
  BRIDGE_REF_DELETE_LIN = 5
};

struct RefFixLIN
{
  unsigned tagNo; // Starting from 1
  unsigned fieldNo; // Starting from 1
  bool reverseFlag;
  string tag;
  string was;
  string is;
  unsigned extent;
};

struct RefFix
{
  unsigned lno; // First line is 1
  FixType type;
  string value;
  RefFixLIN fixLIN;
  unsigned count;
  bool partialFlag;
};

struct RefErrorClass
{
  FixType type;
  RefErrorsType code;
  vector<string> list;
  bool pureFlag;
  unsigned numTags;
};


void setRefTable();

void readRefFix(
  const string& fname,
  vector<RefFix>& refFix,
  RefControl& refControl);

string strRefFix(const RefFix& refFix);

bool modifyLINLine(
  const string& line,
  const RefFix& refFix,
  string& lineNew);

bool classifyRefLine(
  const RefFix& refEntry,
  const string& bufferLine,
  RefErrorClass& diff);

void modifyLINFail(
  const string& line,
  const RefFixLIN& fixLIN,
  const string& text);

#endif
