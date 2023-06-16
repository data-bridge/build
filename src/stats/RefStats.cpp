/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>

#include "RefStats.h"

#include "../edits/Comment.h"
#include "../handling/Bexcept.h"

enum CommentType: unsigned;


static const vector<string> RefTableNames =
{
  "Source files", "Ref files", "Skip files", "Noval files", "Order files"
};


RefStats::RefStats()
{
  data.resize(REFSTATS_SIZE);
  seenComment.resize(REFSTATS_SIZE);
  numFiles.resize(REFSTATS_SIZE);

  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
  {
    data[table].resize(ERR_SIZE);
    seenComment[table].resize(ERR_SIZE);
  }

  RefStats::reset();
}


void RefStats::reset()
{
  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
  {
    numFiles[table] = 0;

    for (unsigned comment = 0; comment < ERR_SIZE; comment++)
    {
      data[table][comment].reset();
      seenComment[table][comment] = false;
    }
  }
}


void RefStats::logFile(const RefTables table)
{
  numFiles[table]++;

  for (unsigned comment = 0; comment < ERR_SIZE; comment++)
    seenComment[table][comment] = false;
}


void RefStats::log(
  const RefTables table,
  const CommentType comment,
  const RefEntry& re)
{
  if (seenComment[table][comment])
  {
    RefEntry re2 = re;
    re2.setFiles(0);
    data[table][comment] += re2;
  }
  else
  {
    data[table][comment] += re;
    seenComment[table][comment] = true;
  }
}


void RefStats::operator += (const RefStats& rs2)
{
  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
  {
    for (unsigned comment = 0; comment < ERR_SIZE; comment++)
    {
      data[table][comment] += rs2.data[table][comment];
      seenComment[table][comment] |= rs2.seenComment[table][comment];
    }

    numFiles[table] += rs2.numFiles[table];
  }
}


string RefStats::str() const
{
  if (numFiles[REFSTATS_SOURCE] == 0)
    return "";

  const string dashes(72, '-');

  stringstream ss;
  RefEntry refSum;

  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
  {
    if (numFiles[table] == 0)
      continue;

    ss << RefTableNames[table] << "\n\n";

    ss << setw(28) << left << "Reference" << refSum.strLineHeader();

    refSum.reset();

    for (unsigned comment = 0; comment < ERR_SIZE; comment++)
    {
      if (data[table][comment].getFiles() == 0)
        continue;

      refSum += data[table][comment];

      if (table == 0)
        continue; // Only show sum for this table

      ss << setw(28) << left << 
        CommentList[comment].name << 
        data[table][comment].strLine();
    }

    ss << dashes << "\n";
    ss << setw(28) << left << "Sum" << refSum.strLine() << "\n";
  }

  return ss.str();
}

