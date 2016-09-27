/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READEML_H
#define BRIDGE_READEML_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setEMLtables();

bool readEML(
  Group& group,
  const string& fname);

bool writeEML(
  Group& group,
  const string& fname);


#endif
