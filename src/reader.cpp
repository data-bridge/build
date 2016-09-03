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
#include "Auction.h"
#include "Play.h"

Debug debug;

void TestTableau(Tableau& tableau);

void TestDeal(Deal& deal);

void TestAuction(Auction& auction);

void TestPlay(Play& play);


int main(int argc, char * argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  Tableau tableau;
  Contract contract;
  Deal deal;
  Auction auction;
  Play play;

  // TestTableau(tableau);
  // TestDeal(deal);
  // TestAuction(auction);

  TestPlay(play);

  // Players players;
  // cout << deal.AsTXT(players) << "\n";
}


void TestTableau(Tableau& tableau)
{
  tableau.SetEntry(BRIDGE_NORTH, BRIDGE_NOTRUMP, 7);
  tableau.SetRBN("::88665+88667:!!");
  cout << "TXT:\n" << tableau.ToString(BRIDGE_FORMAT_TXT) << "\n";

  string text;
  tableau.GetPar(BRIDGE_NORTH, BRIDGE_VUL_NONE, text);
  cout << "Par: '" << text << "'\n";
}


void TestDeal(Deal& deal)
{
  if (1)
  {
    deal.Set("3SKJ5HAJ954DJ83CA8,SA764H83DQT7642CK,SQT3HKT62DK9C9532,S982HQ7DA5CQJT764", BRIDGE_FORMAT_LIN);
  }

  if (0)
  {
    if (! deal.Set("E:A764.83.QT7642.K QT3.KT62.K9.9532 982.Q7.A5.QJT764 KJ5.AJ954.J83.A8", BRIDGE_FORMAT_PBN))
    {
      debug.Print();
      exit(0);
    }
  }

  if (0)
  {
    if (! deal.Set("E:A764.83.QT7642.K:QT3.KT62.K9.9532:982.Q7.A5.QJT764:", BRIDGE_FORMAT_RBN))
    {
      debug.Print();
      exit(0);
    }
  }

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
}


void TestAuction(Auction& auction)
{
  if (! auction.SetDealerVul(BRIDGE_SOUTH, BRIDGE_VUL_EAST_WEST))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddCall("P"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddCall("P"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddCall("1S", "5-card majors"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddCall("P"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddCall("2S!"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.UndoLastCall())
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddCall("2H", "Mixed raise"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddCall("P"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddCall("4S"))
  {
    debug.Print();
    exit(0);
  }

  auction.AddPasses();

  cout << "LIN format:\n";
  string s = auction.AsString(BRIDGE_FORMAT_LIN);
  if (s == "")
  {
    debug.Print();
    exit(0);
  }
  cout << s << "\n\n";

  cout << "PBN format:\n";
  s = auction.AsString(BRIDGE_FORMAT_PBN);
  if (s == "")
  {
    debug.Print();
    exit(0);
  }
  cout << s << "\n";

  cout << "RBN format:\n";
  s = auction.AsString(BRIDGE_FORMAT_RBN);
  if (s == "")
  {
    debug.Print();
    exit(0);
  }
  cout << s << "\n";

  cout << "TXT format:\n";
  stringstream t;
  t << left << 
    setw(15) << "Garozzo" <<
    setw(15) << "Hamman" <<
    setw(15) << "Belladonna" <<
    setw(15) << "Wolff" << "\n";

  s = auction.AsString(BRIDGE_FORMAT_TXT, t.str());
  if (s == "")
  {
    debug.Print();
    exit(0);
  }
  cout << s << "\n";

  auction.Reset();
  if (! auction.SetDealerVul(BRIDGE_SOUTH, BRIDGE_VUL_EAST_WEST))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddAuctionRBN("A SE:PP1S^1P:2H*P4SA"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddAlert(1, "6-card majors"))
  {
    debug.Print();
    exit(0);
  }
  
  cout << "RBN format:\n";
  s = auction.AsString(BRIDGE_FORMAT_RBN);

  if (s == "")
  {
    debug.Print();
    exit(0);
  }

  cout << s << "\n";

  Contract contract;
  contract.SetContract(
    BRIDGE_VUL_EAST_WEST,
    BRIDGE_NORTH,
    4,
    BRIDGE_SPADES,
    BRIDGE_MULT_UNDOUBLED);

  if (auction.ConsistentWith(contract))
    cout << "Consistent" << endl;
  else
    cout << "Inconsistent" << endl;
}


void TestPlay(Play& play)
{
  Contract contract;
  contract.SetContract(
    BRIDGE_VUL_EAST_WEST,
    BRIDGE_EAST,
    4,
    BRIDGE_HEARTS,
    BRIDGE_MULT_UNDOUBLED);

  play.SetContract(contract);

  Deal deal;
  deal.Set("N:Q75.32.AJ43.9543 AJ4.AT976.K72.K2 962.Q.QT95.AQJT8 KT83.KJ854.86.76", BRIDGE_FORMAT_PBN);
  
  unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
  if (! deal.GetDDS(cards))
  {
    assert(false);
  }

  if (! play.SetHoldingDDS(cards)) assert(false);

  const char *vlist[] =
    {"DT", "D6", "DA", "D7",
     "C5", "CK", "CA", "C6",
     "C2", "CQ", "C7", "C3",
     "HQ", "HK", "H2", "H6",
     "H4", "H3", "HA", "C8",
     "H9", "D5", "H5", "D3",
     "DK", "D9", "D8", "D4",
     "D2", "DQ", "H8", "DJ",
     "S3", "S7", "SA", "S2",
     "S4", "S9", "ST", "SQ",
     "S5"
    };
  const vector<string> list(vlist, end(vlist));

  for (const string &card: list)
  {
    if (play.AddPlay(card) != PLAY_NO_ERROR)
    {
      assert(false);
    }
    else
      cout << "Card: " << card << "\n";
      cout << play.AsString(BRIDGE_FORMAT_TXT);
  }

  if (play.Claim(9) != PLAY_CLAIM_NO_ERROR) assert(false);
  cout << play.AsString(BRIDGE_FORMAT_LIN) << "\n";
  cout << play.AsString(BRIDGE_FORMAT_PBN);
  cout << play.AsString(BRIDGE_FORMAT_RBN);
  cout << play.AsString(BRIDGE_FORMAT_TXT);

  cout << play.ClaimAsString(BRIDGE_FORMAT_LIN) << endl; 

  play.Reset();
  play.SetContract(contract);
  if (! play.SetHoldingDDS(cards))
  {
    assert(false);
  }

  if (! play.AddTrickPBN("DT  D6  DA  D7  ")) assert(false);
  if (! play.AddTrickPBN("CA  C6  C5  CK  ")) assert(false);
  if (! play.AddTrickPBN("CQ  C7  C3  C2  ")) assert(false);
  if (! play.AddTrickPBN("HQ  HK  H2  H6  ")) assert(false);
  if (! play.AddTrickPBN("C8  H4  H3  HA  ")) assert(false);
  if (! play.AddTrickPBN("D5  H5  D3  H9  ")) assert(false);
  if (! play.AddTrickPBN("D9  D8  D4  DK  ")) assert(false);
  if (! play.AddTrickPBN("DQ  H8  DJ  D2  ")) assert(false);
  if (! play.AddTrickPBN("S2  S3  S7  SA  ")) assert(false);
  if (! play.AddTrickPBN("S9  ST  SQ  S4  ")) assert(false);

  if (play.Claim(9) != PLAY_CLAIM_NO_ERROR) assert(false);
  cout << play.AsString(BRIDGE_FORMAT_LIN);
  cout << play.AsString(BRIDGE_FORMAT_PBN);
  cout << play.AsString(BRIDGE_FORMAT_RBN);
  cout << play.AsString(BRIDGE_FORMAT_TXT);
}

