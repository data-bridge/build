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
#include "UnitTest.h"

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
#include "Players.h"


#include "Debug.h"

extern Debug debug;


void TestAuctionCall(
  Auction& auction,
  const string& call);

void TestAuctionCallAlert(
  Auction& auction,
  const string& call,
  const string& alert);

void TestAuctionOutput(Auction& auction);

void TestLocationCase(
  Location& location,
  const string& s,
  const formatType f); 

void TestScoringCase(
  Scoring& scoring,
  const string& s,
  const formatType f);

void TestSessionCase(
  Session& session,
  const string& s,
  const formatType f);

void TestTeamsOut(const Teams& teams);

void TestTeamsCase(
  Teams& teams,
  const string list[2],
  const formatType f);

void TestSegmentCaseOutput(Segment& segment);

void TestSegmentCaseLIN(
  Segment& segment,
  const string& title);

void TestSegmentCaseCommon(
  Segment& segment,
  const string& title,
  const string& date,
  const string& location,
  const string& event,
  const string& session,
  const string& scoring,
  const formatType f);

void TestSegmentCasePBN(
  Segment& segment,
  const string& title,
  const string& date,
  const string& location,
  const string& event,
  const string& session,
  const string& scoring,
  const string& team1,
  const string& team2,
  const formatType f);

void TestSegmentCaseRBN(
  Segment& segment,
  const string& title,
  const string& date,
  const string& location,
  const string& event,
  const string& session,
  const string& scoring,
  const string& teams,
  const formatType f);


void TestTableau()
{
  Tableau tableau;

  tableau.SetEntry(BRIDGE_NORTH, BRIDGE_NOTRUMP, 7);
  tableau.Set("::88665+88667:!!", BRIDGE_FORMAT_RBN);
  cout << "TXT:\n" << tableau.AsString(BRIDGE_FORMAT_TXT) << "\n";

  string text;
  tableau.GetPar(BRIDGE_NORTH, BRIDGE_VUL_NONE, text);
  cout << "Par: '" << text << "'\n";
}


void TestDeal()
{
  Deal deal;

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


void TestAuctionCall(
  Auction& auction,
  const string& call)
{
  if (! auction.AddCall(call))
  {
    debug.Print();
    exit(0);
  }
}


void TestAuctionCallAlert(
  Auction& auction,
  const string& call,
  const string& alert)
{
  if (! auction.AddCall(call, alert))
  {
    debug.Print();
    exit(0);
  }
}

void TestAuctionOutput(
  Auction& auction,
  const string& names)
{
  for (unsigned f2 = 0; f2 <= BRIDGE_FORMAT_TXT; f2++)
  {
    formatType ff = static_cast<formatType>(f2);
    cout << FORMAT_NAMES[ff] << ":\n";

    string s;
    if (ff == BRIDGE_FORMAT_TXT)
      s = auction.AsString(ff, names);
    else
      s = auction.AsString(ff);

    if (s == "")
    {
      debug.Print();
      exit(0);
    }
    cout << s << "\n\n";
  }
}


void TestAuction()
{
  Auction auction;

  if (! auction.SetDealerVul("S", "EW", BRIDGE_FORMAT_PBN))
  {
    debug.Print();
    exit(0);
  }

  TestAuctionCall(auction, "P");
  TestAuctionCall(auction, "P");
  TestAuctionCallAlert(auction, "1S", "5-card majors");
  TestAuctionCall(auction, "P");
  TestAuctionCall(auction, "2S!");

  if (! auction.UndoLastCall())
  {
    debug.Print();
    exit(0);
  }

  TestAuctionCallAlert(auction, "2H", "Mixed raise");
  TestAuctionCall(auction, "P");
  TestAuctionCall(auction, "4S");

  auction.AddPasses();

  stringstream t;
  t << left << 
    setw(15) << "Garozzo" <<
    setw(15) << "Hamman" <<
    setw(15) << "Belladonna" <<
    setw(15) << "Wolff" << "\n";

  TestAuctionOutput(auction, t.str());


  auction.Reset();
  if (! auction.SetDealerVul("S", "EW", BRIDGE_FORMAT_PBN))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddAuctionRBN("SE:PP1S^1P:2H*P4SA"))
  {
    debug.Print();
    exit(0);
  }

  if (! auction.AddAlert(1, "6-card majors"))
  {
    debug.Print();
    exit(0);
  }
  
  TestAuctionOutput(auction, t.str());

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


void TestPlay()
{
  Play play;

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

  Players players;
  cout << deal.AsString(players, BRIDGE_FORMAT_TXT);

  // Test setting the play card by card.

  const char *vlist[] =
    {"DT", "D6", "DA", "D7",
     "C5", "CK", "CA", "C6",
     "CQ", "C7", "C3", "C2",
     "HQ", "HK", "H2", "H6",
     "H4", "H3", "HA", "C8",
     "H9", "D5", "H5", "D3",
     "DK", "D9", "D8", "D4",
     "D2", "DQ", "H8", "DJ",
     "S3", "S7", "SA", "S2",
     "S4", "S9", "ST", "SQ",
     "S5"
    };
  const vector<string> vvlist(vlist, end(vlist));

  for (const string &card: vvlist)
  {
    if (play.AddPlay(card) != PLAY_NO_ERROR)
    {
      debug.Print();
      assert(false);
    }
  }

  if (play.Claim(9) != PLAY_CLAIM_NO_ERROR) assert(false);
  cout << play.AsString(BRIDGE_FORMAT_LIN) << "\n";
  cout << play.ClaimAsString(BRIDGE_FORMAT_LIN) << endl; 

  cout << play.AsString(BRIDGE_FORMAT_PBN);
  cout << play.AsString(BRIDGE_FORMAT_RBN);
  cout << play.AsString(BRIDGE_FORMAT_TXT);
  cout << endl;


  play.Reset();
  play.SetContract(contract);
  if (! play.SetHoldingDDS(cards))
  {
    assert(false);
  }

  // Test setting the play one PBN trick at a time.

  const char *tlist[] =
    {"DT  D6  DA  D7  ",
     "CA  C6  C5  CK  ",
     "CQ  C7  C3  C2  ",
     "HQ  HK  H2  H6  ",
     "C8  H4  H3  HA  ",
     "D5  H5  D3  H9  ",
     "D9  D8  D4  DK  ",
     "DQ  H8  DJ  D2  ",
     "S2  S3  S7  SA  ",
     "S9  ST  SQ  S4  ",
     " -   -  S5"
    };
  const vector<string> ttlist(tlist, end(tlist));

  for (const string &trick: ttlist)
  {
    if (play.AddTrickPBN(trick) != PLAY_NO_ERROR)
    {
      debug.Print();
      assert(false);
    }
    else
    {
      cout << "trick " << trick << "\n";
      cout << play.AsString(BRIDGE_FORMAT_TXT);
    }
  }

  if (play.Claim(9) != PLAY_CLAIM_NO_ERROR) assert(false);
  cout << play.AsString(BRIDGE_FORMAT_LIN);
  cout << play.AsString(BRIDGE_FORMAT_PBN);
  cout << play.AsString(BRIDGE_FORMAT_RBN);
  cout << play.AsString(BRIDGE_FORMAT_TXT);
  cout << endl;

  play.Reset();
  play.SetContract(contract);
  if (! play.SetHoldingDDS(cards))
  {
    assert(false);
  }

  // Test setting the RBN play.

  if (play.AddAllRBN(
    "P DT6A7:C5KA6:CQ732:HQK26:H43AC8:H9D55D3:DK984:D2QH8J:S37A2:S49TQ:S5")
    != PLAY_NO_ERROR)
  {
    debug.Print();
    assert(false);
  }

  if (play.Claim(9) != PLAY_CLAIM_NO_ERROR) assert(false);
  cout << play.AsString(BRIDGE_FORMAT_LIN);
  cout << play.AsString(BRIDGE_FORMAT_PBN);
  cout << play.AsString(BRIDGE_FORMAT_RBN);
  cout << play.AsString(BRIDGE_FORMAT_TXT);
  cout << endl;

  play.Reset();
  play.SetContract(contract);
  if (! play.SetHoldingDDS(cards))
  {
    assert(false);
  }

  // Test undo.

  play.AddPlay("DT");
  play.AddPlay("D6");
  play.AddPlay("DA");
  play.AddPlay("D7");
  play.AddPlay("C5");
  play.AddPlay("CK");
  play.AddPlay("CA");
  play.AddPlay("C6");
  cout << play.AsString(BRIDGE_FORMAT_TXT);

  if (! play.UndoPlay())
  {
    cout << "Can't undo 1" << endl;
    assert(false);
  }
  if (! play.UndoPlay())
  {
    cout << "Can't undo 2" << endl;
    assert(false);
  }

  cout << play.AsString(BRIDGE_FORMAT_TXT);

  if (play.AddPlay("CQ") != PLAY_NO_ERROR)
  {
    debug.Print();
    cout << "Can't re-add 1" << endl;
    assert(false);
  }
  cout << play.AsString(BRIDGE_FORMAT_TXT);
  if (play.AddPlay("C6") != PLAY_NO_ERROR)
  {
    debug.Print();
    cout << "Can't re-add 2" << endl;
    assert(false);
  }
  cout << play.AsString(BRIDGE_FORMAT_TXT);

}


void TestDate()
{
  Date date;

  if (! date.Set("20000630", BRIDGE_FORMAT_RBN))
  {
    cout << "Can't set date" << endl;
    assert(false);
  }

  cout << "LIN: " << date.AsString(BRIDGE_FORMAT_LIN) << "\n";
  cout << "PBN: " << date.AsString(BRIDGE_FORMAT_PBN);
  cout << "RBN: " << date.AsString(BRIDGE_FORMAT_RBN);
  cout << "TXT: " << date.AsString(BRIDGE_FORMAT_TXT) << "\n";
}


void TestLocationCase(
  Location& location,
  const string& s,
  const formatType f)
{
  if (! location.Set(s, f))
  {
    cout << "Can't set location" << endl;
    assert(false);
  }

  cout << "LIN: " << location.AsString(BRIDGE_FORMAT_LIN) << "\n";
  cout << "PBN: " << location.AsString(BRIDGE_FORMAT_PBN);
  cout << "RBN: " << location.AsString(BRIDGE_FORMAT_RBN);
  cout << "TXT: " << location.AsString(BRIDGE_FORMAT_TXT) << "\n";
}


void TestLocation()
{
  Location location;

  TestLocationCase(location, "Fort Lauderdale, FL", BRIDGE_FORMAT_RBN);
  TestLocationCase(location, "Fort Lauderdale:FL", BRIDGE_FORMAT_RBN);
  TestLocationCase(location, "Fort Lauderdale, FL", BRIDGE_FORMAT_TXT);
}


void TestScoringCase(
  Scoring& scoring,
  const string& s,
  const formatType f)
{
  if (! scoring.Set(s, f))
  {
    cout << "Can't set scoring" << endl;
    assert(false);
  }

  cout << "LIN: " << scoring.AsString(BRIDGE_FORMAT_LIN) << "\n";
  cout << "PBN: " << scoring.AsString(BRIDGE_FORMAT_PBN);
  cout << "RBN: " << scoring.AsString(BRIDGE_FORMAT_RBN);
  cout << "TXT: " << scoring.AsString(BRIDGE_FORMAT_TXT) << "\n";
}


void TestScoring()
{
  Scoring scoring;

  TestScoringCase(scoring, "I", BRIDGE_FORMAT_LIN);
  TestScoringCase(scoring, "Matchpoints", BRIDGE_FORMAT_PBN);
  TestScoringCase(scoring, "R", BRIDGE_FORMAT_RBN);
}


void TestTeamsOut(const Teams& teams)
{
  cout << "LIN: " << teams.AsString(BRIDGE_FORMAT_LIN) << "\n";
  cout << "PBN: " << teams.AsString(BRIDGE_FORMAT_PBN);
  cout << "RBN: " << teams.AsString(BRIDGE_FORMAT_RBN);
  cout << "TXT: " << teams.AsString(BRIDGE_FORMAT_TXT) << "\n\n";
}


void TestTeamsCase(
  Teams& teams,
  const string s,
  const formatType f)
{
  if (! teams.Set(s, f))
  {
    cout << "Can't set teams" << endl;
    assert(false);
  }

  TestTeamsOut(teams);
}


void TestTeams()
{
  Teams teams;

  string list[2];
  TestTeamsCase(teams, "Nickel:Schwartz", BRIDGE_FORMAT_RBN);

  TestTeamsCase(teams, "Nickell:Schwartz:999:22", BRIDGE_FORMAT_RBN);

  TestTeamsCase(teams, "Nickel:Schwartz:76.33:91.50", BRIDGE_FORMAT_RBN);

  TestTeamsCase(teams, "Meltzer (470) vs. Schwartz (451)", BRIDGE_FORMAT_TXT);

  if (! teams.SetFirst("Nickell:999", BRIDGE_FORMAT_PBN))
  {
    cout << "Can't set first team" << endl;
    assert(false);
  }

  if (! teams.SetSecond("Schwartz:22", BRIDGE_FORMAT_PBN))
  {
    cout << "Can't set first team" << endl;
    assert(false);
  }

  TestTeamsOut(teams);
}


void TestSessionCase(
  Session& session,
  const string& s,
  const formatType f)
{
  if (! session.Set(s, f))
  {
    cout << "Can't set session";
    assert(false);
  }

  cout << "LIN: " << session.AsString(BRIDGE_FORMAT_LIN) << "\n";
  cout << "PBN: " << session.AsString(BRIDGE_FORMAT_PBN);
  cout << "RBN: " << session.AsString(BRIDGE_FORMAT_RBN);
  cout << "TXT: " << session.AsString(BRIDGE_FORMAT_TXT) << "\n\n";
}


void TestSession()
{
  Session session;

  TestSessionCase(session, "R32:2", BRIDGE_FORMAT_RBN);
  TestSessionCase(session, "Round of 32, Segment 7", BRIDGE_FORMAT_RBN);
  TestSessionCase(session, "Round of 32:Segment 7", BRIDGE_FORMAT_RBN);
  TestSessionCase(session, "Round of 32, Segment 7", BRIDGE_FORMAT_TXT);
  TestSessionCase(session, "Heptaquad, Segment 7", BRIDGE_FORMAT_TXT);
  TestSessionCase(session, "Pentacubed", BRIDGE_FORMAT_RBN);
}


void TestSegmentCaseOutput(Segment& segment)
{
  for (unsigned f2 = 0; f2 <= BRIDGE_FORMAT_TXT; f2++)
  {
    formatType ff = static_cast<formatType>(f2);
    cout << FORMAT_NAMES[ff] << ":\n";
    cout << segment.TitleAsString(ff) << "\n";
    cout << segment.DateAsString(ff) << "\n";
    cout << segment.LocationAsString(ff) << "\n";
    cout << segment.EventAsString(ff) << "\n";
    cout << segment.SessionAsString(ff) << "\n";
    cout << segment.ScoringAsString(ff) << "\n";
    cout << segment.TeamsAsString(ff) << "\n" << endl;
  }
}


void TestSegmentCaseLIN(
  Segment& segment,
  const string& title)
{
  if (! segment.SetTitle(title, BRIDGE_FORMAT_LIN))
  {
    cout << "Can't set LIN title";
    assert(false);
  }

  TestSegmentCaseOutput(segment);
}


void TestSegmentCaseCommon(
  Segment& segment,
  const string& title,
  const string& date,
  const string& location,
  const string& event,
  const string& session,
  const string& scoring,
  const formatType f)
{
  if (! segment.SetTitle(title, f))
  {
    cout << "Can't set title";
    assert(false);
  }

  if (! segment.SetDate(date, f))
  {
    cout << "Can't set date";
    assert(false);
  }

  if (! segment.SetLocation(location, f))
  {
    cout << "Can't set location";
    assert(false);
  }

  if (! segment.SetEvent(event, f))
  {
    cout << "Can't set event";
    assert(false);
  }

  if (! segment.SetSession(session, f))
  {
    cout << "Can't set session";
    assert(false);
  }

  if (! segment.SetScoring(scoring, f))
  {
    cout << "Can't set scoring";
    assert(false);
  }
}


void TestSegmentCasePBN(
  Segment& segment,
  const string& title,
  const string& date,
  const string& location,
  const string& event,
  const string& session,
  const string& scoring,
  const string& team1,
  const string& team2,
  const formatType f)
{
  TestSegmentCaseCommon(segment, title, date, location,
    event, session, scoring, f);

  if (! segment.SetFirstTeam(team1, f))
  {
    cout << "Can't set team1";
    assert(false);
  }

  if (! segment.SetSecondTeam(team2, f))
  {
    cout << "Can't set team2";
    assert(false);
  }

  TestSegmentCaseOutput(segment);
}


void TestSegmentCaseRBN(
  Segment& segment,
  const string& title,
  const string& date,
  const string& location,
  const string& event,
  const string& session,
  const string& scoring,
  const string& teams,
  const formatType f)
{
  TestSegmentCaseCommon(segment, title, date, location,
    event, session, scoring, f);

  if (! segment.SetTeams(teams, f))
  {
    cout << "Can't set teams";
    assert(false);
  }

  TestSegmentCaseOutput(segment);
}


void TestSegment()
{
  Segment segment;

  string list[2];

  // LIN inputs.

  TestSegmentCaseLIN(segment,
      "Bridge Base Online,IMPs,P,1,16,Meltzer,490,Schwarz,459");

  TestSegmentCaseLIN(segment,
      "#9651,Bridge Base Online,P,17,32,Meltzer,491,Schwarz,460");
    
  TestSegmentCaseLIN(segment,
      "#9651,Bridge Base Online,I,33,48,Meltzer,492,Schwarz,461");
    
  TestSegmentCaseLIN(segment,
      "BB,Semis,I,49,64,Meltzer,493,Schwarz,462");
    
  TestSegmentCaseLIN(segment,
      "BB Semifinal,Segment 4,I,65,80,Meltzer,493,Schwarz,463");

  TestSegmentCaseLIN(segment,
      "BB%20000630%Hotel Hilton:FL%R32:4,Event,I,65,80,Meltzer,493,Schwarz,463");
    
  // PBN inputs.
  
  TestSegmentCasePBN(segment,
    "Bridge Base Online",
    "2010.06.??",
    "Saratoga:CA",
    "Championship",
    "Semifinal:Segment 1",
    "IMPs",
    "Meltzer:490",
    "Schwartz:460",
    BRIDGE_FORMAT_PBN);

  // RBN inputs.
    
  TestSegmentCaseRBN(segment,
    "Bridge Base Online",
    "201006",
    "Saratoga:CA",
    "Championship",
    "Semifinal:Segment 1",
    "R",
    "Meltzer:Schwartz:495:450",
    BRIDGE_FORMAT_RBN);
}


void TestPlayers()
{
  Players players;

  if (! players.SetPlayers("+Roth:BIG+GIB:O", BRIDGE_FORMAT_RBN))
  {
    cout << "Can't set RBN players";
    assert(false);
  }

  cout << players.AsString(BRIDGE_FORMAT_RBN);

}

