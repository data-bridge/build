/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#include <sstream>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif
#pragma warning(pop)

#include "ddsIF.h"
#include "Bexcept.h"

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

