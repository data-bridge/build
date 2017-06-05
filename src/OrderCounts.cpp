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
  {
    fits[i] = 0;
    misfits[i] = 0;
  }
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
    if (counts.bno > countsPrev.bno)
    {
      // OO or CC increasing.
      fits[ORDER_OCOC]++;
      fits[ORDER_COCO]++;
      fits[ORDER_OOCC]++;
    }
    else
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
    {
      fits[ORDER_COCO]++;
      misfits[ORDER_OCOC]++;
    }
    else
    {
      fits[ORDER_COCO]++;
      fits[ORDER_OCOC]++;
    }
  }
  else
  {
    // OC.
    fits[ORDER_OOCC]++;
    if (counts.bno < countsPrev.bno)
    {
      misfits[ORDER_OCOC]++;
      misfits[ORDER_COCO]++;
    }
    else if (counts.bno == countsPrev.bno)
    {
      fits[ORDER_OCOC]++;
      misfits[ORDER_COCO]++;
    }
    else
    {
      fits[ORDER_COCO]++;
      fits[ORDER_OCOC]++;
    }
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
    return (misfits[ORDER_COCO] ? ORDER_GENERAL : ORDER_COCO);
  }
  else if (misfits[ORDER_COCO] && misfits[ORDER_OOCC])
  {
    return (misfits[ORDER_OCOC] ? ORDER_GENERAL : ORDER_OCOC);
  }
  else
    return ORDER_OCOC;
}

