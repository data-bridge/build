/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALIDATE_H
#define BRIDGE_VALIDATE_H

#include <string>
#include "bconst.h"

using namespace std;


// File-level stats.

struct ValStatType
{
  unsigned numFiles;

  unsigned numIdentical;
  unsigned numExpectedDiffs;
  unsigned numErrors;
};


void validate(
  const string& fileOut,
  const string& fileRef,
  const formatType fOrig,
  const formatType fRef,
  ValStatType vstats[][BRIDGE_FORMAT_LABELS_SIZE]);

void printOverallStats(
  ValStatType vstats[][BRIDGE_FORMAT_LABELS_SIZE]);

#endif
