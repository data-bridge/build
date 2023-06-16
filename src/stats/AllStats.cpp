/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "AllStats.h"

#include "ValStats.h"
#include "TextStats.h"
#include "CompStats.h"
#include "RefStats.h"
#include "DuplStats.h"
#include "Timers.h"

#include "../control/Options.h"


// This is not a struct method.

void mergeLogFiles(const Options& options)
{
  ofstream fbase(options.fileLog.name, std::ofstream::app);
  if (! fbase.is_open())
    return;

  string line;
  for (unsigned i = 1; i < options.numThreads; i++)
  {
    const string fname = options.fileLog.name + to_string(i);
    ifstream flog(fname);
    if (! flog.is_open())
      continue;

    fbase << "From thread " << i << ":\n";
    fbase << "-------------\n\n";

    while (getline(flog, line))
      fbase << line;

    flog.close();
    remove(fname.c_str());
  }
  fbase.close();
}


// This is not a struct method.

void mergeResults(
  vector<AllStats>& allStatsList,
  const Options& options)
{
  if (options.numThreads == 1)
    return;

  for (unsigned i = 1; i < options.numThreads; i++)
    allStatsList[0] += allStatsList[i];

  if (options.equalFlag)
    allStatsList[0].duplStatsPtr->sortOverall();

  if (options.fileLog.setFlag)
    mergeLogFiles(options);
}


AllStats::AllStats()
{
  valStatsPtr = new ValStats;
  textStatsPtr = new TextStats;
  compStatsPtr = new CompStats;
  refStatsPtr = new RefStats;
  duplStatsPtr = new DuplStats;
  timersPtr = new Timers;
}


AllStats::~AllStats()
{
  delete valStatsPtr;
  delete textStatsPtr;
  delete compStatsPtr;
  delete refStatsPtr;
  delete duplStatsPtr;
  delete timersPtr;
}


void AllStats::operator += (const AllStats& as2)
{
  * valStatsPtr += * as2.valStatsPtr;
  * textStatsPtr += * as2.textStatsPtr;
  * compStatsPtr += * as2.compStatsPtr;
  * refStatsPtr += * as2.refStatsPtr;
  * duplStatsPtr += * as2.duplStatsPtr;
  * timersPtr += * as2.timersPtr;
}


string AllStats::str(const Options& options) const
{
  stringstream ss;

  if (options.equalFlag)
    ss << duplStatsPtr->str();

  ss << valStatsPtr->str(options.verboseValStats);

  if (options.statsFlag)
    ss << textStatsPtr->str(options.verboseTextStats);

  if (options.compareFlag)
    ss << compStatsPtr->str();

  if (options.quoteFlag)
    ss << refStatsPtr->str();

  ss << timersPtr->str(options.numThreads);
  
  return ss.str();
}

