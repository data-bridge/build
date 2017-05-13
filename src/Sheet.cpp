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
  {
    // In case of multiple segments, only take the first one.
    if (noHdrFirst == 0)
      noHdrFirst = u;
  }
  else
    noHdrFirst = BIGNUM;

  if (tokens[4] != "" && str2unsigned(tokens[4], u))
  {
    // In case of multiple segments, take the last one.
    noHdrLast = u;
  }
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
    return "=";
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
    THROW("Sheet fail");
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

  unsigned noHdrFirst = 0, noHdrLast = 0;

  vector<string> clist;
  vector<unsigned> blist;

  bool auctionFlawed = false;
  string plays = "";
  unsigned numPlays = 0;

  while (buffer.next(lineData, false))
  {
    if (lineData.type != BRIDGE_BUFFER_STRUCTURED)
      continue;

    toLower(lineData.label);

    if (lineData.label == "vg")
    {
      segment.setTitle(lineData.value, BRIDGE_FORMAT_LIN_VG);

      string st;

      header.headline += segment.strTitle(BRIDGE_FORMAT_TXT);

      st = segment.strEvent(BRIDGE_FORMAT_TXT);
      if (st.length() > 1)
        header.headline += st;

      st = segment.strSession(BRIDGE_FORMAT_TXT);
      if (st.length() > 1)
        header.headline += st;

      st = segment.strTeams(BRIDGE_FORMAT_TXT);
      if (st.length() > 1)
        header.headline += st;

      header.headline += "\n";

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


void Sheet::parseRefs()
{
  // reflines contains a list of line entries from ref file.
  // For each hand, refSource contains a list of ref line numbers
  // affecting that hand.
  
  for (auto &rl: reflines)
  {
    if (rl.isCommented())
      continue;

    const unsigned handNoFirst = Sheet::refLineNoToHandNo(rl.lineno());
    const unsigned handNoLast = (rl.deletion() <= 1 ?  handNoFirst : 
      Sheet::refLineNoToHandNo(rl.lineno() + rl.deletion() - 1));

    if (handNoFirst == BIGNUM || handNoLast == BIGNUM)
      continue;

    for (unsigned hno = handNoFirst; hno <= handNoLast; hno++)
      handsOrig[hno].refSource.push_back(rl.line());
  }
}


bool Sheet::read(
  const string& fname)
{
  Buffer buffer;

  try
  {
    if (! buffer.read(fname, BRIDGE_FORMAT_LIN, reflines,
        BRIDGE_REF_ONLY_PARTIAL))
      return false;

    if (reflines.skip())
      return true;
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
    cout << "Came from " << fname << "\n\n";
    return false;
  }

  try
  {
    Sheet::parse(buffer, headerOrig, handsOrig);
    Sheet::parseRefs();

    if (! buffer.fix(fname, reflines, BRIDGE_REF_ONLY_NONPARTIAL))
      return true; // No ref file

    buffer.rewind();
    Sheet::parse(buffer, headerFixed, handsFixed);
    hasFixed = true;
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
    cout << "Came from " << fname << "\n\n";
    return false;
  }

  sort(headerOrig.links.begin(), headerOrig.links.end());
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


string Sheet::strLinks() const
{
  stringstream ss;
  ss << "Links:\n";
  for (unsigned i = 0; i < headerOrig.links.size(); i++)
  {
    if (i == 0 || headerOrig.links[i] != headerOrig.links[i-1])
      ss << headerOrig.links[i] << "\n";
  }
  ss << "\n";

  return ss.str();
}


string Sheet::strHeader() const
{
  stringstream ss;
  ss << headerOrig.headline << "\n\n";

  if (headerOrig.links.size() > 0)
    ss << strLinks();
  
  return ss.str();
}


string Sheet::strPlays() const
{
  stringstream ss;
  ss << Sheet::strHeader();

  ss << setw(6) << "Board " <<
    setw(6) << "#tr" <<
    setw(6) << "#good" <<
    setw(6) << "#play" <<
    setw(6) << "#bad" <<
    setw(6) << "Grade" << "\n";

  SheetPlayDistance cp;
  cp.numTricks = 0;
  cp.goodTricks = 0;
  cp.numCards = 0;
  cp.distance = 0;
  vector<unsigned> cumVal(SHEET_PLAY_SIZE);

  for (auto &ho: handsOrig)
  {
    const SheetPlayDistance& sp = ho.hand.getPlayDistance();
    const SheetPlayType pv = ho.hand.playValidity();

    ss << setw(5) << ho.label << " " << 
      setw(6) << sp.numTricks <<
      setw(6) << sp.goodTricks <<
      setw(6) << sp.numCards <<
      setw(6) << sp.distance <<
      setw(6) << SheetPlayNames[pv] << "\n";

    cp.numTricks += sp.numTricks;
    cp.goodTricks += sp.goodTricks;
    cp.numCards += sp.numCards;
    cp.distance += sp.distance;

    cumVal[pv]++;
  }

  const string dashes(36, '-');
  ss << dashes << "\n";

  SheetPlayType pv;
  if (cumVal[SHEET_PLAY_OK] == 0 && cumVal[SHEET_PLAY_BAD] > 0)
    pv = SHEET_PLAY_BAD;
  else if (cumVal[SHEET_PLAY_OK] > 0 && cumVal[SHEET_PLAY_BAD] == 0)
    pv = SHEET_PLAY_OK;
  else if (cumVal[SHEET_PLAY_OK] > 5 * cumVal[SHEET_PLAY_BAD])
    pv = SHEET_PLAY_OK;
  else
    pv = SHEET_PLAY_OPEN;

  ss << setw(6) << "Sum" <<
    setw(6) << cp.numTricks <<
    setw(6) << cp.goodTricks <<
    setw(6) << cp.numCards <<
    setw(6) << cp.distance <<
    setw(6) << SheetPlayNames[pv] << "\n";

  if (pv == SHEET_PLAY_BAD)
    ss << "\nSuggest skip\n";

  ss << "\n";

  return ss.str();
}


string Sheet::str() const
{
  stringstream ss;

  bool hasFlaw = false;
  for (auto &ho: handsOrig)
  {
    if (ho.hand.playIsFlawed())
    {
      hasFlaw = true;
      break;
    }
  }
  if (hasFlaw)
    ss << Sheet::strPlays();

  if (! hasFixed)
    return ss.str();


  bool hasDiffs = false;
  if (! hasFlaw)
    ss << Sheet::strHeader();

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

    if (hasDiff || ho.hand.auctionIsFlawed() || ho.hand.playIsFlawed())
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

      // For mb errors, could check that auctionIsFlawed().

      notes << "\nActive ref lines: " << ho.label << "\n";
      for (auto &line: ho.refSource)
        notes << line << "\n";

      notes << "\n----------\n\n";
    }
  }

  if (hasDiffs)
    return ss.str() + "\n" + notes.str();
  else
    return "";
}

