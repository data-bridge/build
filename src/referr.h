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
#include "Refline.h"

using namespace std;


enum RefControl
{
  ERR_REF_STANDARD = 0,
  ERR_REF_SKIP = 1,
  ERR_REF_NOVAL = 2,
  ERR_REF_OUT_COCO = 3,
  ERR_REF_OUT_OOCC = 4
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


void readRefFix(
  const string& fname,
  vector<Refline>& reflines,
  RefControl& refControl);

string strRefFix(const RefFix& refFix);

bool classifyRefLine(
  const Refline& refline,
  const string& bufferLine,
  RefErrorClass& diff);

#endif
