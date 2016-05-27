/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_PARSE_H
#define BRIDGE_PARSE_H

#include <string>
#include <vector>

using namespace std;

void tokenize(
  const std::string& str,
  vector<std::string>& tokens,
  const std::string& delimiters);

#endif
