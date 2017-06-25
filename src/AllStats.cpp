/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "AllStats.h"


void mergeResults(
  vector<AllStats>& allStatsList,
  const Options& options)
{
  if (options.numThreads == 1)
    return;

  for (unsigned i = 1; i < options.numThreads; i++)
  {
    allStatsList[0].vstats += allStatsList[i].vstats;
    allStatsList[0].tstats += allStatsList[i].tstats;
    allStatsList[0].cstats += allStatsList[i].cstats;
    allStatsList[0].timers += allStatsList[i].timers;
    allStatsList[0].duplstats += allStatsList[i].duplstats;
    allStatsList[0].refstats += allStatsList[i].refstats;
  }

  if (! options.fileLog.setFlag)
    return;

  ofstream fbase(options.fileLog.name, std::ofstream::app);
  if (! fbase.is_open())
    return;

  string line;
  for (unsigned i = 1; i < options.numThreads; i++)
  {
    const string fname = options.fileLog.name + STR(i);
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


void printResults(
  const AllStats& allStats,
  const Options& options)
{
  if (options.equalFlag)
    allStats.duplstats.print(cout);
  allStats.vstats.print(cout, options.verboseValStats);
  if (options.statsFlag)
    allStats.tstats.print(cout, true); // Can add switch to control
  if (options.compareFlag)
    allStats.cstats.print(cout);
  if (options.quoteFlag)
    allStats.refstats.print(cout);

  allStats.timers.print(options.numThreads);
}

