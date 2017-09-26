/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#pragma warning(pop)

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


void Bdiff::print(ostream& flog) const
{
  flog << "Difference seen in " << file <<
      ", function " << function <<
      ", line number " << line << endl;
  flog << message << endl;
}

