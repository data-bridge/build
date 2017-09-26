/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_EXCEPT_H
#define BRIDGE_EXCEPT_H

#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#pragma warning(push, 0)
#include <string>
#pragma warning(pop)

using namespace std;


#define THROW(msg) throw Bexcept(__FILE__, __LINE__, __FUNCTION__, msg)


class Bexcept: public exception
{
  private:

    string file;
    int line;
    string function;
    string message;


  public:

    Bexcept(
      const char fileArg[],
      const int lineArg,
      const char functionArg[],
      const string messageArg);

    void print(ostream& flog) const;

    string getMessage() const;

    bool isNoCards() const;

    bool isTricks() const;

    bool isPlay() const;

    bool isPlayDD() const;

    bool isClaim() const;
};

#endif

