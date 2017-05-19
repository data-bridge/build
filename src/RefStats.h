/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFSTATS_H
#define BRIDGE_REFSTATS_H

#include <iostream>
#include <string>

#include "refconst.h"

using namespace std;


struct RefEntry
{
  unsigned files;
  unsigned noRefLines;
  RefCount count;
};


class RefStats
{
  private:

    enum RefTables
    {
      REFSTATS_SOURCE = 0,
      REFSTATS_REF = 1,
      REFSTATS_SKIP = 2,
      REFSTATS_NOVAL = 3,
      REFSTATS_SIZE = 4
    };

    RefEntry data[REFSTATS_SIZE][ERR_SIZE];
    bool tagSeen[ERR_SIZE];
    unsigned numSourceFiles;
    unsigned numRefFiles;

    void resetRefEntry(RefEntry& re) const;

    void incrRefEntry(
      RefEntry& re,
      const RefEntry& ref2) const;

    void resetSeen();

    void incr(
      const RefTables table,
      const RefTag tag,
      const RefEntry& count);


  public:

    RefStats();

    ~RefStats();

    void reset();

    void logFile(const RefEntry& re);

    void logRefFile();

    void logSkip(
      const RefTag tag,
      const RefEntry& re);

    void logOrder(
      const RefTag tag,
      const RefEntry& re);

    void logNoval(
      const RefTag tag,
      const RefEntry& re);

    void logRef(
      const RefTag tag,
      RefEntry& re);

    void operator += (const RefStats& rf2);

    void print(ostream& fstr) const;
};

#endif
