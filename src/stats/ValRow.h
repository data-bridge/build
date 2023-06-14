/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_VALROW_H
#define BRIDGE_VALROW_H

#include <string>

#include "../validate/ValProfile.h"

#include "../Format.h"
#include "../Label.h"


using namespace std;


    // TODO Later on private again
    enum ValSumm
    {
      BRIDGE_VAL_ALL = 0,
      BRIDGE_VAL_MINOR = 1,
      BRIDGE_VAL_PAVLICEK = 2,
      BRIDGE_VAL_MAJOR = 3,
      BRIDGE_VAL_SUMM_SIZE = 4
    };

    struct ValStat
    {
      ValProfile profile;
      size_t count[BRIDGE_VAL_SUMM_SIZE];
    };

class ValRow
{
  private:

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

