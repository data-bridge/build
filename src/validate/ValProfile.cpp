/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <assert.h>

#include "ValProfile.h"

#include "../files/LineData.h"


ValProfile::ValProfile():
  example(BRIDGE_VAL_SIZE),
  count(BRIDGE_VAL_SIZE)
{
  ValProfile::reset();
}


void ValProfile::reset()
{
  for (unsigned v = 0; v < BRIDGE_VAL_SIZE; v++)
  {
    count[v] = 0;

    example[v].out.line = "";
    example[v].out.lno = 0;

    example[v].ref.line = "";
    example[v].ref.lno = 0;
  }
}


void ValProfile::log(
  const ValError label,
  const LineData& dataOut,
  const LineData& dataRef)
{
  // Only keep the first example of each kind.
  assert(label < BRIDGE_VAL_SIZE);
  if (example[label].out.line == "" && example[label].ref.line == "")
  {
    example[label].out.line = dataOut.line;
    example[label].out.lno = dataOut.no;

    example[label].ref.line = dataRef.line;
    example[label].ref.lno = dataRef.no;
  }

  count[label]++;
}


bool ValProfile::labelIsSet(const unsigned label) const
{
  assert(label < BRIDGE_VAL_SIZE);
  return (count[label] > 0);
}


bool ValProfile::hasError(const bool minorFlag) const
{
  unsigned lower = 
    (minorFlag ? 0u : static_cast<unsigned>(BRIDGE_VAL_ERROR));

  for (unsigned v = lower; v < BRIDGE_VAL_SIZE; v++)
  {
    if (count[v] > 0)
      return true;
  }
  return false;
}


unsigned ValProfile::getCount(const unsigned label) const
{
  assert(label < BRIDGE_VAL_SIZE);
  return count[label];
}


void ValProfile::operator += (const ValProfile& prof2)
{
  for (unsigned v = 0; v < BRIDGE_VAL_SIZE; v++)
  {
    if (count[v] == 0 && prof2.count[v] > 0)
      example[v] = prof2.example[v];

    count[v] += prof2.count[v];
  }
}


void ValProfile::addRange(
  const ValProfile& prof,
  const unsigned lower,
  const unsigned upper,
  bool& flag)
{
  assert(upper <= BRIDGE_VAL_SIZE);
  flag = false;
  for (unsigned v = lower; v < upper; v++)
  {
    if (prof.count[v])
    {
      flag = true;
      count[v]++;
    }
  }
}


void ValProfile::print(
  ostream& fstr,
  const bool minorFlag) const
{
  unsigned lower = 
    (minorFlag ? 0u : static_cast<unsigned>(BRIDGE_VAL_ERROR));

  bool showFlag = false;
  for (unsigned v = lower; v < BRIDGE_VAL_SIZE; v++)
  {
    if (count[v] == 0)
      continue;

    fstr << setw(12) << left << ValErrors[v].nameLong <<
      setw(4) << right << count[v] << "\n";
    showFlag = true;
  }
  if (! showFlag)
    return;

  fstr << "\n";

  for (unsigned v = lower; v < BRIDGE_VAL_SIZE; v++)
  {
    if (count[v] == 0)
      continue;
    
    fstr << ValErrors[v].nameLong << ":\n";
    fstr << "Out (" << setw(4) << example[v].out.lno << "): '" <<
      example[v].out.line << "'\n";
    fstr << "Ref (" << setw(4) << example[v].ref.lno << "): '" <<
      example[v].ref.line << "'\n";
    fstr << "\n";
  }
  fstr << "\n";
}

