/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_ORDERCOUNTS_H
#define BRIDGE_ORDERCOUNTS_H

#include <vector>

#include "Order.h"

using namespace std;


class OrderCounts
{
  private:

    vector<unsigned> misfits;
    unsigned no;


  public:

    OrderCounts();

    void reset();

    void incr(
      const Counts& counts,
      const Counts& countsPrev);
    
    BoardOrder classify() const;

};

#endif

