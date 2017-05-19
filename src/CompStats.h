/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_COMPSTATS_H
#define BRIDGE_COMPSTATS_H

#include <iostream>

#include "bconst.h"


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

    ~CompStats();

    void reset();

    void add(
      const bool agreeFlag,
      const Format format);

    void operator += (const CompStats& statsIn);
      
    void print(ostream& fstr) const;
};

#endif

