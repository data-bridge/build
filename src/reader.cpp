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

#include "Group.h"
#include "UnitTest.h"
#include "fileRBN.h"
#include "filePBN.h"
#include "portab.h"
#include "bconst.h"
#include "Debug.h"

Debug debug;

void Test();


int main(int argc, char * argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  Group group;
  setRBNtables();
  setPBNtables();

  if (! readRBN(group, "S10FA1.RBN"))
  {
    debug.Print();
    assert(false);
  }

  if (! writePBN(group, "out.pbn"))
  {
    debug.Print();
    assert(false);
  }
}


void Test()
{
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
  // TestPlayers();
}

