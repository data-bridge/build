/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>

#include "SheetHand.h"
#include "parse.h"
#include "ddsIF.h"
#include "Bexcept.h"
#include "Bdiff.h"


#define SHEET_INIT 20


SheetHand::SheetHand()
{
  SheetHand::reset();
}


void SheetHand::reset()
{
  deal.reset();
  hasDeal = false;

  auction.reset();
  hasAuction = false;
  auctionFlawed = false;
  auctionFlaw = "";

  play.reset();
  hasPlay = false;
  playFlawed = false;

  playDistance.numTricks = 0;
  playDistance.numCards = 0;
  playDistance.goodTricks = 0;
  playDistance.distance = 0;

  cHeader.reset();
  cAuction.reset();

  contractHeader.has = false;
  tricksHeader.has = false;
  contractAuction.has = false;
  tricksPlay.has = false;
  tricksClaim.has = false;

  chats.clear();
}


SheetHand::~SheetHand()
{
}


void SheetHand::strToContract(
  const Contract& contract,
  const SheetContractType type)
{
  // Drop leading "C ", colon and trailing newline.
  string st = contract.str(BRIDGE_FORMAT_RBN);
  const size_t l = st.length();

  SheetContract& sc = (type == SHEET_CONTRACT_HEADER ?
    contractHeader : contractAuction);

  if (l >= 4)
  {
    const size_t p = st.find(":");
    if (p == string::npos || p == l-1)
      sc.value = st.substr(2, l-3);
    else
      sc.value = st.substr(2, p-2) + st.substr(p+1, l-p-2);

    sc.has = true;
  }
}


void SheetHand::strToTricks(
  const Contract& contract,
  const SheetTricksType type)
{
  SheetContract& sc = (type == SHEET_CONTRACT_HEADER ?
    contractHeader : contractAuction);

  if (sc.value != "P")
  {
    switch (type)
    {
      case SHEET_TRICKS_HEADER:
        tricksHeader.value = contract.getTricks();
        if (tricksHeader.value > 13)
          // Happens when header is of the form "6DS" without tricks.
          tricksHeader.has = false;
        else
          tricksHeader.has = true;
        break;

      case SHEET_TRICKS_PLAY:
        tricksPlay.value = contract.getTricks();
        tricksPlay.has = true;
        break;

      case SHEET_TRICKS_CLAIM:
        tricksClaim.value = contract.getTricks();
        tricksClaim.has = true;
        break;

      default:
        return;
    }
  }
}


bool SheetHand::setDeal(const string& text)
{
  try
  {
    deal.set(text, BRIDGE_FORMAT_LIN);
    hasDeal = true;

    unsigned u;
    if (str2unsigned(text.substr(0, 1), u) && u >= 1 && u <= 4)
    {
      string dealer = PLAYER_NAMES_SHORT[PLAYER_LIN_DEALER_TO_DDS[u]];
      auction.setDealer(dealer, BRIDGE_FORMAT_RBN);
      auction.setVul("o", BRIDGE_FORMAT_RBN);
    }
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
    return false;
  }
  return true;
}


bool SheetHand::addCall(const string& text)
{
  try
  {
    string tmp = text;
    toUpper(tmp);
    if (tmp.length() > 3)
    {
      trimLeading(tmp, '-');
      tmp = trimTrailing(tmp, '-');
      auction.addAuction(tmp, BRIDGE_FORMAT_LIN);
    }
    else
      auction.addCall(tmp);
  }
  catch (Bexcept& bex)
  {
    auctionFlawed = true;
    auctionFlaw = bex.getMessage();
    return false;
  }
  return true;
}


bool SheetHand::claim(const string& text)
{
  if (str2unsigned(text, tricksClaim.value))
  {
    tricksClaim.has = true;
    return true;
  }
  else
    return false;
}


void SheetHand::addChat(const string& text)
{
  chats.push_back(text);
}


void SheetHand::incrPlayDistance(const string& trick)
{
  const unsigned l = trick.length();
  if (l % 2 || l > 8)
    SheetHand::fail("Bad trick: " + trick);

  if (l == 0)
    return;

  const unsigned c = l >> 1;
  const int ci = static_cast<int>(c);
  vector<int> cards(4);
  for (unsigned i = 0; i < c; i++)
  {
    cards[i] = static_cast<int>(deal.holdsCard(trick.substr(2*i, 2)));
    if (cards[i] == BRIDGE_PLAYER_SIZE)
      SheetHand::fail("Bad card in: " + trick);
  }

  unsigned distBest = c-1;
  for (int startPos = 0; startPos < ci; startPos++)
  {
    const int start = cards[static_cast<unsigned>(startPos)];
    unsigned dist = 0;
    for (int i = 1; i < ci; i++)
    {
      const int pos = (startPos+i) % ci;
      const int expectPlayer = (start+i) % BRIDGE_PLAYERS;
      if (cards[static_cast<unsigned>(pos)] != expectPlayer)
        dist++;
    }

    if (dist < distBest)
      distBest = dist;
  }

  playDistance.numTricks++;
  playDistance.numCards += c;
  if (distBest == 0)
    playDistance.goodTricks++;
  playDistance.distance += distBest;
}


void SheetHand::setPlayDistance(const string& plays)
{
  vector<string> tricks(13);
  tricks.clear();
  tokenize(plays, tricks, ":");

  for (unsigned t = 0; t < tricks.size(); t++)
  {
    toUpper(tricks[t]);
    SheetHand::incrPlayDistance(tricks[t]);
  }
}


void SheetHand::fail(const string& text) const
{
  // Ugly way to fail.
  try
  {
    THROW(text);
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
    THROW("SheetHand fail");
  }
}


void SheetHand::finishHand(
  const string& ct,
  const string& plays,
  const unsigned numPlays) 
{
  if (auction.isOver())
    hasAuction = true;

  if (ct == "")
  {
    SheetHand::fail("Bad board");
  }
  else if (ct == "=")
  {
    return;
  }
  else
  {
    cHeader.setContract(BRIDGE_VUL_NONE, ct);
    SheetHand::strToContract(cHeader, SHEET_CONTRACT_HEADER);
    SheetHand::strToTricks(cHeader, SHEET_TRICKS_HEADER);
  }

  if (auction.getContract(cAuction))
    SheetHand::strToContract(cAuction, SHEET_CONTRACT_AUCTION);

  if (numPlays > 0 && 
      (cHeader.isSet() || cAuction.isSet()) &&
      deal.isSet())
  {
    try
    {
      if (cHeader.isSet())
      {
        // Header may incorrectly list "PASS".
        if ((ct == "PASS" || ct == "P") && cAuction.isSet())
          play.setContract(cAuction);
        else
          play.setContract(cHeader);
      }
      else
        play.setContract(cAuction);
    }
    catch (Bexcept& bex)
    {
      bex.print(cout);
    }

    try
    {
      unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
      deal.getDDS(cards);
      play.setHoldingDDS(cards);
    }
    catch (Bexcept& bex)
    {
      bex.print(cout);
    }

    try
    {
      play.setPlays(plays, BRIDGE_FORMAT_LIN_VG);
      hasPlay = true;
      playDistance.numTricks = play.getTricks();
      playDistance.goodTricks = playDistance.numTricks;
      playDistance.numCards = numPlays;
      if (play.isOver())
      {
        tricksPlay.value = playDistance.numTricks;
        tricksPlay.has = true;
      }
    }
    catch (Bexcept& bex)
    {
      playFlawed = true;
      SheetHand::setPlayDistance(plays);
      UNUSED(bex);
      // bex.print(cout);
    }
  }
}


bool SheetHand::contractsOrTricksDiffer() const
{
  if (SheetHand::contractsDiffer())
    return true;

  if (cHeader.isPassedOut() && cAuction.isPassedOut())
    return false;

  if (SheetHand::tricksDiffer(tricksHeader, tricksPlay))
    return true;

  if (SheetHand::tricksDiffer(tricksHeader, tricksClaim))
    return true;

  unsigned tmin = BRIDGE_TRICKS+1;
  unsigned tmax = 0;
  if (tricksHeader.has)
  {
    if (tricksHeader.value > tmax) tmax = tricksHeader.value;
    if (tricksHeader.value < tmin) tmin = tricksHeader.value;
  }
  if (tricksPlay.has)
  {
    if (tricksPlay.value > tmax) tmax = tricksPlay.value;
    if (tricksPlay.value < tmin) tmin = tricksPlay.value;
  }
  if (tricksClaim.has)
  {
    if (tricksClaim.value > tmax) tmax = tricksClaim.value;
    if (tricksClaim.value < tmin) tmin = tricksClaim.value;
  }
    
  RunningDD runningDD;
  play.getStateDDS(runningDD);
  if (tmin < runningDD.tricksDecl)
    return true;
  if (tmax + runningDD.tricksDef > BRIDGE_TRICKS)
    return true;

  return false;
}


bool SheetHand::hasData() const
{
  return (hasDeal || hasPlay);
}


bool SheetHand::auctionIsFlawed() const
{
  return (auctionFlawed || (! auction.isEmpty() && ! auction.isOver()));
}


bool SheetHand::playIsFlawed() const
{
  return playFlawed;
}


SheetPlayType SheetHand::playValidity() const
{
  // Expected value if random is about 2 distance units per trick.

  if (playDistance.numCards < 4)
  {
    return (playDistance.distance == 0 ? SHEET_PLAY_OPEN : SHEET_PLAY_BAD);
  }
  else if (playDistance.numCards < 8)
  {
    return (playDistance.distance <= 1 ? SHEET_PLAY_OPEN : SHEET_PLAY_BAD);
  }
  else
  {
    return (10 * playDistance.distance <= playDistance.numCards ? 
      SHEET_PLAY_OK : SHEET_PLAY_BAD);
  }
}


const SheetPlayDistance& SheetHand::getPlayDistance() const
{
  return playDistance;
}


string SheetHand::cstr(const SheetContract& ct) const
{
  if (! ct.has)
    return "-";
  else
    return ct.value;
}


string SheetHand::cstr(
  const SheetContract& ct,
  const SheetContract& cbase) const
{
  if (! ct.has)
    return "-";
  else if (! cbase.has)
    return ct.value;
  else if (ct.value == cbase.value)
    return ".";
  else
    return ct.value;
}


string SheetHand::tstr(const SheetTricks& tr) const
{
  if (! tr.has)
    return "-";
  else
    return STR(tr.value);
}


string SheetHand::tstr(
  const SheetTricks& tr,
  const SheetTricks& tbase) const
{
  if (! tr.has)
    return "-";
  else if (! tbase.has)
    return STR(tr.value);
  else if (tr.value == tbase.value)
    return ".";
  else
    return STR(tr.value);
}


bool SheetHand::contractsDiffer() const
{
  if (contractHeader.has)
  {
    if (contractAuction.has)
      return (contractHeader.value != contractAuction.value);
    else
      // May just be missing, so we accept
      return false;
  }
  else
    return contractAuction.has;
}


bool SheetHand::tricksDiffer(
  const SheetTricks& tr,
  const SheetTricks& tf) const
{
  if (! tr.has)
    return tf.has;
  else if (! tf.has)
    return false;
  else
    return (tr.value != tf.value);
}


string SheetHand::suggestTrick(
  const SheetTricks& tricks1,
  const SheetTricks& tricks2,
  const SheetTricks& tbase) const
{
  if (tricks1.has)
  {
    if (! tricks2.has)
    {
      if (tricks1.value != tbase.value)
        return STR(tricks1.value);
      else
        return "Y"; // No ideas
    }
    else if (tricks1.value == tricks2.value)
    {
      if (tricks1.value != tbase.value)
        return STR(tricks1.value);
      else
        return "Y";
    }
    else if (tricks1.value == tbase.value)
      return STR(tricks2.value);
    else if (tricks2.value == tbase.value)
      return STR(tricks1.value);
    else
      return "X"; // Too many different values
  }
  else if (tricks2.has)
  {
    if (tricks2.value != tbase.value)
      return STR(tricks2.value);
    else
      return "Y";
  }
  else
    return "Y";
}


string SheetHand::tricksAlt() const
{
  const SheetTricks& tbase = (tricksClaim.has ? tricksClaim : tricksPlay);

  return SheetHand::suggestTrick(tricksHeader, tricksDDS, tbase);
}


string SheetHand::strContractHeader()
{
  if (tricksHeader.has)
  {
    cHeader.setTricks(tricksHeader.value);
    return cHeader.str(BRIDGE_FORMAT_LIN);
  }
  else
    return contractHeader.value;
}


string SheetHand::strContractAuction()
{
  if (tricksClaim.has)
  {
    cAuction.setTricks(tricksClaim.value);
    return cAuction.str(BRIDGE_FORMAT_LIN);
  }
  else if (tricksPlay.has)
  {
    cAuction.setTricks(tricksPlay.value);
    return cAuction.str(BRIDGE_FORMAT_LIN);
  }
  else
    return contractAuction.value;
}


string SheetHand::strContractTag() const
{
  return cHeader.strDiffTag(cAuction);
}


string SheetHand::strNotesDetail()
{
  stringstream ss;
  RunningDD runningDD;
  play.getStateDDS(runningDD);

  Deal dl;
  dl.set(runningDD.dl.remainCards);

  if (! auction.isEmpty())
  {
    const int lengths[4] = {12, 12, 12, 12};
    ss << auction.str(BRIDGE_FORMAT_TXT, lengths) << "\n";

    if (auctionIsFlawed())
    {
      if (! auction.isOver() && auctionFlaw == "")
        ss << "Auction error message: Auction not over\n\n";
      else
        ss << "Auction error message: " << auctionFlaw << "\n\n";
    }
  }

  ss << dl.str(auction.getDealer(), BRIDGE_FORMAT_TXT) << "\n";

  ss << "Contract: " << contractHeader.value << "\n";
  ss << "Running : " << runningDD.tricksDecl << " to " <<
    runningDD.tricksDef << "\n\n";
  if (tricksHeader.has)
    ss << "Header  : " << tricksHeader.value << "\n";
  if (tricksPlay.has)
    ss << "Play    : " << tricksPlay.value << "\n";
  if (tricksClaim.has)
    ss << "Claim   : " << tricksClaim.value << "\n";

  if (! play.isOver())
  {
    try
    {
      tricksDDS.has = true;
      tricksDDS.value = tricksDD(runningDD);
    }
    catch (Bexcept& bex)
    {
      bex.print(cout);
    }
    ss << "DD      : " << tricksDDS.value << "\n\n";
  }

  return ss.str();
}


string SheetHand::strNotes()
{
  stringstream ss;
  if (hasDeal)
    ss << deal.str(BRIDGE_NORTH, BRIDGE_FORMAT_TXT) << "\n";

  if (hasPlay || play.getTricks() > 0)
    ss << play.str(BRIDGE_FORMAT_TXT) << "\n";
  ss << SheetHand::strNotesDetail();

  return ss.str();
}


string SheetHand::strChat() const
{
  string st;
  for (auto &line: chats)
    st += line + "\n";
  return st + "\n";
}
  

string SheetHand::str() const
{
  stringstream ss;
  ss << 
    setw(6) << SheetHand::cstr(contractHeader) <<
    setw(6) << SheetHand::tstr(tricksHeader) <<
    setw(6) << SheetHand::cstr(contractAuction, contractHeader) <<
    setw(6) << SheetHand::tstr(tricksPlay, tricksHeader) <<
    setw(6) << SheetHand::tstr(tricksClaim, tricksHeader);

  return ss.str();
}


string SheetHand::str(
  const SheetHand& href) const
{
  stringstream ss;
  ss <<
    setw(6) << SheetHand::cstr(contractHeader, href.contractHeader) <<
    setw(6) << SheetHand::tstr(tricksHeader, href.tricksHeader) <<
    setw(6) << SheetHand::cstr(contractAuction, href.contractHeader) <<
    setw(6) << SheetHand::tstr(tricksPlay, href.tricksHeader) <<
    setw(6) << SheetHand::tstr(tricksClaim, href.tricksHeader);

  return ss.str();
}


string SheetHand::strDummy() const
{
  stringstream ss;
  ss << setw(6) << "-" <<
    setw(6) << "-" <<
    setw(6) << "-" <<
    setw(6) << "-" <<
    setw(6) << "-" << "\n";

  return ss.str();
}

