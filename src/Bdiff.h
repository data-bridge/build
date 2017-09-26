/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BDIFF_H
#define BRIDGE_BDIFF_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#pragma warning(pop)

using namespace std;


#define DIFF(msg) throw Bdiff(__FILE__, __LINE__, __FUNCTION__, msg)


class Bdiff: public exception
{
  private:

    string file;
    int line;
    string function;
    string message;


  public:

    Bdiff(
      const char fileArg[],
      const int lineArg,
      const char functionArg[],
      const string messageArg);

    void print(ostream& flog) const;
};

#endif

