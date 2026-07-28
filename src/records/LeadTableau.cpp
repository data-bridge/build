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
#include <cassert>

#include "LeadTableau.h"

#include "../util/parse.h"
#include "../handling/Bexcept.h"
#include "../handling/Bdiff.h"

namespace 
{
  // constexpr char HEX_DIGITS[] = "0123456789ABCDEF";
  constexpr char CARD_NAMES[] = "..23456789TJQKA";
  constexpr char SUIT_NAMES[] = "SHDC";
}


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


void LeadTableau::makeSuitGroups()
{
  if (! LeadTableau::isComplete())
    return;

  for (unsigned strain = 0; strain < BRIDGE_DENOMS; strain++)
  {
    for (unsigned leader = 0; leader < BRIDGE_PLAYERS; leader++)
    {
      // For [suit][tricks], a list of indices into table[strain][leader].
      array<array<list<unsigned>, BRIDGE_TRICKS+1>, BRIDGE_SUITS> 
        suitIndices;

      // For [suit], a list of trick values (not yet sorted).
      array<list<unsigned>, BRIDGE_SUITS> trickValues{};

      for (unsigned card = 0; card < BRIDGE_TRICKS; card++)
      {
        auto& triple = table[strain][leader][card];
        unsigned suit = static_cast<unsigned>(triple.suit);
        unsigned tricks = static_cast<unsigned>(triple.score);
        if (suitIndices[suit][tricks].size() == 0)
        {
          trickValues[suit].push_back(tricks);
        }

        suitIndices[suit][tricks].push_back(card);
      }

      auto& sg = suitGroups[strain][leader];

      for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
      {
        trickValues[suit].sort();
        for (auto tricks: trickValues[suit])
        {
          sg[suit].emplace_back(LeadGroup());
          auto& lg = sg[suit].back();
          lg.clear();

          lg.tricks = tricks;
          for (auto card: suitIndices[suit][tricks])
          {
            const auto& triple = table[strain][leader][card];

            if (lg.numCards == 0 || 
                static_cast<unsigned>(triple.rank + 1) != lg.lastSeenRank)
            {
              lg.numRanks++;
              lg.lastSeenRank = static_cast<unsigned>(triple.rank);
            }

            lg.numCards++;
            lg.text += CARD_NAMES[triple.rank];
          }
        }
      }
    }
  }
}


void LeadTableau::makeSuitProfiles()
{
  for (unsigned strain = 0; strain < BRIDGE_DENOMS; strain++)
  {
    for (unsigned leader = 0; leader < BRIDGE_PLAYERS; leader++)
    {
      const auto& sg = suitGroups[strain][leader];
      auto& sp = suitProfiles[strain][leader];

      for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
      {
        const auto count = sg[suit].size();
        if (count >= 2)
        {
          sp[suit].voidFlag = 0;
          sp[suit].constantFlag = 0;
        }
        else if (count == 0)
        {
          sp[suit].voidFlag = 1;
          sp[suit].constantFlag = 1;
        }
        else
        {
          sp[suit].voidFlag = 0;
          sp[suit].constantFlag = 1;
          sp[suit].value = sg[suit].front().tricks;
        }
      }
    }
  }
}


bool LeadTableau::singleValue(
  const unsigned strain,
  const unsigned leader,
  unsigned& value) const
{
  unsigned flag = 0, v;

  const auto& sp = suitProfiles[strain][leader];
  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
  {
    if (! sp[suit].constantFlag)
      return false;
    else if (sp[suit].voidFlag)
      continue;
    else if (flag == 0)
    {
      flag = 1;
      v = sp[suit].value;
    }
    else if (v != sp[suit].value)
    {
      return false;
    }
  }

  value = v;
  return true;
}


bool LeadTableau::flatValues(
  const unsigned strain,
  const unsigned leader) const
{
  // For each suit led, there is only one number of tricks possible.
  // This could differ among suits.
  const auto& sp = suitProfiles[strain][leader];
  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
  {
    if (! sp[suit].constantFlag)
      return false;
  }
  return true;
}


string LeadTableau::strFlatValues(
  const unsigned strain,
  const unsigned leader) const
{
  // For each suit led, there is only one number of tricks possible.
  // This could differ among suits.
  const auto& sp = suitProfiles[strain][leader];
  string s;

  array<unsigned, BRIDGE_SUITS> seen{};
  unsigned count = 0;

  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
  {
    if (sp[suit].voidFlag || seen[suit])
    {
      count++;
      continue;
    }

    const unsigned v = sp[suit].value;
    for (unsigned suit2 = suit; suit2 < BRIDGE_SUITS; suit2++)
    {
      if (sp[suit2].value == v)
      {
        s += SUIT_NAMES[suit2];
        seen[suit2] = 1;
        count++;
      }
    }

    s += " " + to_string(v);
    if (count < BRIDGE_SUITS)
      s += " ";
  }
  return s;
}


void LeadTableau::setDDS(
  const array<array<array<LeadTriple, BRIDGE_TRICKS>, 
    BRIDGE_PLAYERS>, BRIDGE_DENOMS> res)
{
  // No checks.
  for (unsigned strain = 0; strain < BRIDGE_DENOMS; strain++)
    for (unsigned leader = 0; leader < BRIDGE_PLAYERS; leader++)
      for (unsigned t = 0; t < BRIDGE_TRICKS; t++)
        table[strain][leader][t] = res[strain][leader][t];
  
  setNum = BRIDGE_DENOMS * BRIDGE_PLAYERS * BRIDGE_TRICKS;

  LeadTableau::makeSuitGroups();
  LeadTableau::makeSuitProfiles();

  unsigned value;

  for (unsigned strain = 0; strain < BRIDGE_DENOMS; strain++)
  {
    for (unsigned leader = 0; leader < BRIDGE_PLAYERS; leader++)
    {
      cout << "Strain " << strain << " leader " << leader << "\n";

      if (LeadTableau::singleValue(strain, leader, value))
      {
        cout << "ALLSUITS " << value << "\n\n";
        continue;
      }

      if (LeadTableau::flatValues(strain, leader))
      {
        cout << "FLATSUITS " << 
          LeadTableau::strFlatValues(strain, leader) << "\n\n";
        continue;
      }

      const auto& sg = suitGroups[strain][leader];
      for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
      {
        if (sg[suit].size() == 0)
        {
          cout << "Suit " << suit << ": void\n";
        }
        else if (sg[suit].size() == 1)
        {
          cout << "Suit " << suit << ": CONST " << 
            sg[suit].front().tricks << "\n";
        }
        else if (sg[suit].size() == 2)
        {
          const auto& lg1 = sg[suit].front();
          const auto& lg2 = sg[suit].back();

          if (lg1.numRanks > 1 && lg2.numRanks == 1)
          {
            cout << "Suit " << suit << 
              ": MOSTLY " << lg1.tricks << 
              " except " << lg2.tricks << 
              " (" << lg2.text << ")\n";
          }
          else if (lg1.numRanks == 1 && lg2.numRanks > 1)
          {
            cout << "Suit " << suit << 
              ": MOSTLY " << lg2.tricks << 
              " except " << lg1.tricks << 
              " (" << lg1.text << ")\n";
          }
          else
          {
            cout << "Suit " << suit << 
              ": BIMODAL " << lg1.tricks << " (" << lg1.text << ") " <<
              lg2.tricks << " (" << lg2.text << ")\n";
          }
        }
        else
        {
          cout << "Suit " << suit << " GENERAL";
          for (const auto& lg: sg[suit])
          {
            cout << " " << lg.tricks << " (" << lg.text << ")";
          }
          cout << "\n";
        }
      }
      cout << "\n";
    }
  }
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

