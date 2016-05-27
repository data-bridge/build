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

Debug debug;


int main(int argc, char * argv[])
{
  Tableau tableau;
  Contract contract;

  // tableau.SetEntry(BRIDGE_NORTH, BRIDGE_NOTRUMP, 7);

  tableau.SetRBN("::88665+88667:!!");

  cout << "TXT:\n" << tableau.ToString(BRIDGE_FORMAT_TXT) << "\n";

  string text;
  tableau.GetPar(BRIDGE_NORTH, BRIDGE_VUL_NONE, text);
  cout << "Par: '" << text << "'\n";
}

