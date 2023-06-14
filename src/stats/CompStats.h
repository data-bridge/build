/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This class provides a very basic error rate of formats:

   format     count  errors
   LIN          745     683
*/

#ifndef BRIDGE_COMPSTATS_H
#define BRIDGE_COMPSTATS_H

#include <string>

#include "../Format.h"


using namespace std;


class CompStats
{
  private:

    struct CompStat
    {
      unsigned count;
      unsigned errors;
    };

    CompStat stats[BRIDGE_FORMAT_SIZE];

  public:

    CompStats();

    void reset();

    void add(
      const bool agreeFlag,
      const Format format);

    void operator += (const CompStats& cs2);
      
    string str() const;
};

#endif

