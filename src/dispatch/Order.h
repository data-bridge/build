/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_ORDER_H
#define BRIDGE_ORDER_H

#include <string>

using namespace std;


struct Counts
{
  unsigned segno;
  unsigned bno;
  unsigned prevno;
  bool openFlag;
};

enum BoardOrder: unsigned
{
  ORDER_OCOC = 0,
  ORDER_COCO = 1,
  ORDER_OOCC = 2,
  ORDER_GENERAL = 3
};

const string orderNames[] =
{
  "OCOC",
  "COCO",
  "OOCC",
  "General"
};

#endif

