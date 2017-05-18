/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFACTION_H
#define BRIDGE_REFACTION_H

#include <string>
#include "refconst.h"

using namespace std;


class RefAction
{
  private:

    string filename;
    FixType action;
    string name;

    void setTables();


  public:

    RefAction();

    ~RefAction();

    void reset();

    void set(
      const string& refName,
      const FixType actionIn);

    void set(
      const string& refName,
      const string& str);

    FixType number() const;

    string str() const;
};

#endif
