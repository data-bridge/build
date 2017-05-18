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


enum ActionCategory
{
  ACTION_DELETE_LINE = 0,
  ACTION_INSERT_LINE = 1,
  ACTION_GENERAL = 2,
  ACTION_ERROR = 3
};


class RefAction
{
  private:

    string filename;
    ActionType action;
    string name;

    void setTables();


  public:

    RefAction();

    ~RefAction();

    void reset();

    void set(
      const string& refName,
      const ActionType actionIn);

    void set(
      const string& refName,
      const string& str);

    ActionType number() const;

    ActionCategory category() const;

    string str() const;
};

#endif
