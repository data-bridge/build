/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <array>
#include <algorithm>

#include "LeadTableau.h"

#include "../util/parse.h"
#include "../handling/Bexcept.h"
#include "../handling/Bdiff.h"



LeadTableau::LeadTableau()
{
  LeadTableau::reset();
}


void LeadTableau::reset()
{
  setNum = 0;
  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
      for (unsigned t = 0; t < BRIDGE_TRICKS; t++)
        table[d][p][t].score = BRIDGE_TRICKS+1;
}


bool LeadTableau::isComplete() const
{
  return (setNum == BRIDGE_DENOMS * BRIDGE_PLAYERS * BRIDGE_TRICKS);
}


bool LeadTableau::set(
  [[maybe_unused]] const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_RBN:
      // return LeadTableau::setRBN(text);
      THROW("LeadTableau format not implemented:" + to_string(format));

    default:
      THROW("LeadTableau format not implemented:" + to_string(format));
  }
}


void LeadTableau::constantTricks()
{
  if (! LeadTableau::isComplete())
    return;

  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
  {
    array<unsigned, BRIDGE_PLAYERS> constantFlags;
    array<int, BRIDGE_PLAYERS> constantValues;
    unsigned overallConstant = 1;

    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned flag = 1;
      auto& ttable = table[d][p];
      int value = ttable[0].score;
      for (unsigned t = 1; t < BRIDGE_TRICKS; t++)
      {
        if (ttable[t].score != value)
        {
          flag = 0;
          break;
        }
      }

      constantFlags[p] = flag;
      constantValues[p] = value;

      if (flag == 0)
        overallConstant = 0;
    }

cout << "Denom " << d << ": " <<
  constantValues[0] << 
  constantValues[1] << 
  constantValues[2] << 
  constantValues[3] << ", ";

    if (overallConstant)
    {
      cout << "FULLCONST\n";
    }
    else if (constantValues[BRIDGE_NORTH] != constantValues[BRIDGE_SOUTH] ||
        constantValues[BRIDGE_EAST] != constantValues[BRIDGE_WEST])
    {
      cout << "NOTCONST\n";
    }
    else if (constantValues[BRIDGE_NORTH] + constantValues[BRIDGE_EAST] !=
        BRIDGE_TRICKS)
    {
      cout << "HALFCONST\n";
    }
    else
    {
      cout << "SYMMCONST\n";
    }
  }
}


void LeadTableau::setDDS(
  const array<array<array<LeadTriple, BRIDGE_TRICKS>, 
    BRIDGE_PLAYERS>, BRIDGE_DENOMS> res)
{
  // No checks.
  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
      for (unsigned t = 0; t < BRIDGE_TRICKS; t++)
        table[d][p][t] = res[d][p][t];
  
  setNum = BRIDGE_DENOMS * BRIDGE_PLAYERS * BRIDGE_TRICKS;

  LeadTableau::constantTricks();
}


bool LeadTableau::operator == (const LeadTableau& tableau2) const
{
  if (setNum != tableau2.setNum) 
    DIFF("Different numbers");

  for (int p = 0; p < BRIDGE_PLAYERS; p++)
  {
    for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    {
      for (unsigned l = 0; l < BRIDGE_TRICKS; l++)
      {
        const auto& lt1 = table[d][p][l];
        const auto& lt2 = tableau2.table[d][p][l];

      if (lt1.score == BRIDGE_TRICKS+1)
      {
        if (lt2.score != BRIDGE_TRICKS+1)
	  DIFF("p " + to_string(p) + ", d " + to_string(d) + 
              "t " + to_string(l) +
              ": First unset, second set");
      }
      else if (lt2.score == BRIDGE_TRICKS+1)
        DIFF("p " + to_string(p) + ", d " + to_string(d) + 
          "t " + to_string(l) +
          ": First set, second unset");
      else if (! (lt1 == lt2))
        DIFF("p " + to_string(p) + ", d " + to_string(d) + 
          "t " + to_string(l) +
          ": Different values");
      }
    }
  }

  return true;
}


bool LeadTableau::operator != (const LeadTableau& tableau2) const
{
  return ! (*this == tableau2);
}


string LeadTableau::str(const Format format) const
{
  if (! LeadTableau::isComplete())
    return "";

  stringstream ss("");

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return "XXXXXXXXXXXX";
      // THROW("Unknown format: " + to_string(format));

    case BRIDGE_FORMAT_PBN:
      return "XXXXXXXXXXXX";
      // THROW("Unknown format: " + to_string(format));

    case BRIDGE_FORMAT_RBN:
      return "XXXXXXXXXXXX";
      // THROW("Unknown format: " + to_string(format));

    case BRIDGE_FORMAT_TXT:
      return "XXXXXXXXXXXX";
      // THROW("Unknown format: " + to_string(format));
    
    default:
      return "XXXXXXXXXXXX";
      // THROW("Unknown format: " + to_string(format));
  }
}

