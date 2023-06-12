/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
#else
  #include <thread>
#endif

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

  timer.stop();

  mergeResults(allStatsList, options);
  printResults(allStatsList[0], options);

  cout << "Time spent overall (elapsed): " << timer.str(2) << "\n";
}

