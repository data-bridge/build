/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "OrderCounts.h"


OrderCounts::OrderCounts()
{
  OrderCounts::reset();
}


OrderCounts::~OrderCounts()
{
}


void OrderCounts::reset()
{
  no = 0;
  for (unsigned i = 0; i < ORDER_GENERAL; i++)
    misfits[i] = 0;
}


void OrderCounts::incr(
  const Counts& counts,
  const Counts& countsPrev)
{
  if (countsPrev.bno == 0)
    return;

  no++;

  if (counts.openFlag == countsPrev.openFlag)
  {
    if (counts.bno <= countsPrev.bno)
    {
      // OO or CC not increasing.
      misfits[ORDER_OCOC]++;
      misfits[ORDER_COCO]++;
      misfits[ORDER_OOCC]++;
    }
  }
  else if (counts.openFlag)
  {
    // CO.
    misfits[ORDER_OOCC]++;
    if (counts.bno < countsPrev.bno)
    {
      misfits[ORDER_OCOC]++;
      misfits[ORDER_COCO]++;
    }
    else if (counts.bno == countsPrev.bno)
      misfits[ORDER_OCOC]++;
  }
  else
  {
    // OC.
    if (counts.bno < countsPrev.bno)
    {
      misfits[ORDER_OCOC]++;
      misfits[ORDER_COCO]++;
    }
    else if (counts.bno == countsPrev.bno)
      misfits[ORDER_COCO]++;
  }
}


BoardOrder OrderCounts::classify() const
{
  if (no == 0)
    return ORDER_OCOC;

  if (misfits[ORDER_OCOC] && misfits[ORDER_COCO])
  {
    return (misfits[ORDER_OOCC] ? ORDER_GENERAL : ORDER_OOCC);
  }
  else if (misfits[ORDER_OCOC] && misfits[ORDER_OOCC])
  {
    return ORDER_COCO;
  }
  else
    return ORDER_OCOC;
}

