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
#include "args.h"
#include "fileRBN.h"
#include "fileRBX.h"
#include "filePBN.h"
#include "fileEML.h"
#include "fileTXT.h"
#include "fileLIN.h"
#include "fileREC.h"
#include "portab.h"
#include "bconst.h"
#include "Debug.h"

Debug debug;
OptionsType options;


void Test();


int main(int argc, char * argv[])
{
  ReadArgs(argc, argv);

  Group group;
  setRBNtables();
  setRBXtables();
  setPBNtables();
  setEMLtables();
  setTXTtables();
  setLINtables();
  setRECtables();

  /*
  if (! readRBX(group, "S10FA1.RBX"))
  {
    debug.Print();
    assert(false);
  }
  */

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

  /*
  if (! readTXT(group, "S10FA1.TXT"))
  {
    debug.Print();
    assert(false);
  }
  */

  // 29123 is a pairs tournament.
  // 34567 has a claim "error".
  // 34999 is a team tournament.
  /* */
  if (! readLIN(group, "S10FA1.LIN"))
  // if (! readLIN(group, "02-20-12-1.lin"))
  // if (! readLIN(group, "Tournament 5586 02-20-12.lin"))
  // if (! readLIN(group, "29123.lin"))
  // if (! readLIN(group, "34567.lin"))
  // if (! readLIN(group, "34999.lin"))
  {
    debug.Print();
    assert(false);
  }
  /* */

  /*
  if (! readREC(group, "S10FA1.REC"))
  {
    debug.Print();
    assert(false);
  }
  */


  if (! writeRBN(group, "out.rbn"))
  {
    debug.Print();
    assert(false);
  }

  if (! writeRBX(group, "out.rbx"))
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

  // if (! writeLIN(group, "out.lin"))
  if (! writeLIN_RP(group, "out.lin"))
  // if (! writeLIN_TRN(group, "out.lin"))
  // if (! writeLIN_VG(group, "out.lin"))
  {
    debug.Print();
    assert(false);
  }

  if (! writeREC(group, "out.rec"))
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

