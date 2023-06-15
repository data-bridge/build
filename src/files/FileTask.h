/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_FILETASK_H
#define BRIDGE_FILETASK_H

#include <vector>
#include <string>

enum Format: unsigned;

using namespace std;


struct FileOutputTask
{
  string fileOutput;
  Format formatOutput;

  bool refFlag;
  string fileRef;
};


struct FileTask
{
  string fileInput;
  Format formatInput;
  bool removeOutputFlag;

  vector<FileOutputTask> taskList;
};

#endif

