/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCREAD_H
#define BRIDGE_FUNCREAD_H

#include <iostream>
#include <string>

struct Options;
class Buffer;
class Group;
class RefLines;

enum Format: unsigned;
enum BoardOrder: unsigned;

using namespace std;


void setReadTables();

bool dispatchReadBuffer(
  const Format format,
  const Options& options,
  Buffer& buffer,
  Group& group,
  BoardOrder& order,
  ostream& flog);

bool dispatchReadFile(
  const string& fname,
  const Format format,
  const Options& options,
  Group& group,
  RefLines& refLines,
  ostream& flog);

#endif
