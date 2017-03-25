/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFERR_H
#define BRIDGE_REFERR_H

#include <string>
#include <vector>

using namespace std;


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

enum RefErrorsType
{
  ERR_LIN_VG_WRONG,

  ERR_LIN_RS_CONTRACT_MISSING,
  ERR_LIN_RS_CONTRACT_WRONG,
  ERR_LIN_RS_RESULT_MISSING,
  ERR_LIN_RS_RESULT_WRONG,

  ERR_LIN_PN_PLAYERS_UNKNOWN,
  ERR_LIN_PN_PLAYERS_WRONG,

  ERR_LIN_QX_MISSING,
  ERR_LIN_QX_WRONG,
  ERR_LIN_QX_UNORDERED,

  ERR_LIN_DISTS_WRONG,

  ERR_LIN_MD_FORMAT,
  ERR_LIN_MD_CONTENT,
  ERR_LIN_MD_MISSING,

  ERR_LIN_SV_MISSING,
  ERR_LIN_SV_WRONG,

  ERR_LIN_MB_OVERLONG,
  ERR_LIN_MB_WRONG,

  ERR_LIN_PC_ROTATED,
  ERR_LIN_PC_WRONG,

  ERR_LIN_MC_CLAIM_WRONG,

  ERR_LIN_SYNTAX_ERROR,

  ERR_LIN_TOO_FLAWED,

  ERR_LIN_DIRECTOR,

  ERR_SIZE
};

struct RefErrorBundle
{
  RefErrorsType val;
  string name;
  string text;
};

const vector<RefErrorBundle> RefErrors =
{
  {ERR_LIN_VG_WRONG,
   "ERR_LIN_VG_WRONG",
   "LIN vg wrong"},

  {ERR_LIN_RS_CONTRACT_MISSING, 
   "ERR_LIN_RS_CONTRACT_MISSING",
   "LIN rs missing contract"},

  {ERR_LIN_RS_CONTRACT_WRONG,
   "ERR_LIN_RS_CONTRACT_WRONG",
   "LIN rs wrong contract"},

  {ERR_LIN_RS_RESULT_MISSING,
   "ERR_LIN_RS_RESULT_MISSING",
   "LIN rs missing result"},

  {ERR_LIN_RS_RESULT_WRONG,
   "ERR_LIN_RS_RESULT_WRONG",
   "LIN rs wrong result"},

  {ERR_LIN_PN_PLAYERS_UNKNOWN,
   "ERR_LIN_PN_PLAYERS_UNKNOWN",
   "LIN pn not all names"},

  {ERR_LIN_PN_PLAYERS_WRONG,
   "ERR_LIN_PN_PLAYERS_WRONG",
   "LIN pn name commas"},

  {ERR_LIN_QX_MISSING,
   "ERR_LIN_QX_MISSING",
   "LIN qx not present"},

  {ERR_LIN_QX_WRONG,
   "ERR_LIN_QX_WRONG",
   "LIN qx wrong"},

  {ERR_LIN_QX_UNORDERED,
   "ERR_LIN_QX_UNORDERED",
   "LIN qx unordered"},

  {ERR_LIN_DISTS_WRONG,
   "ERR_LIN_DISTS_WRONG",
   "LIN dists wrong"},

  {ERR_LIN_MD_FORMAT,
   "ERR_LIN_MD_FORMAT",
   "LIN md deal format"},

  {ERR_LIN_MD_CONTENT,
   "ERR_LIN_MD_CONTENT",
   "LIN md deal content"},

  {ERR_LIN_MD_MISSING,
   "ERR_LIN_MD_MISSING",
   "LIN md deal missing"},

  {ERR_LIN_SV_MISSING,
   "ERR_LIN_SV_MISSING",
   "LIN sv missing"},

  {ERR_LIN_SV_WRONG,
   "ERR_LIN_SV_WRONG",
   "LIN sv wrong"},

  {ERR_LIN_MB_OVERLONG,
   "ERR_LIN_MB_OVERLONG",
   "LIN mb auction too long"},

  {ERR_LIN_MB_WRONG,
   "ERR_LIN_MB_WRONG",
   "LIN mb auction wrong"},

  {ERR_LIN_PC_ROTATED,
   "ERR_LIN_PC_ROTATED",
   "LIN pc play rotated"},

  {ERR_LIN_PC_WRONG,
   "ERR_LIN_PC_WRONG",
   "LIN pc play wrong"},

  {ERR_LIN_MC_CLAIM_WRONG,
   "ERR_LIN_MC_CLAIM_WRONG",
   "LIN mc claim wrong"},

  {ERR_LIN_SYNTAX_ERROR,
   "ERR_LIN_SYNTAX_ERROR",
   "LIN syntax error"},

  {ERR_LIN_TOO_FLAWED,
   "ERR_LIN_TOO_FLAWED",
   "LIN too flawed"},

  {ERR_LIN_DIRECTOR,
   "ERR_LIN_DIRECTOR",
   "LIN director decision"},

  {ERR_SIZE,
   "ERR_SIZE",
   "Unclassified error"},
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
  vector<RefFix>& refFix);

string strRefFix(const RefFix& refFix);

bool modifyLINLine(
  const string& line,
  const RefFix& refFix,
  string& lineNew);

bool classifyRefLine(
  const RefFix& refEntry,
  const string& bufferLine,
  RefErrorClass& diff);

#endif
