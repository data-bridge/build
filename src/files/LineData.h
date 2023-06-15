/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_LINEDATA_H
#define BRIDGE_LINEDATA_H

#include <string>

using namespace std;


enum LineType
{
  BRIDGE_BUFFER_STRUCTURED = 0,
  BRIDGE_BUFFER_EMPTY = 1,
  BRIDGE_BUFFER_DASHES = 2,
  BRIDGE_BUFFER_COMMENT = 3,
  BRIDGE_BUFFER_GENERAL = 4,
  BRIDGE_BUFFER_SIZE = 5
};

struct LineData
{
  string line;
  unsigned len;
  unsigned no;
  LineType type;
  string label;
  string value;
};

#endif
