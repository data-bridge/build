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

static string ActionTable[BRIDGE_REF_FIX_SIZE];

static ActionCategory ActionCatTable[BRIDGE_REF_FIX_SIZE];


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
  action = BRIDGE_REF_FIX_SIZE;
  name = "";
}


void RefAction::setTables()
{
  ActionMap["replace"] = BRIDGE_REF_REPLACE_GEN;
  ActionMap["insert"] = BRIDGE_REF_INSERT_GEN;
  ActionMap["delete"] = BRIDGE_REF_DELETE_GEN;

  ActionMap["replaceLIN"] = BRIDGE_REF_REPLACE_LIN;
  ActionMap["insertLIN"] = BRIDGE_REF_INSERT_LIN;
  ActionMap["deleteLIN"] = BRIDGE_REF_DELETE_LIN;

  ActionMap["replacePBN"] = BRIDGE_REF_REPLACE_PBN;
  ActionMap["insertPBN"] = BRIDGE_REF_INSERT_PBN;
  ActionMap["deletePBN"] = BRIDGE_REF_DELETE_PBN;

  ActionMap["replaceRBN"] = BRIDGE_REF_REPLACE_RBN;
  ActionMap["insertRBN"] = BRIDGE_REF_INSERT_RBN;
  ActionMap["deleteRBN"] = BRIDGE_REF_DELETE_RBN;

  ActionMap["replaceRBX"] = BRIDGE_REF_REPLACE_RBX;
  ActionMap["insertRBX"] = BRIDGE_REF_INSERT_RBX;
  ActionMap["deleteRBX"] = BRIDGE_REF_DELETE_RBX;

  ActionMap["replaceTXT"] = BRIDGE_REF_REPLACE_TXT;
  ActionMap["insertTXT"] = BRIDGE_REF_INSERT_TXT;
  ActionMap["deleteTXT"] = BRIDGE_REF_DELETE_TXT;

  ActionMap["replaceWORD"] = BRIDGE_REF_REPLACE_WORD;
  ActionMap["insertWORD"] = BRIDGE_REF_INSERT_WORD;
  ActionMap["deleteWORD"] = BRIDGE_REF_DELETE_WORD;

  for (auto &s: ActionMap)
    ActionTable[s.second] = s.first;

  for (unsigned i = 0; i < BRIDGE_REF_FIX_SIZE; i++)
    ActionCatTable[i] = ACTION_GENERAL;

  ActionCatTable[BRIDGE_REF_INSERT_GEN] = ACTION_INSERT_LINE;
  ActionCatTable[BRIDGE_REF_DELETE_GEN] = ACTION_DELETE_LINE;

  ActionCatTable[BRIDGE_REF_INSERT_PBN] = ACTION_INSERT_LINE;
  ActionCatTable[BRIDGE_REF_DELETE_PBN] = ACTION_DELETE_LINE;

  ActionCatTable[BRIDGE_REF_DELETE_RBN] = ACTION_DELETE_LINE;
}


void RefAction::set(
  const string& refName,
  const ActionType actionIn)
{
  if (actionIn == BRIDGE_REF_FIX_SIZE)
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
  if (action == BRIDGE_REF_FIX_SIZE)
    return ACTION_ERROR;
  else
    return ActionCatTable[action];
}


string RefAction::str() const
{
  return name;
}

