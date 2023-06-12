/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "../Group.h"
#include "../TextStats.h"
#include "funcTextStats.h"
#include "../Bexcept.h"


static void logLengths(
  const string& fname,
  const Format format,
  const Group& group,
  TextStats& tstats)
{
  for (auto &segment: group)
  {
    tstats.add(segment.strTitle(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_TITLE, format);

    tstats.add(segment.strLocation(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_LOCATION, format);

    tstats.add(segment.strEvent(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_EVENT, format);

    tstats.add(segment.strSession(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_SESSION, format);

    tstats.add(segment.strFirstTeam(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_TEAMS, format);

    tstats.add(segment.strSecondTeam(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_TEAMS, format);

    for (auto &bpair: segment)
    {
      const Board& board = bpair.board;

      for (unsigned i = 0; i < board.countAll(); i++)
      {
        const Instance& instance = board.getInstance(i);

        tstats.add(instance.lengthAuction(),
          fname, BRIDGE_FORMAT_AUCTION, format);

        for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
        {
          tstats.add(
            instance.strPlayer(static_cast<Player>(p), BRIDGE_FORMAT_PAR),
            fname, BRIDGE_FORMAT_PLAYERS, format);
        }
      }
    }
  }
}


void dispatchTextStats(
  const FileTask& task,
  const Group& group,
  TextStats& tstats,
  ostream& flog)
{
  try
  {
    logLengths(task.fileInput, task.formatInput, group, tstats);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

