/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>

#include "Buffer.h"
#include "Segment.h"
#include "Sheet.h"
#include "parse.h"
#include "Bexcept.h"


#define SHEET_INIT 20


Sheet::Sheet()
{
  Sheet::reset();
}


void Sheet::resetHeader(SheetHeader& hdr)
{
  hdr.headline = "";
  hdr.links.clear();
}


void Sheet::resetHand(SheetHand& hd)
{
  hd.label = "";

  hd.deal.reset();
  hd.hasDeal = false;

  hd.auction.reset();
  hd.hasAuction = false;

  hd.play.reset();
  hd.hasPlay = false;

  hd.contractHeader.has = false;
  hd.tricksHeader.has = false;
  hd.contractAuction.has = false;
  hd.tricksPlay.has = false;
  hd.tricksClaim.has = false;

  hd.chats.clear();
}


void Sheet::reset()
{
  hasFixed = false;

  Sheet::resetHeader(headerOrig);
  Sheet::resetHeader(headerFixed);

  handsOrig.clear();
  handsFixed.clear();

  handsOrig.reserve(SHEET_INIT);
  handsFixed.reserve(SHEET_INIT);
}


Sheet::~Sheet()
{
}


void Sheet::parseVG(
  const string& value,
  unsigned& noHdrFirst,
  unsigned& noHdrLast) const
{
  size_t c = countDelimiters(value, ",");
  if (c != 8)
    return;

  vector<string> tokens(9);
  tokens.clear();
  tokenize(value, tokens, ",");

  unsigned u;
  if (tokens[3] != "" && str2unsigned(tokens[3], u))
    noHdrFirst = u;
  else
    noHdrFirst = BIGNUM;

  if (tokens[4] != "" && str2unsigned(tokens[4], u))
    noHdrLast = u;
  else
    noHdrLast = BIGNUM;
}


void Sheet::parseRS(
  const string& value,
  vector<string>& clist) const
{
  size_t c = countDelimiters(value, ",");
  if (c > 256)
    return;

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(value, tokens, ",");

  for (unsigned i = 0; i <= c; i++)
    clist.push_back(tokens[i]);
}


void Sheet::parseBN(
  const string& value,
  vector<unsigned>& blist) const
{
  // Similar to Segment::setBoardsList().
  size_t c = countDelimiters(value, ",");
  if (c > 128)
    return;

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(value, tokens, ",");

  for (unsigned i = 0; i <= c; i++)
  {
    unsigned u;
    if (str2unsigned(tokens[i], u))
      blist.push_back(u);
    else
      blist.push_back(BIGNUM);
  }
}


string Sheet::parseQX(const string& value) const
{
  // ,BOARD etc
  size_t p = value.find(",");
  return value.substr(0, p);
}


bool Sheet::isLink(
  const string& value,
  string& link) const
{
  regex re_http("(http\\S+)");
  smatch match;
  if (regex_search(value, match, re_http))
  {
    link = match.str(1);
    return true;
  }
  return false;
}


string Sheet::qxToHeaderContract(
  const string& value,
  const vector<string>& clist,
  const vector<unsigned>& blist,
  const unsigned noHdrFirst,
  const unsigned noHdrLast) const
{
  UNUSED(blist);

  if (value.length() < 2)
    return "";

  const string room = value.substr(0, 1);
  if (room != "o" && room != "c")
    return "";

  unsigned adder = (room == "o" ? 0u : 1u);
  
  unsigned bno;
  if (! str2unsigned(value.substr(1), bno))
    return "";

  if (bno == 0 || bno < noHdrFirst || bno > noHdrLast)
    return "";

  const unsigned index = 2*(bno-1) + adder;
  if (index >= clist.size())
    return "-";
  else if (clist[index] == "")
    return "-";
  else
    return clist[index];
}


void Sheet::contractToComponents(
  const Contract& contract,
  SheetContract& contractSheet,
  SheetTricks& tricksSheet)
{
  contractSheet.value = contract.str(BRIDGE_FORMAT_LIN);
  contractSheet.has = true;

  if (contractSheet.value != "P")
  {
    tricksSheet.value = contract.getTricks();
    tricksSheet.has = true;
  }
}


void Sheet::finishHand(
  SheetHand& hand,
  const vector<string>& clist,
  const vector<unsigned>& blist,
  const unsigned noHdrFirst,
  const unsigned noHdrLast,
  const string& plays,
  const unsigned numPlays) 
{
  if (hand.auction.isOver())
    hand.hasAuction = true;

  string c = Sheet::qxToHeaderContract(hand.label,
    clist, blist, noHdrFirst, noHdrLast);
  Contract contractHdr;
  if (c == "")
  {
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
    contractHdr.setContract(c, BRIDGE_FORMAT_LIN);
    Sheet::contractToComponents(contractHdr,
      hand.contractHeader, hand.tricksHeader);
  }

  Contract contract;
  if (hand.auction.getContract(contract))
  {
    Sheet::contractToComponents(contract,
      hand.contractAuction, hand.tricksPlay);
  }

  if (numPlays > 0 && 
      (contractHdr.isSet() || contract.isSet()) &&
      hand.deal.isSet())
  {
    try
    {
      if (contractHdr.isSet())
        hand.play.setContract(contractHdr);
      else
        hand.play.setContract(contract);

      unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
      hand.deal.getDDS(cards);
      hand.play.setHoldingDDS(cards);

      hand.play.setPlays(plays, BRIDGE_FORMAT_RBN);
      hand.hasPlay = true;
      if (hand.play.isOver())
      {
        hand.tricksPlay.value = hand.play.getTricks();
        hand.tricksPlay.has = true;
      }
    }
    catch (Bexcept& bex)
    {
      bex.print(cout);
    }
  }
}


void Sheet::parse(
  Buffer& buffer,
  SheetHeader& header,
  vector<SheetHand>& hands)
{
  Segment segment;
  LineData lineData;
  SheetHand handTmp;
  Sheet::resetHand(handTmp);

  unsigned noHdrFirst, noHdrLast;
  bool hasVG = false;

  vector<string> clist;
  bool hasClist = false;

  vector<unsigned> blist;
  bool hasBlist = false;
  bool auctionFlawed = false;
  string plays = "";
  unsigned numPlays = 0;

  while (buffer.next(lineData))
  {
    if (lineData.type != BRIDGE_BUFFER_STRUCTURED)
      continue;

    if (lineData.label == "vg")
    {
      hasVG = true;

      segment.setTitle(lineData.value, BRIDGE_FORMAT_LIN_VG);
      header.headline += segment.strTitle(BRIDGE_FORMAT_TXT) + "\n";
      header.headline += segment.strEvent(BRIDGE_FORMAT_TXT);
      header.headline += segment.strSession(BRIDGE_FORMAT_TXT);
      header.headline += segment.strTeams(BRIDGE_FORMAT_TXT) + "\n\n";

      Sheet::parseVG(lineData.value, noHdrFirst, noHdrLast);
    }
    else if (lineData.label == "rs")
    {
      hasClist = true;

      Sheet::parseRS(lineData.value, clist);
    }
    else if (lineData.label == "bn")
    {
      hasBlist = true;

      Sheet::parseBN(lineData.value, blist);
    }
    else if (lineData.label == "qx")
    {
      if (handTmp.label != "")
      {
        Sheet::finishHand(handTmp, clist, blist, noHdrFirst, noHdrLast,
          plays, numPlays);
        hands.push_back(handTmp);
      }

      Sheet::resetHand(handTmp);
      auctionFlawed = false;
      plays = "";
      numPlays = 0;
      handTmp.label = Sheet::parseQX(lineData.value);
    }
    else if (lineData.label == "md")
    {
      try
      {
        handTmp.deal.set(lineData.value, BRIDGE_FORMAT_LIN);
        handTmp.hasDeal = true;
      }
      catch (Bexcept& bex)
      {
        bex.print(cout);
      }
    }
    else if (lineData.label == "mb")
    {
      try
      {
        string tmp = lineData.value;
        toUpper(tmp);
        handTmp.auction.addCall(tmp);
      }
      catch (Bexcept& bex)
      {
        if (! auctionFlawed)
          bex.print(cout);
        auctionFlawed = true;
      }
    }
    else if (lineData.label == "pc")
    {
      if (numPlays > 0 && numPlays % 4 == 0)
        plays += ":";
      plays += lineData.value;
      numPlays++;
    }
    else if (lineData.label == "mc")
    {
      if (str2unsigned(lineData.value, handTmp.tricksClaim.value))
      {
        handTmp.tricksClaim.has = true;
      }
      else
      {
        try
        {
          THROW("Bad claim");
        }
        catch (Bexcept& bex)
        {
          bex.print(cout);
        }
      }
    }
    else if (lineData.label == "nt")
    {
      handTmp.chats.push_back(lineData.value);
      string link;
      if (isLink(lineData.value, link))
        header.links.push_back(link);
    }
  }

  if (handTmp.label != "")
  {
    Sheet::finishHand(handTmp, clist, blist, noHdrFirst, noHdrLast,
      plays, numPlays);
    hands.push_back(handTmp);
  }
}


bool Sheet::read(
  const string& fname)
{
  Buffer buffer;

  if (! buffer.read(fname, BRIDGE_FORMAT_LIN))
    return false;

  try
  {
    Sheet::parse(buffer, headerOrig, handsOrig);

    if (! buffer.fix(fname))
      return true; // No ref file

    buffer.rewind();
    Sheet::parse(buffer, headerFixed, handsFixed);
    hasFixed = true;
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
    return false;
  }
  return true;
}


unsigned Sheet::findFixed(const string& label) const
{
  for (unsigned i = 0; i < handsFixed.size(); i++)
  {
    if (handsFixed[i].label == label)
      return i;
  }
  return BIGNUM;
}


string Sheet::cstr(const SheetContract& ct) const
{
  if (! ct.has)
    return "-";
  else
    return ct.value;
}


string Sheet::cstr(
  const SheetContract& ct,
  const SheetContract& cbase) const
{
  if (! ct.has)
    return "-";
  else if (! cbase.has)
    return ct.value;
  else if (ct.value == cbase.value)
    return "=";
  else
    return ct.value;
}


string Sheet::tstr(const SheetTricks& tr) const
{
  if (! tr.has)
    return "-";
  else
    return STR(tr.value);
}


string Sheet::tstr(
  const SheetTricks& tr,
  const SheetTricks& tbase) const
{
  if (! tr.has)
    return "-";
  else if (! tbase.has)
    return STR(tr.value);
  else if (tr.value == tbase.value)
    return "=";
  else
    return STR(tr.value);
}


bool Sheet::contractsDiffer(
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


bool Sheet::tricksDiffer(
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


bool Sheet::handsDiffer(
  const SheetHand& ho,
  const SheetHand& hf) const
{
  if (Sheet::contractsDiffer(ho.contractHeader, hf.contractHeader))
    return true;

  if (Sheet::tricksDiffer(ho.tricksHeader, hf.tricksHeader))
    return true;

  if (Sheet::contractsDiffer(ho.contractAuction, hf.contractAuction))
    return true;

  if (Sheet::tricksDiffer(ho.tricksPlay, hf.tricksPlay))
    return true;

  if (Sheet::tricksDiffer(ho.tricksClaim, hf.tricksClaim))
    return true;

  return false;
}


void Sheet::extendNotes(
  const SheetHand& ho,
  const SheetHand& hf,
  stringstream& notes) const
{
  if (ho.hasDeal)
  {
    notes << ho.deal.str(BRIDGE_NORTH, BRIDGE_FORMAT_TXT) << "\n";
    if (hf.hasDeal && ho.deal != hf.deal)
      notes << "Original and fixed deals differ\n";
  }
  else if (hf.hasDeal)
  {
    notes << "No original deal -- fixed deal:\n";
    notes << hf.deal.str(BRIDGE_NORTH, BRIDGE_FORMAT_TXT) << "\n";
  }

  if (ho.hasPlay)
  {
    notes << ho.play.str(BRIDGE_FORMAT_TXT) << "\n";
    if (hf.hasPlay && ho.play != hf.play)
      notes << "Original and fixed plays differ\n";
  }
  else if (hf.hasPlay)
  {
    notes << "No original play -- fixed play:\n";
    notes << hf.play.str(BRIDGE_FORMAT_TXT) << "\n";
  }

  for (auto &line: ho.chats)
  {
    notes << line << "\n";
  }

  if (ho.hasDeal || ho.hasPlay)
    notes << "----------\n\n";
}
  

string Sheet::str() const
{
  bool hasDiffs = false;
  stringstream ss;
  ss << headerOrig.headline << "\n\n";

  if (headerOrig.links.size() > 0)
  {
    ss << "Links:\n";
    for (auto &s: headerOrig.links)
      ss << s << "\n";
    ss << "\n";
  }

  ss << setw(6) << "Board " << 
    setw(6) << "Chdr" <<
    setw(6) << "Thdr" <<
    setw(6) << "Cauct" <<
    setw(6) << "Tplay" <<
    setw(6) << "Tclm" <<
    setw(6) << "Chdr" <<
    setw(6) << "Thdr" <<
    setw(6) << "Cauct" <<
    setw(6) << "Tplay" <<
    setw(6) << "Tclm" << "\n";

  stringstream notes;
  for (auto &ho: handsOrig)
  {
    ss << setw(5) << ho.label << " " <<
      setw(6) << Sheet::cstr(ho.contractHeader) <<
      setw(6) << Sheet::tstr(ho.tricksHeader) <<
      setw(6) << Sheet::cstr(ho.contractAuction, ho.contractHeader) <<
      setw(6) << Sheet::tstr(ho.tricksPlay, ho.tricksHeader) <<
      setw(6) << Sheet::tstr(ho.tricksClaim, ho.tricksHeader);

    const unsigned index = Sheet::findFixed(ho.label);
    if (index == BIGNUM)
    {
      ss << setw(6) << "-" <<
        setw(6) << "-" <<
        setw(6) << "-" <<
        setw(6) << "-" <<
        setw(6) << "-" << "\n";
    }
    else
    {
      const SheetHand& hf = handsFixed[index];
      ss <<
        setw(6) << Sheet::cstr(hf.contractHeader, ho.contractHeader) <<
        setw(6) << Sheet::tstr(hf.tricksHeader, ho.tricksHeader) <<
        setw(6) << Sheet::cstr(hf.contractAuction, ho.contractHeader) <<
        setw(6) << Sheet::tstr(hf.tricksPlay, ho.tricksHeader) <<
        setw(6) << Sheet::tstr(hf.tricksClaim, ho.tricksHeader) << "\n";

      if (Sheet::handsDiffer(ho, hf))
      {
        notes << "Board " << ho.label << "\n";
        Sheet::extendNotes(ho, hf, notes);
        hasDiffs = true;
      }
    }
  }

  if (hasDiffs)
    return ss.str() + "\n" + notes.str();
  else
    return "";
}

