/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FUNCWRITE_H
#define BRIDGE_FUNCWRITE_H

#include <iostream>
#include <string>

class Group;

enum Format: unsigned;
enum BoardOrder: unsigned;

using namespace std;


void setWriteTables();

void dispatchWrite(
  const string& fname,
  const Format format,
  const BoardOrder order,
  const Group& group,
  string& text,
  ostream& flog);

#endif
