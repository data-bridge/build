/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>

#include "Bexcept.h"


Bexcept::Bexcept(
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


void Bexcept::print(ostream& flog) const
{
  flog << "Exception thrown in " << file << 
      ", function " << function << 
      ", line number " << line << endl;
  flog << message << endl;
}


bool Bexcept::isTricks() const
{
  return (file == "Contract.cpp" &&
      function == "Contract::setTricks" &&
      message == "Tricks already set differently");
}
