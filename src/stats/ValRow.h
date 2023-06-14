/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_VALROW_H
#define BRIDGE_VALROW_H

#include <string>

#include "../validate/ValProfile.h"

#include "../Format.h"
#include "../Label.h"


using namespace std;

enum ValSumm
{
  BRIDGE_VAL_ALL = 0,
  BRIDGE_VAL_MINOR = 1,
  BRIDGE_VAL_PAVLICEK = 2,
  BRIDGE_VAL_MAJOR = 3,
  BRIDGE_VAL_SUMM_SIZE = 4
};


class ValRow
{
  private:

    struct ValStat
    {
      ValProfile profile;
      size_t count[BRIDGE_VAL_SUMM_SIZE];

      void add(const ValProfile& prof)
      {
        bool minorFlag, pavlicekBugFlag, programErrorFlag;

        profile.addRange(prof, 0, BRIDGE_VAL_TXT_ALL_PASS, minorFlag);

        profile.addRange(prof, BRIDGE_VAL_TXT_ALL_PASS,
          BRIDGE_VAL_ERROR, pavlicekBugFlag);

        profile.addRange(prof, BRIDGE_VAL_ERROR,
          BRIDGE_VAL_SIZE, programErrorFlag);

        count[BRIDGE_VAL_ALL]++;

        if (minorFlag)
          count[BRIDGE_VAL_MINOR]++;

        if (pavlicekBugFlag)
          count[BRIDGE_VAL_PAVLICEK]++;

        if (programErrorFlag)
          count[BRIDGE_VAL_MAJOR]++;
      };
    };

    ValStat row[BRIDGE_FORMAT_LABELS_SIZE];

    string posOrDash(const size_t u) const;


  public:

    ValRow();

    void reset();

    void add(
      const Format formatRef,
      const ValProfile& prof);

    void operator += (const ValRow& vr2);

    bool profileHasLabel(const unsigned label) const;

    bool countHasLabel(const ValSumm label) const;

    string strProfile(const unsigned label) const;

    string strCount(
      const string& header,
      const ValSumm label) const;

};

#endif

