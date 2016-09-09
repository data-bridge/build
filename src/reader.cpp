/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

using namespace std;

#include <assert.h>

#include "UnitTest.h"
#include "portab.h"
#include "bconst.h"
#include "Debug.h"

Debug debug;


int main(int argc, char * argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  Tableau tableau;
  Contract contract;
  Deal deal;
  Auction auction;
  Play play;
  Date date;
  Location location;
  Scoring scoring;
  Teams teams;
  Session session;
  Segment segment;

  // TestTableau(tableau);
  // TestDeal(deal);
  // TestAuction(auction);
  // TestPlay(play);
  // TestDate(date);
  // TestLocation(location);
  // TestScoring(scoring);
  // TestTeams(teams);
  // TestSession(session);

  TestSegment(segment);

  // Players players;
  // cout << deal.AsTXT(players) << "\n";
}

