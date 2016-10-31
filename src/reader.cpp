/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <thread>

using namespace std;

#include "args.h"
#include "dispatch.h"



int main(int argc, char * argv[])
{
  OptionsType options;
  readArgs(argc, argv, options);

  setTables();

  Files files;
  files.Set(options);

  vector<thread> thr(options.numThreads);
  vector<ValStats> vstats(options.numThreads);
  vector<Timer> timer(options.numThreads);

  for (unsigned i = 0; i < options.numThreads; i++)
    thr[i] = thread(dispatch, i, 
      ref(files), options, ref(vstats[i]), ref(timer[i]));

  for (unsigned i = 0; i < options.numThreads; i++)
    thr[i].join();

  mergeResults(vstats, timer, options);

  vstats[0].print(cout, options.verboseValStats);
  timer[0].print(options.numThreads);
}

