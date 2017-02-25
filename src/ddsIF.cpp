/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <sstream>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif

#include "ddsIF.h"
#include "Bexcept.h"

static mutex mtx;

using namespace std;


unsigned tricksDD(
  RunningDD& running)
{
  futureTricks fut;
  mtx.lock();
  int res = SolveBoard(running.dl, -1, 1, 1, &fut, 0);
  mtx.unlock();

  if (res != RETURN_NO_FAULT)
  {
    char line[80];
    mtx.lock();
    ErrorMessage(res, line);
    mtx.unlock();
    stringstream ss;
    ss << "DDS error: " << line;
    THROW(ss.str());
  }

  return (running.declLeadFlag ? running.tricksDecl + fut.score[0] :
    13 - (running.tricksDef + fut.score[0]));
}

