/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DISTMATCHER_H
#define BRIDGE_DISTMATCHER_H

#include "DistElem.h"

using namespace std;


class DistMatcher
{
  private:

    string spec;
    bool setFlag;

    DistElem spades, hearts, diamonds, clubs;
    DistElem long1, long2, long3, long4;


  public:

    DistMatcher();

    void reset();

    void set(const string& specIn);

    bool match(
      const int spadesL,
      const int heartsL,
      const int diamondsL,
      const int clubsL,
      const int long1L,
      const int long2L,
      const int long3L,
      const int long4L) const;

    string str() const;
};

#endif
