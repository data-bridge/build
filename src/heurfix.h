/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_HEURFIX_H
#define BRIDGE_HEURFIX_H

#include <string>
#include <vector>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "Buffer.h"
#include "bconst.h"

using namespace std;

void heurFixTricks(
  Group& group,
  Segment * segment,
  Board * board,
  const Buffer& buffer,
  const vector<string>& chunk,
  const Counts& counts,
  const Options& options);

void heurFixPlayDD(
  Group& group,
  Segment * segment,
  Board * board,
  const Buffer& buffer,
  const vector<string>& chunk,
  const Options& options);

#endif
