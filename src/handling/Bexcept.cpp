/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


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


string Bexcept::getMessage() const
{
  return message;
}


bool Bexcept::isTricks() const
{
  return (file == "Contract.cpp" &&
      function == "Contract::setTricks");
}


bool Bexcept::isNoCards() const
{
  return (file == "fileLIN.cpp" &&
      function == "readLINChunk" &&
      message == "No LIN cards found (md)");
}


bool Bexcept::isPlay() const
{
  return (file == "Play.cpp" &&
      function == "Play::addPlay");
}


bool Bexcept::isPlayDD() const
{
  return (file == "Play.cpp" &&
      function == "Play::setDeclAndDenom");
}


bool Bexcept::isClaim() const
{
  return (file == "Play.cpp" &&
      function == "Play::makeClaim");
}

