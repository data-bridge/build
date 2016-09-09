/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_UNITTEST_H
#define BRIDGE_UNITTEST_H


#include "Tableau.h"
#include "Contract.h"
#include "Deal.h"
#include "Auction.h"
#include "Play.h"
#include "Date.h"
#include "Location.h"
#include "Scoring.h"
#include "Teams.h"
#include "Session.h"
#include "Segment.h"


void TestTableau(Tableau& tableau);

void TestDeal(Deal& deal);

void TestAuction(Auction& auction);

void TestPlay(Play& play);

void TestDate(Date& date);

void TestLocation(Location& location);

void TestScoring(Scoring& scoring);

void TestTeams(Teams& teams);

void TestSession(Session& session);

void TestSegment(Segment& segment);

#endif

