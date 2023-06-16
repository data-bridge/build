/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCVALIDATE_H
#define BRIDGE_FUNCVALIDATE_H

#include <iostream>
#include <string>

struct FileTask;
struct FileOutputTask;
struct Options;
class ValStats;

using namespace std;


void dispatchValidate(
  const FileTask& task,
  const FileOutputTask& otask,
  const Options& options,
  const string& text,
  ValStats& vstats,
  ostream& flog);

#endif
