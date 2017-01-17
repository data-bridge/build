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

  play.reset();
  hasPlay = false;

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
    auction.addCall(tmp);
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
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


void SheetHand::finishHand(
  const string& ct,
  const string& plays,
  const unsigned numPlays) 
{
  if (auction.isOver())
    hasAuction = true;

  Contract contractHdr;
  if (ct == "")
  {
    // Ugly way to fail.
    try
    {
      THROW("Bad board");
    }
    catch (Bexcept& bex)
    {
      bex.print(cout);
    }
  }
  else
  {
    contractHdr.setContract(ct, BRIDGE_FORMAT_LIN);
    SheetHand::strToContract(contractHdr, SHEET_CONTRACT_HEADER);
    SheetHand::strToTricks(contractHdr, SHEET_TRICKS_HEADER);
  }

  Contract contract;
  if (auction.getContract(contract))
    SheetHand::strToContract(contract, SHEET_CONTRACT_AUCTION);

  if (numPlays > 0 && 
      (contractHdr.isSet() || contract.isSet()) &&
      deal.isSet())
  {
    try
    {
      if (contractHdr.isSet())
        play.setContract(contractHdr);
      else
        play.setContract(contract);

      unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
      deal.getDDS(cards);
      play.setHoldingDDS(cards);

      play.setPlays(plays, BRIDGE_FORMAT_RBN);
      hasPlay = true;
      if (play.isOver())
      {
        tricksPlay.value = play.getTricks();
        tricksPlay.has = true;
      }
    }
    catch (Bexcept& bex)
    {
      bex.print(cout);
    }
  }
}


bool SheetHand::operator ==(const SheetHand& href) const
{
  return (! (* this == href));
}


bool SheetHand::operator !=(const SheetHand& href) const
{
  if (SheetHand::contractsDiffer(contractHeader, href.contractHeader))
    return true;

  if (SheetHand::tricksDiffer(tricksHeader, href.tricksHeader))
    return true;

  if (SheetHand::contractsDiffer(contractAuction, href.contractAuction))
    return true;

  if (SheetHand::tricksDiffer(tricksPlay, href.tricksPlay))
    return true;

  if (SheetHand::tricksDiffer(tricksClaim, href.tricksClaim))
    return true;

  return false;
}


bool SheetHand::hasData() const
{
  return (hasDeal || hasPlay);
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


bool SheetHand::contractsDiffer(
  const SheetContract& ct,
  const SheetContract& cf) const
{
  if (ct.has != cf.has)
    return true;
  else if (ct.has && ct.value != cf.value)
    return true;
  else
    return false;
}


bool SheetHand::tricksDiffer(
  const SheetTricks& tr,
  const SheetTricks& tf) const
{
  if (tr.has != tf.has)
    return true;
  else if (tr.has && tr.value != tf.value)
    return true;
  else
    return false;
}


string SheetHand::strNotesDetail() const
{
  stringstream ss;
  RunningDD runningDD;
  play.getStateDDS(runningDD);

  Deal dl;
  dl.set(runningDD.dl.remainCards);

  if (hasAuction)
  {
    const int lengths[4] = {12, 12, 12, 12};
    ss << auction.str(BRIDGE_FORMAT_TXT, lengths) << "\n";
  }

  unsigned ddTricks;
  try
  {
    ddTricks = tricksDD(runningDD);
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
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
  ss << "DD      : " << ddTricks << "\n\n";

  return ss.str();
}


string SheetHand::strNotes(const SheetHand& href) const
{
  stringstream ss;
  if (hasDeal)
  {
    ss << deal.str(BRIDGE_NORTH, BRIDGE_FORMAT_TXT) << "\n";
    if (href.hasDeal && deal != href.deal)
      ss << "Original and fixed deals differ\n";
  }
  else if (href.hasDeal)
  {
    ss << "No original deal -- fixed deal:\n";
    ss << href.deal.str(BRIDGE_NORTH, BRIDGE_FORMAT_TXT) << "\n";
  }

  if (hasPlay)
  {
    ss << play.str(BRIDGE_FORMAT_TXT) << "\n";
    ss << SheetHand::strNotesDetail();

    if (href.hasPlay && play != href.play)
      ss << "Original and fixed plays differ\n";

  }
  else if (href.hasPlay)
  {
    ss << "No original play -- fixed play:\n";
    ss << href.play.str(BRIDGE_FORMAT_TXT) << "\n";
    ss << href.strNotesDetail();
  }

  return ss.str();
}


string SheetHand::strChat() const
{
  string st;
  for (auto &line: chats)
    st += line + "\n";
  return st;
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

