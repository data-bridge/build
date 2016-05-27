/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Debug.h"


Debug::Debug()
{
}


Debug::~Debug()
{
}


void Debug::Set(
  const char fileArg[],
  const int lineArg,
  const char functionArg[],
  const string messageArg)
{
  file = string(fileArg);
  line = lineArg;
  function = string(functionArg);
  message = messageArg;
}


void Debug::Print() const
{
  cout << "[File " << file << "]\n";
  cout << "[Line " << line << "]\n";
  cout << "[Func " << function << "]\n";
  cout << message << endl;
}

