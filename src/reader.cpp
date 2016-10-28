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


OptionsType options;

static const int numThreads = 1;


int main(int argc, char * argv[])
{
  readArgs(argc, argv);

  setTables();

  Files files;
  files.Set(options);

  thread thr[numThreads];
  for (int i = 0; i < numThreads; i++)
    thr[i] = thread(dispatch, i, files, options);

  for (int i = 0; i < numThreads; i++)
    thr[i].join();
}

