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
#include "fileEML.h"
#include "fileTXT.h"
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
  setEMLtables();
  setTXTtables();

  /*
  if (! readRBN(group, "S10FA1.RBN"))
  {
    debug.Print();
    assert(false);
  }
  */

  /*
  if (! readPBN(group, "S10FA1.PBN"))
  {
    debug.Print();
    assert(false);
  }
  */

  /*
  if (! readEML(group, "S10FA1.EML"))
  {
    debug.Print();
    assert(false);
  }
  */

  if (! readTXT(group, "S10FA1.TXT"))
  {
    debug.Print();
    assert(false);
  }


  if (! writeRBN(group, "out.rbn"))
  {
    debug.Print();
    assert(false);
  }

  if (! writePBN(group, "out.pbn"))
  {
    debug.Print();
    assert(false);
  }

  if (! writeEML(group, "out.eml"))
  {
    debug.Print();
    assert(false);
  }

  if (! writeTXT(group, "out.txt"))
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

