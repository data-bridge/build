/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <sstream>

#include "ddsIF.h"
#include "Bexcept.h"

using namespace std;


unsigned tricksDD(
  RunningDD& running)
{
  futureTricks fut;
  int res = SolveBoard(running.dl, -1, 1, 1, &fut, 0);

  if (res != RETURN_NO_FAULT)
  {
    char line[80];
    ErrorMessage(res, line);
    stringstream ss;
    ss << "DDS error: " << line;
    THROW(ss.str());
  }

  return (running.declLeadFlag ? running.tricksDecl + fut.score[0] :
    13 - (running.tricksDef + fut.score[0]));
}

