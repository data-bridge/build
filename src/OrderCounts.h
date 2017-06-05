/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_ORDERCOUNTS_H
#define BRIDGE_ORDERCOUNTS_H

#include "bconst.h"

using namespace std;


class OrderCounts
{
  private:

    unsigned misfits[ORDER_GENERAL];
    unsigned no;


  public:

    OrderCounts();

    ~OrderCounts();

    void reset();

    void incr(
      const Counts& counts,
      const Counts& countsPrev);
    
    BoardOrder classify() const;

};

#endif

