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
#include "ddsIF.h"
#include "Bexcept.h"
#include "Bdiff.h"


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


void Sheet::resetHand(SheetHandData& hd)
{
  hd.label = "";
  hd.room = "";
  hd.numberQX = BIGNUM;
  hd.lineLIN = BIGNUM;
  hd.hand.reset();
  hd.refSource.clear();
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
  SheetHandData& hd,
  const vector<string>& clist,
  const vector<unsigned>& blist,
  const unsigned noHdrFirst,
  const unsigned noHdrLast)
{
  UNUSED(blist);

  if (hd.label.length() < 2)
    return "";

  hd.room = hd.label.substr(0, 1);
  if (hd.room != "o" && hd.room != "c")
    return "";

  unsigned adder = (hd.room == "o" ? 0u : 1u);
  
  if (! str2unsigned(hd.label.substr(1), hd.numberQX))
    return "";

  if (hd.numberQX == 0 || 
      hd.numberQX < noHdrFirst || 
      hd.numberQX > noHdrLast)
    return "";

  const unsigned index = 2*(hd.numberQX-noHdrFirst) + adder;
  if (index >= clist.size())
    return "-";
  else if (clist[index] == "")
    return "-";
  else
    return clist[index];
}


void Sheet::fail(const string& text) const
{
  // Ugly way to fail.
  try
  {
    THROW(text);
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
  }
}


void Sheet::parse(
  Buffer& buffer,
  SheetHeader& header,
  vector<SheetHandData>& hands)
{
  Segment segment;
  LineData lineData;
  SheetHandData handTmp;
  Sheet::resetHand(handTmp);

  unsigned noHdrFirst, noHdrLast;

  vector<string> clist;
  vector<unsigned> blist;

  bool auctionFlawed = false;
  string plays = "";
  unsigned numPlays = 0;

  while (buffer.next(lineData, false))
  {
    if (lineData.type != BRIDGE_BUFFER_STRUCTURED)
      continue;

    if (lineData.label == "vg")
    {
      segment.setTitle(lineData.value, BRIDGE_FORMAT_LIN_VG);
      header.headline += segment.strTitle(BRIDGE_FORMAT_TXT) + "\n";
      header.headline += segment.strEvent(BRIDGE_FORMAT_TXT);
      header.headline += segment.strSession(BRIDGE_FORMAT_TXT);
      header.headline += segment.strTeams(BRIDGE_FORMAT_TXT) + "\n\n";

      Sheet::parseVG(lineData.value, noHdrFirst, noHdrLast);
    }
    else if (lineData.label == "rs")
    {
      header.lineRS = lineData.no;
      Sheet::parseRS(lineData.value, clist);
    }
    else if (lineData.label == "bn")
    {
      Sheet::parseBN(lineData.value, blist);
    }
    else if (lineData.label == "qx")
    {
      if (handTmp.label != "")
      {
        string c = Sheet::qxToHeaderContract(handTmp,
          clist, blist, noHdrFirst, noHdrLast);
        handTmp.hand.finishHand(c, plays, numPlays);
        hands.push_back(handTmp);
      }

      Sheet::resetHand(handTmp);
      auctionFlawed = false;
      plays = "";
      numPlays = 0;
      handTmp.label = Sheet::parseQX(lineData.value);
      handTmp.lineLIN = lineData.no;
    }
    else if (lineData.label == "md")
    {
      (void) handTmp.hand.setDeal(lineData.value);
    }
    else if (lineData.label == "mb")
    {
      if (! auctionFlawed)
      {
        if (! handTmp.hand.addCall(lineData.value))
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
      if (! handTmp.hand.claim(lineData.value))
        Sheet::fail("Bad claim");
    }
    else if (lineData.label == "nt")
    {
      handTmp.hand.addChat(lineData.value);
      string link;
      if (isLink(lineData.value, link))
        header.links.push_back(link);
    }
  }

  if (handTmp.label != "")
  {
    string c = Sheet::qxToHeaderContract(handTmp,
      clist, blist, noHdrFirst, noHdrLast);
    handTmp.hand.finishHand(c, plays, numPlays);
    hands.push_back(handTmp);
  }
}


unsigned Sheet::refLineNoToHandNo(const unsigned lineNo) const
{
  const unsigned l = handsOrig.size();
  if (lineNo < handsOrig[0].lineLIN)
    return BIGNUM; // Header

  for (unsigned i = 0; i < l-1; i++)
  {
    if (lineNo < handsOrig[i+1].lineLIN)
      return i;
  }

  if (lineNo >= handsOrig[l-1].lineLIN)
    return l-1;
  else
    return BIGNUM;
}


void Sheet::parseRefs(const Buffer& buffer)
{
  // refFix contains a list of line entries from ref file.
  // For each entry, refEffects contains the type of change
  // (e.g. the mc claim is wrong) and a list of hand numbers
  // affected by it.
  // For each hand, refSource contains a list of line entries
  // affecting that hand.
  
  for (unsigned refNo = 0; refNo < refFix.size(); refNo++)
  {
    // TODO: Could be the rs line

    if (refFix[refNo].partialFlag)
      continue;

    const unsigned handNoFirst = 
      Sheet::refLineNoToHandNo(refFix[refNo].lno);
    const unsigned handNoLast = 
      (refFix[refNo].count == 1 ?  handNoFirst : 
      Sheet::refLineNoToHandNo(
        refFix[refNo].lno + refFix[refNo].count - 1));

    if (handNoFirst == BIGNUM || handNoLast == BIGNUM)
      continue;

    for (unsigned hno = handNoFirst; hno <= handNoLast; hno++)
    {
      handsOrig[hno].refSource.push_back(refNo);
      refEffects[refNo].list.push_back(hno);
    }

    RefErrorClass refError;
    classifyRefLine(refFix[refNo],
      buffer.getLine(refFix[refNo].lno), refError);
    refEffects[refNo].type = refError.code;
    refEffects[refNo].numTags = refError.numTags;
    refEffects[refNo].line = strRefFix(refFix[refNo]);
  }
}


bool Sheet::read(
  const string& fname)
{
  Buffer buffer;

  if (! buffer.read(fname, BRIDGE_FORMAT_LIN))
    return false;

  if (! buffer.fix(fname, BRIDGE_REF_ONLY_PARTIAL))
  {
    // Ignore -- file may be missing, or no partials.
  }

  try
  {
    Sheet::parse(buffer, headerOrig, handsOrig);

    readRefFix(fname, refFix);
    refEffects.resize(refFix.size());
    Sheet::parseRefs(buffer);

    if (! buffer.fix(fname, BRIDGE_REF_ONLY_NONPARTIAL))
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


unsigned Sheet::findOrig(const string& label) const
{
  for (unsigned i = 0; i < handsOrig.size(); i++)
  {
    if (handsOrig[i].label == label)
      return i;
  }
  return BIGNUM;
}


string Sheet::str() const
{
  if (! hasFixed)
    return "";

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
    ss << setw(5) << ho.label << " " << ho.hand.str();

    const unsigned index = Sheet::findFixed(ho.label);
    bool hasDiff = false;
    if (index == BIGNUM)
    {
      ss << ho.hand.strDummy();
      hasDiff = true;
    }
    else
    {
      const SheetHandData& hf = handsFixed[index];
      ss << hf.hand.str(ho.hand) << "\n";

      try
      {
        if (ho.hand != hf.hand)
          hasDiff = true;
      }
      catch (Bdiff& bdiff)
      {
        hasDiff = true;
	bdiff.print(cout);
      }
    }

    if (hasDiff)
    {
      hasDiffs = true;
      notes << "Board " << ho.label << "\n";
      if (index == BIGNUM)
        notes << ho.hand.strNotes();
      else
        notes << ho.hand.strNotes(handsFixed[index].hand);
      notes << ho.hand.strChat();

      const unsigned indexOrig = Sheet::findOrig(ho.label);
      if (indexOrig != BIGNUM)
      {
        const unsigned l = handsOrig.size();
        if (indexOrig < l-1)
        {
          notes << "--\n";
          notes << handsOrig[indexOrig+1].hand.strChat();
          if (indexOrig < l-2)
          {
            notes << "--\n";
            notes << handsOrig[indexOrig+2].hand.strChat();
          }
        }
      }

      notes << "\nActive ref lines: " << ho.label << "\n";
      for (auto &no: ho.refSource)
        notes << refEffects[no].line << " {" << 
        RefErrors[refEffects[no].type].name << "(" <<
        refEffects[no].numTags << ",1,1)}\n";

      notes << "\n----------\n\n";
    }
  }

  if (hasDiffs)
    return ss.str() + "\n" + notes.str();
  else
    return "";
}

