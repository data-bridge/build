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

#include "portab.h"
#include "bconst.h"

#include "Debug.h"
#include "Tableau.h"
#include "Contract.h"
#include "Deal.h"

Debug debug;


int main(int argc, char * argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  Tableau tableau;
  Contract contract;
  Deal deal;

  // tableau.SetEntry(BRIDGE_NORTH, BRIDGE_NOTRUMP, 7);
  // tableau.SetRBN("::88665+88667:!!");
  // cout << "TXT:\n" << tableau.ToString(BRIDGE_FORMAT_TXT) << "\n";

  // string text;
  // tableau.GetPar(BRIDGE_NORTH, BRIDGE_VUL_NONE, text);
  // cout << "Par: '" << text << "'\n";

  deal.Set("3SKJ5HAJ954DJ83CA8,SA764H83DQT7642CK,SQT3HKT62DK9C9532,S982HQ7DA5CQJT764", BRIDGE_FORMAT_LIN);

  // if (! deal.Set("E:A764.83.QT7642.K QT3.KT62.K9.9532 982.Q7.A5.QJT764 KJ5.AJ954.J83.A8", BRIDGE_FORMAT_PBN))
  // {
    // debug.Print();
    // exit(0);
  // }
  // if (! deal.Set("E:A764.83.QT7642.K:QT3.KT62.K9.9532:982.Q7.A5.QJT764:", BRIDGE_FORMAT_RBN))
  // {
    // debug.Print();
    // exit(0);
  // }

  cout << deal.AsString() << "\n";
  cout << deal.AsString(BRIDGE_EAST) << "\n";
  cout << deal.AsString(BRIDGE_SOUTH) << "\n";
  cout << deal.AsString(BRIDGE_WEST) << "\n";
  cout << "\n";
  cout << deal.AsString(BRIDGE_NORTH, BRIDGE_FORMAT_PBN);
  cout << deal.AsString(BRIDGE_EAST, BRIDGE_FORMAT_PBN);
  cout << deal.AsString(BRIDGE_SOUTH, BRIDGE_FORMAT_PBN);
  cout << deal.AsString(BRIDGE_WEST, BRIDGE_FORMAT_PBN);
  cout << "\n";
  cout << deal.AsString(BRIDGE_NORTH, BRIDGE_FORMAT_RBN);
  cout << deal.AsString(BRIDGE_EAST, BRIDGE_FORMAT_RBN);
  cout << deal.AsString(BRIDGE_SOUTH, BRIDGE_FORMAT_RBN);
  cout << deal.AsString(BRIDGE_WEST, BRIDGE_FORMAT_RBN);
  cout << "\n";

  Players players;
  cout << deal.AsTXT(players) << "\n";
}

