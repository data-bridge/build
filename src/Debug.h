/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DEBUG_H
#define BRIDGE_DEBUG_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

using namespace std;


#define BRIDGE_DEBUG

#ifdef BRIDGE_DEBUG
  #define LOG(msg) debug.Set(__FILE__, __LINE__, __FUNCTION__, msg)
  // #define STR(x) \
    // static_cast<ostringstream*>(&(ostringstream() << x))->str()
#else
  #define LOG(msg)
  // #define STR(x)
#endif


class Debug
{
  private:

    string file;
    int line;
    string function;
    string message;


  public:

    Debug();

    ~Debug();

    void Set(
      const char fileArg[],
      const int lineArg,
      const char functionArg[],
      const string messageArg);

    void Print() const;
};

#endif

