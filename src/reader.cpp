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
#include "validate.h"
#include "Debug.h"


OptionsType options;
Debug debug;

static const int numThreads = 1;


int main(int argc, char * argv[])
{
  ValStatType vstats[BRIDGE_FORMAT_LABELS_SIZE][BRIDGE_FORMAT_LABELS_SIZE];
  // validate("tmp/S10FA1.TXT", "S10FA1.TXT",
    // BRIDGE_FORMAT_RBN, BRIDGE_FORMAT_TXT,
  validate("tmp/S10FA1.REC", "S10FA1.REC",
    BRIDGE_FORMAT_RBN, BRIDGE_FORMAT_REC,
    vstats);
  exit(0);

  cout << "OVerall stats:\n";
  printOverallStats(vstats);


  readArgs(argc, argv);

  setTables();

  Files files;
  files.Set(options);

  thread thr[numThreads];
  for (int i = 0; i < numThreads; i++)
    thr[i] = thread(dispatch, i, files);

  for (int i = 0; i < numThreads; i++)
    thr[i].join();
}

