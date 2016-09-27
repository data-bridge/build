/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FILES_H
#define BRIDGE_FILES_H

#include "bconst.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

using namespace std;


class Files
{
  private:

    vector<FileTaskType> fileTasks;
    unsigned len;


  public:

    Files();

    ~Files();

    void Reset();

    void Set(const OptionsType& options);

    bool GetNextTask(FileTaskType& ftask) const;

};

#endif

