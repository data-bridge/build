/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Files.h"
#include "Debug.h"
#include "portab.h"

#include <map>

extern Debug debug;


Files::Files()
{
  Files::Reset();
}


Files::~Files()
{
}


void Files::Reset()
{
  len = 0;
}


void Files::Set(const OptionsType& options)
{
  // TODO
  UNUSED(options);
}


bool Files::GetNextTask(FileTaskType& ftask) const
{
  // TODO
  UNUSED(ftask);
  return true;
}

