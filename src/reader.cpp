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
#include "Bexcept.h"
#include "Debug.h"


OptionsType options;
Debug debug;

static const int numThreads = 1;


int main(int argc, char * argv[])
{
  ValStatType vstats[BRIDGE_FORMAT_LABELS_SIZE][BRIDGE_FORMAT_LABELS_SIZE];

  /*
  validate("tmp/W96FA5.TXT", "W96FA5.TXT",
    BRIDGE_FORMAT_RBN, BRIDGE_FORMAT_TXT,
    vstats);
  exit(0);

  cout << "OVerall stats:\n";
  printOverallStats(vstats);
  exit(0);
  */

  readArgs(argc, argv);

  setTables();

  Files files;
  files.Set(options);

  thread thr[numThreads];
  for (int i = 0; i < numThreads; i++)
    thr[i] = thread(dispatch, i, files, options);

  for (int i = 0; i < numThreads; i++)
    thr[i].join();
  
  files.Rewind();
  FileTaskType task;
  while (files.GetNextTask(task))
  {
    for (auto &t: task.taskList)
    {
      try
      {
        validate(t.fileOutput, t.fileRef,
          task.formatInput, t.formatOutput, options, vstats);
      }
      catch(Bexcept& bex)
      {
        bex.Print();
        exit(0);
      }
    }
  }

  cout << "Overall stats:\n";
  printOverallStats(vstats, options.verboseValStats);
}

