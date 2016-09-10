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

  // TestTableau();
  // TestDeal();
  // TestAuction();
  // TestPlay();
  // TestDate();
  // TestLocation();
  // TestScoring();
  // TestTeams();
  // TestSession();
  // TestSegment();
  TestPlayers();
}

