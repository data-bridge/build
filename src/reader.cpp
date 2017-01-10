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
#include "dispatch.h"

using namespace std;


int main(int argc, char * argv[])
{
  Options options;
  readArgs(argc, argv, options);

  setTables();

  Files files;
  files.set(options);

  vector<thread> thr(options.numThreads);
  vector<ValStats> vstats(options.numThreads);
  vector<TextStats> tstats(options.numThreads);
  vector<CompStats> cstats(options.numThreads);
  vector<Timers> timers(options.numThreads);

  Timer timer;
  timer.start();

  for (unsigned i = 0; i < options.numThreads; i++)
    thr[i] = thread(dispatch, i, 
      ref(files), 
      options, 
      ref(vstats[i]), 
      ref(tstats[i]), 
      ref(cstats[i]), 
      ref(timers[i]));

  for (unsigned i = 0; i < options.numThreads; i++)
    thr[i].join();

  timer.stop();

  mergeResults(vstats, tstats, cstats, timers, options);

  vstats[0].print(cout, options.verboseValStats);
  if (options.statsFlag)
    tstats[0].print(cout, true); // Add switch to control
  if (options.compareFlag)
    cstats[0].print(cout);

  cout << "Time spent overall (elapsed): " << timer.str(2) << "\n";
  timers[0].print(options.numThreads);
}

