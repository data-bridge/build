/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>

#include "Bdiff.h"


Bdiff::Bdiff(
  const char fileArg[],
  const int lineArg,
  const char functionArg[],
  const string messageArg):
    file(string(fileArg)),
    line(lineArg),
    function(string(functionArg)),
    message(messageArg)
{
}


void Bdiff::Print() const
{
  cout << "[File " << file << "]\n";
  cout << "[Line " << line << "]\n";
  cout << "[Func " << function << "]\n";
  cout << message << endl;
}

