/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALINT_H
#define BRIDGE_VALINT_H

#include <string>

using namespace std;


// A single example of a difference of any kind.

struct valSide
{
  string line;
  unsigned lno;
};

struct ValExample
{
  valSide out;
  valSide ref;
};

// Counts of differences between two files, with the first examples.

struct ValFileStats
{
  unsigned counts[BRIDGE_VAL_SIZE];
  ValExample examples[BRIDGE_VAL_SIZE];
};


bool valProgress(
  ifstream& fstr,
  valSide& side);

void valError(
  ValFileStats& stats,
  const ValExample& running,
  const ValDiffs label);

#endif
