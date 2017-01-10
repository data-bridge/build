/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_VALPROF_H
#define BRIDGE_VALPROF_H

#include <string>
#include <iostream>
#include <vector>

#include "validate.h"

using namespace std;


enum ValError
{
  // This group of errors covers different or missing headers.
  // These are to expected in conversions, as not all file formats
  // contain the complete information.
  BRIDGE_VAL_TITLE = 0,
  BRIDGE_VAL_DATE = 1,
  BRIDGE_VAL_LOCATION = 2,
  BRIDGE_VAL_EVENT = 3,
  BRIDGE_VAL_SESSION = 4,
  BRIDGE_VAL_BOARDS_HEADER = 5,
  BRIDGE_VAL_SCORING = 6,
  BRIDGE_VAL_TEAMS = 7,
  BRIDGE_VAL_ROOM = 8,
  BRIDGE_VAL_DEAL = 9,
  BRIDGE_VAL_VUL = 10,
  BRIDGE_VAL_AUCTION = 11,
  BRIDGE_VAL_PLAY = 12,
  BRIDGE_VAL_SCORE = 13,
  BRIDGE_VAL_DD = 14,
  BRIDGE_VAL_NAMES_SHORT = 15,
  BRIDGE_VAL_TXT_DASHES = 16,
  BRIDGE_VAL_VG_MD = 17,
  BRIDGE_VAL_VG_CHAT = 18,

  // These are Pavlicek bugs.
  BRIDGE_VAL_TXT_ALL_PASS = 19,
  BRIDGE_VAL_LIN_PN_EMBEDDED = 20,
  BRIDGE_VAL_LIN_RH_AH = 21,
  BRIDGE_VAL_LIN_EXCLAIM = 22,
  BRIDGE_VAL_LIN_ST_MISSING = 23,
  BRIDGE_VAL_LIN_SV_MISSING = 24,
  BRIDGE_VAL_LIN_PLAY_NL = 25,
  BRIDGE_VAL_PLAY_SHORT = 26,
  BRIDGE_VAL_REC_MADE_32 = 27,
  BRIDGE_VAL_TXT_RESULT = 28,
  BRIDGE_VAL_RECORD_NUMBER = 29,

  // These are real errors.
  BRIDGE_VAL_ERROR = 30,
  BRIDGE_VAL_OUT_SHORT = 31,
  BRIDGE_VAL_REF_SHORT = 32,
  BRIDGE_VAL_VG_MC = 33,
  BRIDGE_VAL_SIZE = 34
};


class ValProfile 
{
  private:

    struct ValSide
    {
      string line;
      unsigned lno;
    };

    struct ValExample
    {
      ValSide out;
      ValSide ref;
    };

    vector<ValExample> example;
    vector<unsigned> count;


  public:

    ValProfile();

    ~ValProfile();

    void reset();

    void log(
      const ValError label,
      const ValState& valState);

    bool labelIsSet(const unsigned label) const;

    bool hasError(const bool minorFlag = false) const;

    unsigned getCount(const unsigned label) const;

    void operator += (const ValProfile& prof2);

    void addRange(
      const ValProfile& prof,
      const unsigned lower,
      const unsigned upper,
      bool& flag);

    void print(
      ostream& fstr = cout,
      const bool minorFlag = false) const;
};

#endif

