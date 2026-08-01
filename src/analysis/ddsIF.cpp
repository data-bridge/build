/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <sstream>
#include <cstring>
#include <mutex>
#include <algorithm>
#include <cassert>

#include "ddsIF.h"

#include "../handling/Bexcept.h"

static mutex mtx;

using namespace std;


void errorDD(const int res)
{
  char line[80];
  mtx.lock();
  ErrorMessage(res, line);
  mtx.unlock();
  stringstream ss;
  ss << "DDS error: " << line;
  THROW(ss.str());
}


unsigned tricksDD(
  RunningDD& running)
{
  futureTricks fut;
  mtx.lock();
  int res = SolveBoard(running.dl, -1, 1, 1, &fut, 0);
  mtx.unlock();

  if (res != RETURN_NO_FAULT)
    errorDD(res);

  return (running.declLeadFlag ? running.tricksDecl + fut.score[0] :
    13 - (running.tricksDef + fut.score[0]));
}


void tableauDD(
  ddTableDealsPBN * tablePBN,
  ddTablesRes * resDDS)
{
  int trumpFilter[5] = {0, 0, 0, 0, 0};

  mtx.lock();
  int res = CalcAllTablesPBN(tablePBN, -1, trumpFilter, resDDS, nullptr);
  mtx.unlock();

  if (res != RETURN_NO_FAULT)
    errorDD(res);
}


void addBoardToInput(
  const char cards[],
  const unsigned base,
  boardsPBN& bop)
{
  for (int player = 0; player < DDS_HANDS; player++)
  {
    for (int denom = 0; denom < DDS_STRAINS; denom++)
    {
      const unsigned index = base + DDS_STRAINS * player + denom;
     
      auto& dp = bop.deals[index];
      dp.trump = denom;
      dp.first = player;
      for (unsigned i = 0; i < 3; i++)
      {
        dp.currentTrickSuit[i] = 0;
        dp.currentTrickRank[i] = 0;
      }
      std::strcpy(dp.remainCards, cards);

      bop.target[index] = 0;
      bop.solutions[index] = 3;
      bop.mode[index] = 0;
    }
  }

  bop.noOfBoards += DDS_HANDS * DDS_STRAINS;
}


void transfer13Leads(
  const futureTricks& solved,
  array<LeadTriple, 13>& boardOutput)
{
  unsigned count = 0;
  for (int c = 0; c < solved.cards; c++)
  {
    const int suit = solved.suit[c];
    int rank = solved.rank[c];
    const int score = solved.score[c];
    int equals = solved.equals[c];

    boardOutput[count].fill(suit, rank, score);
    count++;

    rank = 0;
    while (equals != 0)
    {
      if (equals & 1)
      {
        boardOutput[count].fill(suit, rank, score);
        count++;
      }
      equals >>= 1;
      rank++;
    }
  }

  assert(count == 13);

  std::sort(boardOutput.begin(),
    boardOutput.end(),
    [](const LeadTriple& a, const LeadTriple& b)
    {
      return (a.suit != b.suit) ? 
        (a.suit < b.suit) : (a.rank > b.rank);
    });
}


void transferSolutions(
  boardsPBN& bop,
  solvedBoards& sob, 
  const unsigned base,
  ddLeadsRes& leadsDDS)
{
  mtx.lock();
  int res = SolveAllBoards(&bop, &sob);
  mtx.unlock();

  if (res != RETURN_NO_FAULT)
    errorDD(res);

  assert(sob.noOfBoards == bop.noOfBoards);

  const unsigned step = DDS_HANDS * DDS_STRAINS;

  for (unsigned b = 0; b < sob.noOfBoards / step; b++)
  {
    for (unsigned i = 0; i < step; i++)
    {
      const unsigned index = b * step + i;

      const auto& boardInput = bop.deals[index];
      const int trump = boardInput.trump;
      const int first = boardInput.first;
    
      transfer13Leads(sob.solvedBoard[index], 
        leadsDDS.results[base + b]
        [static_cast<unsigned>(trump)]
        [static_cast<unsigned>(first)]);
    }
  }
}


void tableauLeadsDD(
  ddTableDealsPBN * tablePBN,
  ddLeadsRes * leadsDDS)
{
  // Footprint of a single deal.
  const unsigned step = DDS_HANDS * DDS_STRAINS;
  assert(step <= MAXNOOFBOARDS);

  boardsPBN bop;
  bop.noOfBoards = 0;

  solvedBoards sob;

  leadsDDS->results.resize(static_cast<unsigned>(tablePBN->noOfTables));

  unsigned baseInput = 0;
  unsigned baseOutput = 0;

  for (int b = 0; b < tablePBN->noOfTables; b++)
  {
    if (baseInput + step > MAXNOOFBOARDS)
    {
      transferSolutions(bop, sob, baseOutput, * leadsDDS);
      baseOutput += bop.noOfBoards / step;

      baseInput = 0;
      bop.noOfBoards = 0;
      sob.noOfBoards = 0;
    }

    addBoardToInput(tablePBN->deals[b].cards, baseInput, bop);
    baseInput += step;
  }

  if (baseInput > 0)
  {
    transferSolutions(bop, sob, baseOutput, * leadsDDS);
  }
}


void traceDD(
  boardsPBN * bopPBN,
  playTracesPBN * plpPBN,
  solvedPlays * resDDS)
{
  mtx.lock();
  int res = AnalyseAllPlaysPBN(bopPBN, plpPBN, resDDS, 0);
  mtx.unlock();

  if (res != RETURN_NO_FAULT)
    errorDD(res);
}

