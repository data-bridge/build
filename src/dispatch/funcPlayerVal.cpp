/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iomanip>
#include <sstream>
#include <fstream>
#pragma warning(pop)

#include "../records/Group.h"

#include "funcPlayerVal.h"

#include "../handling/Bexcept.h"


static void validatePlayers(
  const Group& group,
  ostream& flog)
{
  unsigned numSegs = 0;
  unsigned numCombos = 0;
  bool overlaps = false;
  bool nonFull = false;
  vector<unsigned> completions(65);
  completions.clear();

  for (auto &segment: group)
  {
    numSegs++;
    for (auto &bpair: segment)
    {
      const Board& board = bpair.board;
      unsigned c = 0;
      for (unsigned i = 0; i < board.countAll(); i++)
      {
        const Instance& instance = board.getInstance(i);
        c *= 8;
        if (! board.skipped(i))
          c += instance.missingPlayers();
      }
      if (c > 65)
        THROW("Bad combo: " + to_string(c));
      if (c > 0)
        nonFull = true;
      if (completions[c] == 0)
        numCombos++;

      completions[c]++;

      if (board.overlappingPlayers())
        overlaps = true;
    }
  }

  if (numSegs > 1)
  {
    flog << "Input: " << group.name() << "\n";
    flog << "Number of segments: " << numSegs << "\n\n";
  }

  if (overlaps || nonFull)
  {
    if (overlaps)
      flog << "File " << group.name() << ": Overlaps exist\n\n";
    if (nonFull)
      flog << "File " << group.name() << ": Incomplete players exist\n\n";
    for (auto &segment: group)
    {
      flog << segment.strTitle(BRIDGE_FORMAT_LIN_VG);
      flog << segment.strPlayers(BRIDGE_FORMAT_LIN_VG) << "\n";
    }
  }

  if (numCombos <= 1)
    return;

  flog << "Input: " << group.name() << "\n";

  for (auto &segment: group)
  {
    flog << segment.strTitle(BRIDGE_FORMAT_LIN_VG);
    flog << segment.strPlayers(BRIDGE_FORMAT_LIN_VG);
  }

  for (unsigned i = 0; i < 65; i++)
  {
    if (completions[i] == 0)
      continue;
    flog << i/8 << ", " << (i % 8) << ": " << completions[i] << "\n";
  }
  flog << "\n";
}


void dispatchPlayersValidate(
  const Group& group,
  ostream& flog)
{
  try
  {
    validatePlayers(group, flog);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

