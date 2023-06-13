/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#include <iostream>
#include <iomanip>
#include <sstream>

#include "RefCount.h"

using namespace std;


void RefCount::reset()
{
  lines = 0;
  units = 0;
  hands = 0;
  boards = 0;
}


void RefCount::setLines(size_t linesIn)
{
  lines = linesIn;
}


void RefCount::setUnitsFrom(const RefCount& rc2)
{
  units = rc2.units;
}


void RefCount::set(
  const size_t unitsIn,
  const size_t handsIn,
  const size_t boardsIn)
{
  lines = 0;
  units = unitsIn;
  hands = handsIn;
  boards = boardsIn;
}


void RefCount::fixSomething()
{
  // TODO Why is this needed in Reflines?
  lines = units;
  units = 0;
}


size_t RefCount::getUnits() const
{
  return units;
}


void RefCount::operator += (const RefCount& rc2)
{
  lines += rc2.lines;
  units += rc2.units;
  hands += rc2.hands;
  boards += rc2.boards;
}


bool RefCount::sameValues(const RefCount& rc2) const
{
  return (units == rc2.units &&
    hands == rc2.hands &&
    boards == rc2.boards);
}


string RefCount::str() const
{
  stringstream ss;
  ss << left << setw(14) << "Count tags" << units << "\n";
  ss << left << setw(14) << "Count hands" << hands << "\n";
  ss << left << setw(14) << "Count boards" << boards << "\n";
  return ss.str();
}


string RefCount::strShort() const
{
  return 
    to_string(units) + "," +
    to_string(hands) + "," +
    to_string(boards);
}


string RefCount::strLineHeader() const
{
  stringstream ss;
  ss <<
    setw(9) << right << "Lines" <<
    setw(7) << right << "Units" <<
    setw(7) << right << "Hands" <<
    setw(7) << right << "Boards";
  return ss.str();
}


string RefCount::strLine() const
{
  stringstream ss;
  ss <<
    setw(9) << right << lines <<
    setw(7) << right << units <<
    setw(7) << right << hands <<
    setw(7) << right << boards;
  return ss.str();
}

