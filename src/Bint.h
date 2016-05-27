/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BINT_H
#define BRIDGE_BINT_H

#include "Bint.h"

// Bint is an int that knows whether or not it has been set.


class Bint
{
  private:

    bool setFlag;
    int value;


  public:

    Bint();

    ~Bint();

    void Reset();

    bool IsSet() const;

    bool Set(const int v2);

    int Get() const;

    bool operator == (const Bint& b2);

    bool operator == (const int v2);

    bool operator != (const Bint& b2);

    bool operator != (const int v2);
};

#endif

