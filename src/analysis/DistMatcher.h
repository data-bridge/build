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
      const unsigned spadesL,
      const unsigned heartsL,
      const unsigned diamondsL,
      const unsigned clubsL,
      const unsigned long1L,
      const unsigned long2L,
      const unsigned long3L,
      const unsigned long4L) const;

    string str() const;
};

#endif
