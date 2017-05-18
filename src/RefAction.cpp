/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif

#include "RefAction.h"
#include "Bexcept.h"

using namespace std;


static map<string, ActionType> ActionMap;

static string ActionTable[REF_ACTION_SIZE];

static ActionCategory ActionCatTable[REF_ACTION_SIZE];


static mutex mtx;
static bool setActionTables = false;


RefAction::RefAction()
{
  RefAction::reset();
  if (! setActionTables)
  {
    mtx.lock();
    if (! setActionTables)
      RefAction::setTables();
    setActionTables = true;
    mtx.unlock();
  }
}


RefAction::~RefAction()
{
}


void RefAction::reset()
{
  action = REF_ACTION_SIZE;
  name = "";
}


void RefAction::setTables()
{
  ActionMap["replace"] = REF_ACTION_REPLACE_GEN;
  ActionMap["insert"] = REF_ACTION_INSERT_GEN;
  ActionMap["delete"] = REF_ACTION_DELETE_GEN;

  ActionMap["replaceLIN"] = REF_ACTION_REPLACE_LIN;
  ActionMap["insertLIN"] = REF_ACTION_INSERT_LIN;
  ActionMap["deleteLIN"] = REF_ACTION_DELETE_LIN;

  ActionMap["replacePBN"] = REF_ACTION_REPLACE_PBN;
  ActionMap["insertPBN"] = REF_ACTION_INSERT_PBN;
  ActionMap["deletePBN"] = REF_ACTION_DELETE_PBN;

  ActionMap["replaceRBN"] = REF_ACTION_REPLACE_RBN;
  ActionMap["insertRBN"] = REF_ACTION_INSERT_RBN;
  ActionMap["deleteRBN"] = REF_ACTION_DELETE_RBN;

  ActionMap["replaceRBX"] = REF_ACTION_REPLACE_RBX;
  ActionMap["insertRBX"] = REF_ACTION_INSERT_RBX;
  ActionMap["deleteRBX"] = REF_ACTION_DELETE_RBX;

  ActionMap["replaceTXT"] = REF_ACTION_REPLACE_TXT;
  ActionMap["insertTXT"] = REF_ACTION_INSERT_TXT;
  ActionMap["deleteTXT"] = REF_ACTION_DELETE_TXT;

  ActionMap["replaceWORD"] = REF_ACTION_REPLACE_WORD;
  ActionMap["insertWORD"] = REF_ACTION_INSERT_WORD;
  ActionMap["deleteWORD"] = REF_ACTION_DELETE_WORD;

  for (auto &s: ActionMap)
    ActionTable[s.second] = s.first;

  for (unsigned i = 0; i < REF_ACTION_SIZE; i++)
    ActionCatTable[i] = ACTION_GENERAL;

  ActionCatTable[REF_ACTION_INSERT_GEN] = ACTION_INSERT_LINE;
  ActionCatTable[REF_ACTION_DELETE_GEN] = ACTION_DELETE_LINE;

  ActionCatTable[REF_ACTION_INSERT_PBN] = ACTION_INSERT_LINE;
  ActionCatTable[REF_ACTION_DELETE_PBN] = ACTION_DELETE_LINE;

  ActionCatTable[REF_ACTION_DELETE_RBN] = ACTION_DELETE_LINE;
}


void RefAction::set(
  const string& refName,
  const ActionType actionIn)
{
  if (actionIn == REF_ACTION_SIZE)
    THROW("File " + refName + ": Bad input action");

  filename = refName;
  action = actionIn;
  name = ActionTable[action];
}


void RefAction::set(
  const string& refName,
  const string& str)
{
  auto it = ActionMap.find(str);
  if (it == ActionMap.end())
    THROW("File " + refName + ": Bad input action " + str);

  filename = refName;
  action = it->second;
  name = str;
}


ActionType RefAction::number() const
{
  return action;
}


ActionCategory RefAction::category() const
{
  if (action == REF_ACTION_SIZE)
    return ACTION_ERROR;
  else
    return ActionCatTable[action];
}


string RefAction::str() const
{
  return name;
}

