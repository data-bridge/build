/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This class generates a summary of source and reference files
   (that "edit" the source files), like this:

Source files

Reference                     Files   Refs    Lines  Units  Hands Boards
------------------------------------------------------------------------
Sum                             998    597   256062      0  19063  11068

   This counts the actual source files (LIN, PBN etc.)  
   * Refs is the number of reference files.  
   * Lines is the total number of lines in the source files.  
   * Units the number of times that the reference occurs
     (not relevant for source files).
   * Hands is the number of hands (so if the board is played twice,
   * that count as 2).
   * Boards is the number of boards.

Ref files

Reference                     Files   Refs    Lines  Units  Hands Boards
ERR_LIN_VG_REPLACE                1      1        1      1     18     10
ERR_LIN_RS_REPLACE               12     13       13     13     13     13
...
    
   These are the edits/comments in ref files.

   Skips are a special form of edit where entire input files are
   skipped.

   Noval are files that are not subject to validation.

   Order are files in which the hand ordering is non-standard.
   I think I may have given up on this and just edited the original
   LIN files in the few, early cases where this occurred.
*/


#ifndef BRIDGE_REFSTATS_H
#define BRIDGE_REFSTATS_H

#include <vector>
#include <deque>
#include <string>

#include "../RefEntry.h"

using namespace std;

enum CommentType: unsigned;

enum RefTables
{
  REFSTATS_SOURCE = 0,
  REFSTATS_REF = 1,
  REFSTATS_SKIP = 2,
  REFSTATS_NOVAL = 3,
  REFSTATS_ORDER = 4,
  REFSTATS_SIZE = 5
};


class RefStats
{
  private:

    vector<vector<RefEntry>> data;
    vector<deque<bool>> seenComment;
    vector<unsigned> numFiles;


  public:

    RefStats();

    void reset();

    // This is called once per file.
    void logFile(const RefTables table);

    // This may be called multiple times per file.
    void log(
      const RefTables table,
      const CommentType comment,
      const RefEntry& re);

    void operator += (const RefStats& rs2);

    string str() const;
};

#endif
