/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SDPOINTS_H
#define BRIDGE_SDPOINTS_H

#include <string>

using namespace std;

struct SDPoints
{
  // The card played (starting from 1) at which the whole deal
  // became known to a given player, based solely on the play.
  // dummyFlag is set if this struct is for dummy, who does not
  // try to reconstruct the deal.
  bool dummyFlag;

  bool distKnown;
  unsigned distCard;

  bool dealKnown;
  unsigned dealCard;

  void reset()
  {
    dummyFlag = false;
    distKnown = false;
    dealKnown = false;
  };

  string str() const
  {
    if (dummyFlag)
      return "D";

    string s;
    if (distKnown)
      s = to_string(distCard);
    else
      s = 'x';

    s += '-';

    if (dealKnown)
      s += to_string(dealCard);
    else
      s += 'x';

    return s;
  };
};

#endif
