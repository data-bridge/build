/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <fstream>

#include "Group.h"
#include "dispatch.h"
#include "validate.h"
#include "Files.h"
#include "AllStats.h"
#include "RefLines.h"

#include "funcCompare.h"
#include "funcDigest.h"
#include "funcIMPSheet.h"
#include "funcRead.h"
#include "funcPlayerVal.h"
#include "funcRefStats.h"
#include "funcTextStats.h"
#include "funcValidate.h"
#include "funcWrite.h"


void setTables()
{
  setReadTables();
  setWriteTables();
  setValidateTables();
}


void dispatch(
  const int thrNo,
  Files& files,
  const Options& options,
  AllStats& allStats)
{
  ofstream freal;
  if (options.fileLog.setFlag)
    freal.open(options.fileLog.name + (thrNo == 0 ? "" : STR(thrNo)));
  ostream& flog = (options.fileLog.setFlag ? freal : cout);

  FileTask task;
  string text;
  text.reserve(100000);

  RefLines refLines;

  while (files.next(task))
  {
    if (options.verboseIO)
      flog << "Input " << task.fileInput << endl;

    Group group;

    if (options.fileDigest.setFlag || options.dirDigest.setFlag)
      goto DIGEST;

    refLines.reset();
    allStats.timers.start(BRIDGE_TIMER_READ, task.formatInput);
    bool b = dispatchReadFile(task.fileInput, task.formatInput, 
      options, group, refLines, flog);
    allStats.timers.stop(BRIDGE_TIMER_READ, task.formatInput);
    if (! b)
    {
      flog << "Failed to read " << task.fileInput << endl;
      continue;
    }

    if (options.quoteFlag)
    {
      if (options.verboseIO)
        flog << "Ref file for " << task.fileInput << endl;
    
      allStats.timers.start(BRIDGE_TIMER_REF_STATS, task.formatInput);
      dispatchRefStats(task.fileInput, task.formatInput, 
        refLines, allStats.refstats, flog);
      allStats.timers.stop(BRIDGE_TIMER_REF_STATS, task.formatInput);
    }

    if (refLines.skip())
      continue;

    if (options.playersFlag)
      dispatchPlayersValidate(group, flog);

    if (options.statsFlag)
    {
      if (options.verboseIO)
        flog << "Input " << task.fileInput << endl;
    
      allStats.timers.start(BRIDGE_TIMER_STATS, task.formatInput);
      dispatchTextStats(task, group, allStats.tstats, flog);
      allStats.timers.stop(BRIDGE_TIMER_STATS, task.formatInput);
    }

    for (auto &t: task.taskList)
    {
      if (options.verboseIO && t.fileOutput != "")
        flog << "Output " << t.fileOutput << endl;

      allStats.timers.start(BRIDGE_TIMER_WRITE, t.formatOutput);
      dispatchWrite(t.fileOutput, t.formatOutput, refLines.order(), 
        group, text, flog);
      allStats.timers.stop(BRIDGE_TIMER_WRITE, t.formatOutput);

      if (t.refFlag && refLines.validate())
      {
        if (options.verboseIO)
          flog << "Validating " << t.fileOutput <<
              " against " << t.fileRef << endl;

        allStats.timers.start(BRIDGE_TIMER_VALIDATE, t.formatOutput);
        dispatchValidate(task, t, options, text, allStats.vstats, flog);
        allStats.timers.stop(BRIDGE_TIMER_VALIDATE, t.formatOutput);
      }

      if (options.compareFlag && task.formatInput == t.formatOutput)
      {
        if (options.verboseIO)
          flog << "Comparing " << t.fileOutput <<
              " against " << task.fileInput << endl;

        allStats.timers.start(BRIDGE_TIMER_COMPARE, t.formatOutput);
        dispatchCompare(task.fileInput, task.formatInput, options,
          text, group, allStats.cstats, flog);
        allStats.timers.stop(BRIDGE_TIMER_COMPARE, t.formatOutput);
      }

      if (task.removeOutputFlag)
        remove(t.fileOutput.c_str());
    }

    DIGEST:
    if (options.fileDigest.setFlag || options.dirDigest.setFlag)
    {
      if (options.verboseIO)
        flog << "Digest input " << task.fileInput << endl;
    
      allStats.timers.start(BRIDGE_TIMER_DIGEST, task.formatInput);
      dispatchDigest(task, options, flog);
      allStats.timers.stop(BRIDGE_TIMER_DIGEST, task.formatInput);
    }

    if (options.tableIMPFlag)
      dispatchIMPSheet(group, flog);
  }
}

