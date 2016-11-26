/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_EXCEPT_H
#define BRIDGE_EXCEPT_H

#include <string>

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

    bool isTricks() const;

    bool isPlay() const;
};

#endif

