/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
#else
  #include <thread>
#endif
#pragma warning(pop)

#include "args.h"
#include "Files.h"
#include "AllStats.h"

#include "dispatch/dispatch.h"

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
  printResults(allStatsList[0], options);

  if (options.solveFlag)
    files.writeDDInfo(BRIDGE_DD_INFO_SOLVE);
  if (options.traceFlag)
    files.writeDDInfo(BRIDGE_DD_INFO_TRACE);

  timer.stop();

  cout << "Time spent overall (elapsed): " << timer.str(2) << "\n";
}

