/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <sstream>
#include <mutex>

#include "ddsIF.h"

#include "../handling/Bexcept.h"

static mutex mtx;

using namespace std;


void errorDD(const int res)
{
  char line[80];
  mtx.lock();
  ErrorMessage(res, line);
  mtx.unlock();
  stringstream ss;
  ss << "DDS error: " << line;
  THROW(ss.str());
}


unsigned tricksDD(
  RunningDD& running)
{
  futureTricks fut;
  mtx.lock();
  int res = SolveBoard(running.dl, -1, 1, 1, &fut, 0);
  mtx.unlock();

  if (res != RETURN_NO_FAULT)
    errorDD(res);

  return (running.declLeadFlag ? running.tricksDecl + fut.score[0] :
    13 - (running.tricksDef + fut.score[0]));
}


void tableauDD(
  ddTableDealsPBN * tablePBN,
  ddTablesRes * resDDS)
{
  int trumpFilter[5] = {0, 0, 0, 0, 0};

  mtx.lock();
  int res = CalcAllTablesPBN(tablePBN, -1, trumpFilter, resDDS, nullptr);
  mtx.unlock();

  if (res != RETURN_NO_FAULT)
    errorDD(res);
}


void traceDD(
  boardsPBN * bopPBN,
  playTracesPBN * plpPBN,
  solvedPlays * resDDS)
{
  mtx.lock();
  int res = AnalyseAllPlaysPBN(bopPBN, plpPBN, resDDS, 0);
  mtx.unlock();

  if (res != RETURN_NO_FAULT)
    errorDD(res);
}

