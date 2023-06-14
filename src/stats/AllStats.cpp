/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <string>
#include <fstream>

#include "AllStats.h"

#include "../Options.h"


void mergeResults(
  vector<AllStats>& allStatsList,
  const Options& options)
{
  if (options.numThreads == 1)
    return;

  for (unsigned i = 1; i < options.numThreads; i++)
    allStatsList[0] += allStatsList[i];

  if (options.equalFlag)
    allStatsList[0].duplstats.sortOverall();

  if (! options.fileLog.setFlag)
    return;

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


void printResults(
  const AllStats& allStats,
  const Options& options)
{
  if (options.equalFlag)
    allStats.duplstats.print(cout);
  cout << allStats.vstats.str(options.verboseValStats);
  if (options.statsFlag)
    cout << allStats.tstats.str(options.verboseTextStats);
  if (options.compareFlag)
    allStats.cstats.print(cout);
  if (options.quoteFlag)
    cout << allStats.refstats.str();

  allStats.timers.print(options.numThreads);
}

