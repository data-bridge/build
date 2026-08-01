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
#include "Deal.h"

#include "../util/parse.h"
#include "../handling/Bexcept.h"
#include "../handling/Bdiff.h"

namespace 
{
  constexpr char HEX_DIGITS[] = "0123456789ABCDEF";
  constexpr char CARD_NAMES[] = "..23456789TJQKA";
  constexpr char RANK_NAMES[] = "AKQJT98765432";
  constexpr char SUIT_NAMES[] = "SHDC";
  constexpr string_view DENOM_NAMES = "SHDCN";
  constexpr string_view POSITION_NAMES = "NESW";
}


LeadTableau::LeadTableau()
{
  LeadTableau::reset();
}


void LeadTableau::reset()
{
  setNum = 0;
  cardsKnownFlag = false;
  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
      for (unsigned t = 0; t < BRIDGE_TRICKS; t++)
        table[d][p][t].score = BRIDGE_TRICKS+1;
}


bool LeadTableau::isComplete() const
{
  return (setNum == BRIDGE_DENOMS * BRIDGE_PLAYERS * BRIDGE_TRICKS);
}


int LeadTableau::lookupSuit(const char suitChar) const
{
  const char * ps = strchr(SUIT_NAMES, suitChar);
  if (ps == nullptr)
    THROW("Bad suit '" + string(1, suitChar) + "'");

  return static_cast<int>(ps - SUIT_NAMES);
}


int LeadTableau::lookupRank(const char rankChar) const
{
  const char * pr = strchr(RANK_NAMES, rankChar);
  if (pr == nullptr)
    THROW("Bad rank '" + string(1, rankChar) + "'");

  return static_cast<int>(BRIDGE_TRICKS + 1 + RANK_NAMES - pr);
}


void LeadTableau::setRBNline(
  const size_t strain,
  const size_t leader,
  const string& text)
{
  size_t pos = 0;
  unsigned nextCard = 0;
  auto& tb = table[strain][leader];

  while (pos < text.size())
  {
    while (pos < text.size() && text[pos] == ' ')
      pos++;

    if (pos >= text.size())
      break;

    size_t len;
    const int tricks = stoi(text.substr(pos), &len);
    pos += len;

    while (pos < text.size() && text[pos] == ' ')
      pos++;

    if (pos >= text.size() || text[pos] != '(')
      THROW("Expected '(' in '" + text + "'");

    const size_t close = text.find(')', pos);
    if (close == string::npos)
      THROW("Missing ')' in '" + text + "'");

    const string desc = text.substr(pos + 1, close - pos - 1);
    pos = close + 1;

    vector<string> tokens;
    tokenize(desc, tokens, " ");

    for (const string& tok: tokens)
    {
      const size_t colon = tok.find(':');

      if (colon == string::npos)
      {
        for (char suitChar: tok)
        {
          const int suit = LeadTableau::lookupSuit(suitChar);
          for (char rankChar: cardStrings[leader][suit])
          {
            const int rank = lookupRank(rankChar);
            tb[nextCard].suit = suit;
            tb[nextCard].rank = rank;
            tb[nextCard].score = tricks;
            nextCard++;
          }
        }
      }
      else
      {
        if (colon != 1)
          THROW("Bad token '" + tok + "'");

        const int suit = LeadTableau::lookupSuit(tok[0]);

        for (size_t i = colon + 1; i < tok.size(); i++)
        {
          const int rank = LeadTableau::lookupRank(tok[i]);
          tb[nextCard].suit = suit;
          tb[nextCard].rank = rank;
          tb[nextCard].score = tricks;
          nextCard++;
        }
      }
    }
  }

  if (nextCard != BRIDGE_TRICKS)
    THROW("Expected 13 cards: " + text);

  std::sort(
    tb,
    tb + BRIDGE_TRICKS,
    [](
      const LeadTriple& a,
      const LeadTriple& b)
    {
      if (a.suit != b.suit)
        return a.suit < b.suit;  // ascending suit
  
      return a.rank < b.rank;    // descending bridge rank
    });
}


void LeadTableau::setRBN(const string& text)
{
  if (text == "")
    THROW("Empty text");

  if (LeadTableau::isComplete())
    THROW("LeadTableau is already complete");

  if (! cardsKnownFlag)
    THROW("Cards must be known");

  istringstream iss(text);
  string line;

  while (getline(iss, line))
  {
    if (line.empty())
      continue;

    const size_t colon = line.find(':');
    if (colon != 2)
      THROW("Bad line: " + line);

    const size_t strain = DENOM_NAMES.find(line[0]);
    const size_t leader = POSITION_NAMES.find(line[1]);

    if (strain == string::npos || leader == string::npos)
      THROW("Bad prefix in line: " + line);

    LeadTableau::setRBNline(strain, leader, line.substr(3));
  }

  setNum = BRIDGE_DENOMS * BRIDGE_PLAYERS * BRIDGE_TRICKS;
}


void LeadTableau::set(
  const Deal& deal,
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_RBN:
      LeadTableau::setCards(deal);
      return LeadTableau::setRBN(text);

    default:
      THROW("LeadTableau format not implemented: " + to_string(format));
  }
}


void LeadTableau::setCards(const Deal& deal)
{
  if (LeadTableau::isComplete())
    THROW("LeadTableau already complete before cards");

  if (cardsKnownFlag)
    THROW("Cards already known");

  for (unsigned player = 0; player < BRIDGE_PLAYERS; player++)
  {
    const string s = deal.strHand(
      static_cast<Player>(player), BRIDGE_FORMAT_TXT);
    vector<string> cards;
    tokenize(s, cards, ",");

    if (cards.size() != 4)
      THROW("Expected 4 suits in " + s);

    for (unsigned sno = 0; sno < BRIDGE_SUITS; sno++)
    {
      string c = cards[sno];
      while (! c.empty() && c.front() == ' ')
        c.erase(0, 1);

      while (! c.empty() && c.back() == ' ')
        c.pop_back();

      cardStrings[player][sno] = c;
    }
  }

  cardsKnownFlag = true;
}


void LeadTableau::makeSuitGroups(
  array<array<SuitGroupElem, BRIDGE_PLAYERS>, BRIDGE_DENOMS>&
    suitGroups,
  array<array<SuitIndicesElem, BRIDGE_PLAYERS>, BRIDGE_DENOMS>&
  suitIndices) const
{
  if (! LeadTableau::isComplete())
    return;

  // suitGroups[strain][leader][lead suit] is a list of groups.
  // A group leads to the same number of tricks.
  // suitIndices[strain][leader][lead suit][tricks] is a list of
  // indices into table[strain][leader] that yield the same number
  // of tricks.
  for (unsigned strain = 0; strain < BRIDGE_DENOMS; strain++)
  {
    for (unsigned leader = 0; leader < BRIDGE_PLAYERS; leader++)
    {
      // For [suit], a list of trick values (not yet sorted).
      array<list<unsigned>, BRIDGE_SUITS> trickValues{};

      auto& si = suitIndices[strain][leader];

      for (unsigned card = 0; card < BRIDGE_TRICKS; card++)
      {
        auto& triple = table[strain][leader][card];
        unsigned suit = static_cast<unsigned>(triple.suit);
        unsigned tricks = static_cast<unsigned>(triple.score);
        if (si[suit][tricks].size() == 0)
        {
          trickValues[suit].push_back(tricks);
        }

        si[suit][tricks].push_back(card);
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
          for (auto card: si[suit][tricks])
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


string LeadTableau::strRBN() const
{
  // This is not real RBN.
  string s = "";
  for (unsigned strain = 0; strain < BRIDGE_DENOMS; strain++)
  {
    for (unsigned leader = 0; leader < BRIDGE_PLAYERS; leader++)
    {
      s += DENOM_NAMES[strain];
      s += POSITION_NAMES[leader];
      s += ":";

      auto& tb = table[strain][leader];

      for (unsigned tricks = 0; tricks < BRIDGE_TRICKS; tricks++)
        s += HEX_DIGITS[tb[tricks].score];
      
      s += "\n";
    }
  }

  return s;

}


string LeadTableau::strElementTXT(
  const SuitGroupElem& leadGroups,
  const SuitIndicesElem& trickIndices) const
{
  array<unsigned, BRIDGE_TRICKS+1> tricksOverall{};

  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
  {
    for (unsigned tricks = 0; tricks <= BRIDGE_TRICKS; tricks++)
    {
      if (trickIndices[suit][tricks].size() > 0)
        tricksOverall[tricks] = 1;
    }
  }

  string s = "";
  for (unsigned tricks = 0; tricks <= BRIDGE_TRICKS; tricks++)
  {
    if (tricksOverall[tricks] == 0)
      continue;

    s += " " + to_string(tricks) + " (";
    unsigned flagConstantSuits = 0;

    // Suits with exactly one number of tricks.
    for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
    {
      if (leadGroups[suit].size() == 1 &&
          leadGroups[suit].front().tricks == tricks)
      {
        s += SUIT_NAMES[suit];
        flagConstantSuits = 1;
      }
    }

    // Suits with multiple outcomes.
    unsigned flagMultipleOutput = 0;
    for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
    {
      // Suit symbol output already?
      unsigned flagSuitSymbol = 0;
      if (leadGroups[suit].size() <= 1)
        continue;

      for (const auto& lg: leadGroups[suit])
      {
        if (lg.tricks != tricks)
          continue;
      
        if (flagConstantSuits)
        {
          // Need a space the first time if there are 
          // constant suits already.
          s += " ";
          flagConstantSuits = 0;
        }
        else if (flagMultipleOutput)
          s += " ";

        flagMultipleOutput = 1;

        if (! flagSuitSymbol)
        {
          s += SUIT_NAMES[suit];
          s += ":";
          flagSuitSymbol = 1;
        }

        s += lg.text;
      }
    }

    s += ")";
  }

  return s;
}


string LeadTableau::strTXT() const
{
  // This one is more semantic than strRBN, but there is no real
  // RBN for this.

  array<array<SuitGroupElem, BRIDGE_PLAYERS>, BRIDGE_DENOMS> 
    suitGroups;
  array<array<SuitIndicesElem, BRIDGE_PLAYERS>, BRIDGE_DENOMS> 
    suitIndices;

  LeadTableau::makeSuitGroups(suitGroups, suitIndices);

  // In order to have separation from the board number.
  string s = "\n";
  for (unsigned strain = 0; strain < BRIDGE_DENOMS; strain++)
  {
    for (unsigned leader = 0; leader < BRIDGE_PLAYERS; leader++)
    {
      s += DENOM_NAMES[strain];
      s += POSITION_NAMES[leader];
      s += ":";

      s += LeadTableau::strElementTXT(
          suitGroups[strain][leader],
          suitIndices[strain][leader]) + "\n";
    }
  }

  return s;
}


string LeadTableau::str(const Format format) const
{
  if (! LeadTableau::isComplete())
    return "";

  stringstream ss("");

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      THROW("Unknown format: " + to_string(format));

    case BRIDGE_FORMAT_PBN:
      THROW("Unknown format: " + to_string(format));

    case BRIDGE_FORMAT_RBN:
      return LeadTableau::strRBN();

    case BRIDGE_FORMAT_TXT:
      return LeadTableau::strTXT();
    
    default:
      THROW("Unknown format: " + to_string(format));
  }
}

