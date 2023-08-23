/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <thread>

#include "control/args.h"
#include "files/Files.h"
#include "dispatch/dispatch.h"
#include "dispatch/funcPasses.h" // TODO TMP for passPostprocess
#include "stats/AllStats.h"
#include "stats/Timer.h"


using namespace std;


int main(int argc, char * argv[])
{
  Options options;
  readArgs(argc, argv, options);

  setTables();

  Files files;
  files.set(options);

  vector<thread> thr(options.numThreads);
  vector<AllStats> allStatsList(options.numThreads);

  Timer timer;
  timer.start();

  for (size_t i = 0; i < options.numThreads; i++)
    thr[i] = thread(dispatch, i, ref(files), options, ref(allStatsList[i]));

  for (size_t i = 0; i < options.numThreads; i++)
    thr[i].join();

  mergeResults(allStatsList, options);
  cout << allStatsList[0].str(options);

  // if (options.passStatsFlag)
    // passPostprocess(* allStatsList[0].paramStats1DPtr);
  if (options.solveFlag)
    files.writeDDInfo(BRIDGE_DD_INFO_SOLVE);
  if (options.traceFlag)
    files.writeDDInfo(BRIDGE_DD_INFO_TRACE);

  timer.stop();

  cout << "Time spent overall (elapsed): " << timer.str(2) << "\n";
}

