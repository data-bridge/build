/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "dispatch.h"

#include "../control/Options.h"

#include "../records/Group.h"

#include "../files/Files.h"

#include "../edits/RefLines.h"

#include "../stats/AllStats.h"

#include "../validate/validate.h"

#include "funcCompare.h"
#include "funcDD.h"
#include "funcDigest.h"
#include "funcDupl.h"
#include "funcIMPSheet.h"
#include "funcPasses.h"
#include "funcRead.h"
#include "funcPlayerVal.h"
#include "funcRefStats.h"
#include "funcTextStats.h"
#include "funcTrace.h"
#include "funcValidate.h"
#include "funcValuation.h"
#include "funcWrite.h"

#include "../stats/Timers.h"


void setTables()
{
  setReadTables();
  setWriteTables();
  setValidateTables();
}


void dispatch(
  const size_t thrNo,
  Files& files,
  const Options& options,
  AllStats& allStats)
{
  ofstream freal;
  if (options.fileLog.setFlag)
    freal.open(options.fileLog.name + (thrNo == 0 ? "" : to_string(thrNo)));
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
    allStats.timersPtr->start(BRIDGE_TIMER_READ, task.formatInput);
    bool b = dispatchReadFile(task.fileInput, task.formatInput, 
      options, group, refLines, flog);
    allStats.timersPtr->stop(BRIDGE_TIMER_READ, task.formatInput);
    if (! b)
    {
      flog << "Failed to read " << task.fileInput << endl;
      continue;
    }

    if (options.quoteFlag)
    {
      if (options.verboseIO)
        flog << "Ref file for " << task.fileInput << endl;
    
      allStats.timersPtr->start(BRIDGE_TIMER_REF_STATS, task.formatInput);
      dispatchRefStats(task.fileInput, task.formatInput, 
        refLines, * allStats.refStatsPtr, flog);
      allStats.timersPtr->stop(BRIDGE_TIMER_REF_STATS, task.formatInput);
    }

    if (refLines.skip())
      continue;

    if (options.playersFlag)
      dispatchPlayersValidate(group, flog);

    if (options.solveFlag)
      dispatchDD(group, files, task.fileInput, flog);

    if (options.traceFlag)
      dispatchTrace(group, files, task.fileInput, flog);

    if (options.statsFlag)
    {
      if (options.verboseIO)
        flog << "Input " << task.fileInput << endl;
    
      allStats.timersPtr->start(BRIDGE_TIMER_STATS, task.formatInput);
      dispatchTextStats(task, group, * allStats.textStatsPtr, flog);
      allStats.timersPtr->stop(BRIDGE_TIMER_STATS, task.formatInput);
    }

    if (options.valuationFlag)
    {
      if (options.verboseIO)
        flog << "Valuation " << task.fileInput << endl;

      allStats.timersPtr->start(BRIDGE_TIMER_VALUE, task.formatInput);
      dispatchValuation(group, flog);
      allStats.timersPtr->stop(BRIDGE_TIMER_VALUE, task.formatInput);

      if (options.passStatsFlag)
      {
        if (options.verboseIO)
          flog << "Pass stats\n" << endl;
        
        allStats.timersPtr->start(BRIDGE_TIMER_PASS, task.formatInput);
        dispatchPasses(group, options,
          * allStats.paramStats1DPtr, * allStats.paramStats2DPtr, flog);
        allStats.timersPtr->stop(BRIDGE_TIMER_PASS, task.formatInput);
      }
    }

    if (options.equalFlag)
    {
      if (options.verboseIO)
        flog << "Hand hashes " << task.fileInput << endl;

      dispatchDupl(group, refLines, * allStats.duplStatsPtr, flog);
    }

    for (auto &t: task.taskList)
    {
      if (options.verboseIO && t.fileOutput != "")
        flog << "Output " << t.fileOutput << endl;

      allStats.timersPtr->start(BRIDGE_TIMER_WRITE, t.formatOutput);
      dispatchWrite(t.fileOutput, t.formatOutput, refLines.order(), 
        group, text, flog);
      allStats.timersPtr->stop(BRIDGE_TIMER_WRITE, t.formatOutput);

      if (t.refFlag && refLines.validate())
      {
        if (options.verboseIO)
          flog << "Validating " << t.fileOutput <<
              " against " << t.fileRef << endl;

        allStats.timersPtr->start(BRIDGE_TIMER_VALIDATE, t.formatOutput);
        dispatchValidate(task, t, options, text, 
          * allStats.valStatsPtr, flog);
        allStats.timersPtr->stop(BRIDGE_TIMER_VALIDATE, t.formatOutput);
      }

      if (options.compareFlag && task.formatInput == t.formatOutput)
      {
        if (options.verboseIO)
          flog << "Comparing " << t.fileOutput <<
              " against " << task.fileInput << endl;

        allStats.timersPtr->start(BRIDGE_TIMER_COMPARE, t.formatOutput);
        dispatchCompare(task.fileInput, task.formatInput, options,
          text, group, * allStats.compStatsPtr, flog);
        allStats.timersPtr->stop(BRIDGE_TIMER_COMPARE, t.formatOutput);
      }

      if (task.removeOutputFlag)
        remove(t.fileOutput.c_str());
    }

    DIGEST:
    if (options.fileDigest.setFlag || options.dirDigest.setFlag)
    {
      if (options.verboseIO)
        flog << "Digest input " << task.fileInput << endl;
    
      allStats.timersPtr->start(BRIDGE_TIMER_DIGEST, task.formatInput);
      dispatchDigest(task, options, flog);
      allStats.timersPtr->stop(BRIDGE_TIMER_DIGEST, task.formatInput);
    }

    if (options.tableIMPFlag)
      dispatchIMPSheet(group, flog);
  }
}

