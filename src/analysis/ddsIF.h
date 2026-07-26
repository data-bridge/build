/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_DDSIF_H
#define BRIDGE_DDSIF_H

#include <vector>
#include <array>

#include "LeadTriple.h"
#include "dll.h"

using namespace std;


struct RunningDD
{
  unsigned tricksDecl;
  unsigned tricksDef;
  bool declLeadFlag;
  deal dl;
};

struct ddLeadsRes
{
  // board, denomination, player, 0..12: triples.
  // Each lead triple is the card led and the number of tricks.
  vector<array<array<array<LeadTriple, 13>, DDS_HANDS>, DDS_STRAINS>> results;
};

unsigned tricksDD(
  RunningDD& running);

void tableauDD(
  ddTableDealsPBN * tablePBN,
  ddTablesRes * resDDS);

void tableauLeadsDD(
  ddTableDealsPBN * tablePBN,
  ddLeadsRes * resDDS);

void traceDD(
  boardsPBN * bopPBN,
  playTracesPBN * plpPBN,
  solvedPlays * resDDS);

#endif
