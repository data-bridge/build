/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFSTATS_H
#define BRIDGE_REFSTATS_H

#include <iostream>
#include <string>

#include "../refconst.h"
#include "../bconst.h"

using namespace std;


class RefStats
{
  private:

    enum RefTables
    {
      REFSTATS_SOURCE = 0,
      REFSTATS_REF = 1,
      REFSTATS_SKIP = 2,
      REFSTATS_NOVAL = 3,
      REFSTATS_ORDER = 4,
      REFSTATS_SIZE = 5
    };

    vector<vector<RefEntry>> data;
    vector<bool> catSeen;
    vector<unsigned> numFiles;

    void resetRefEntry(RefEntry& re) const;

    void incrRefEntry(
      RefEntry& re,
      const RefEntry& ref2) const;

    void resetSeen();

    void incr(
      const RefTables table,
      const CommentType cat,
      const RefEntry& count);


  public:

    RefStats();

    ~RefStats();

    void reset();

    void logFile(const RefEntry& re);

    void logRefFile();

    void logSkip(
      const CommentType cat,
      const RefEntry& re);

    void logOrder(
      const CommentType cat,
      const RefEntry& re);

    void logOrder(
      const BoardOrder order,
      const Format format,
      const RefEntry& re);

    void logNoval(
      const CommentType cat,
      const RefEntry& re);

    void logRef(
      const CommentType cat,
      RefEntry& re);

    void operator += (const RefStats& rf2);

    void print(ostream& fstr) const;
};

#endif
