/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <thread>

#include "args.h"
#include "dispatch.h"

using namespace std;



int main(int argc, char * argv[])
{
  OptionsType options;
  readArgs(argc, argv, options);

  setTables();

  Files files;
  files.Set(options);

  vector<thread> thr(options.numThreads);
  vector<ValStats> vstats(options.numThreads);
  vector<Timers> timers(options.numThreads);

  Timer timer;
  timer.start();

  for (unsigned i = 0; i < options.numThreads; i++)
    thr[i] = thread(dispatch, i, 
      ref(files), options, ref(vstats[i]), ref(timers[i]));

  for (unsigned i = 0; i < options.numThreads; i++)
    thr[i].join();

  timer.stop();

  mergeResults(vstats, timers, options);

  vstats[0].print(cout, options.verboseValStats);

  cout << "Time spent overall (elapsed): " << timer.str(2) << "\n";
  timers[0].print(options.numThreads);
}

