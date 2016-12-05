/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <windows.h>
#include <stdio.h>


#include "StackWalker.h"
#include "trace.h"

class StackWalker1: public StackWalker
{
  public:
    StackWalker1(): StackWalker() {}
    StackWalker1(int opt): StackWalker(opt) {}

  protected:
    virtual void OnOutput(LPCSTR text) { printf("%s", text); }
};


void trace()
{
  StackWalker1 sw(0x0);
  sw.ShowCallstack();
}
