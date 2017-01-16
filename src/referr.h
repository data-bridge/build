/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


enum RefErrorsType
{
  ERR_LIN_RS_CONTRACT_MISSING,
  ERR_LIN_RS_CONTRACT_WRONG,

  ERR_LIN_MD_FORMAT,
  ERR_LIN_MD_CONTENT,

  ERR_LIN_SV_MISSING,
  ERR_LIN_SV_WRONG,

  ERR_LIN_MB_OVERLONG,
  ERR_LIN_MB_WRONG,

  ERR_LIN_MC_CLAIM_WRONG,

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
  {ERR_LIN_RS_CONTRACT_MISSING, 
   "ERR_LIN_RS_CONTRACT_MISSING",
   "LIN rs missing contract"},

  {ERR_LIN_RS_CONTRACT_WRONG,
   "ERR_LIN_RS_CONTRACT_WRONG",
   "LIN rs wrong contract"},

  {ERR_LIN_MD_FORMAT,
   "ERR_LIN_MD_FORMAT",
   "LIN md deal format"},

  {ERR_LIN_MD_CONTENT,
   "ERR_LIN_MD_CONTENT",
   "LIN md deal content"},

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

  {ERR_LIN_MC_CLAIM_WRONG,
   "ERR_LIN_MC_CLAIM_WRONG",
   "LIN mc claim wrong"},

  {ERR_SIZE,
   "ERR_SIZE",
   "Unclassified error"},
};
