/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_VALSTATS_H
#define BRIDGE_VALSTATS_H

#include <string>
#include <iostream>

#include "ValProfile.h"

#include "../bconst.h"


using namespace std;


class ValStats
{
  private:

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
      unsigned count[BRIDGE_VAL_SUMM_SIZE];
    };

    ValStat stats[BRIDGE_FORMAT_LABELS_SIZE][BRIDGE_FORMAT_LABELS_SIZE];


    string posOrDash(const unsigned u) const;

    bool rowHasEntries(
      const ValStat stat[],
      const unsigned label) const;

    bool summRowHasEntries(
      const ValStat stat[],
      const unsigned summLabel) const;

    void printRow(
      ostream& fstr,
      const ValStat vstat[],
      const unsigned label) const;

    void printRows(
      ostream& fstr,
      const ValStat vstat[],
      const unsigned lower,
      const unsigned upper) const;

    void printSummRow(
      ostream& fstr,
      const ValStat vstat[],
      const string& header,
      const ValSumm summLabel) const;


  public:

    ValStats();

    ~ValStats();

    void reset();

    void add(
      const Format formatOrig,
      const Format formatRef,
      const ValProfile& prof);

    void operator += (const ValStats& statsIn);
      
    void print(
      ostream& fstr,
      const bool detailFlag = true) const;
};

#endif

