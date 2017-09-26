/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#pragma warning(pop)

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


void Sheet::resetHeader()
{
  header.headline = "";
  header.links.clear();

  header.linenoRS.clear();
  header.lineRS.clear();
  header.indexmin.clear();

  header.lineCount = BIGNUM;

  bmin = 0;
  bmax = BIGNUM;
}


void Sheet::resetHand(SheetHandData& hd)
{
  hd.label = "";
  hd.roomQX = "";
  hd.numberQX = BIGNUM;
  hd.linenoQX = BIGNUM;
  hd.linenoMC = BIGNUM;
  hd.lineMC = "";
  hd.claim = "";
  hd.hand.reset();
  hd.refSource.clear();
}


void Sheet::reset()
{
  Sheet::resetHeader();

  hands.clear();
  hands.reserve(SHEET_INIT);
}


Sheet::~Sheet()
{
}


void Sheet::parseVG(const string& value) 
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
    if (bmin == 0)
      bmin = u;
  }
  else
    bmin = BIGNUM;

  if (tokens[4] != "" && str2unsigned(tokens[4], u))
  {
    // In case of multiple segments, take the last one.
    bmax = u;
  }
  else
    bmax = BIGNUM;
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
  const vector<string>& clist)
{
  if (hd.label.length() < 2)
    return "";

  hd.roomQX = hd.label.substr(0, 1);
  if (hd.roomQX != "o" && hd.roomQX != "c")
    return "";

  unsigned adder = (hd.roomQX == "o" ? 0u : 1u);
  
  if (! str2unsigned(hd.label.substr(1), hd.numberQX))
    return "";

  if (hd.numberQX == 0 || 
      hd.numberQX < bmin || 
      hd.numberQX > bmax)
    return "";

  const unsigned index = 2*(hd.numberQX-bmin) + adder;
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


void Sheet::parse(Buffer& buffer)
{
  Segment segment;
  LineData lineData;
  SheetHandData handTmp;
  Sheet::resetHand(handTmp);

  header.lineCount = buffer.lengthOrig();

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

      Sheet::parseVG(lineData.value);
    }
    else if (lineData.label == "rs")
    {
      if (header.linenoRS.size() == 0)
      {
        header.linenoRS.push_back(lineData.no);
        header.lineRS.push_back(buffer.getLine(lineData.no));
        header.indexmin.push_back(0);
      }
      else
      {
        header.linenoRS.push_back(lineData.no);
        header.lineRS.push_back(buffer.getLine(lineData.no));
        header.indexmin.push_back(hands.size()+1); // Next index
      }
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
        string c = Sheet::qxToHeaderContract(handTmp, clist);
        handTmp.hand.finishHand(c, plays, numPlays);
        hands.push_back(handTmp);
        Sheet::resetHand(handTmp);
      }

      Sheet::resetHand(handTmp);
      auctionFlawed = false;
      plays = "";
      numPlays = 0;
      handTmp.label = Sheet::parseQX(lineData.value);
      handTmp.linenoQX = lineData.no;
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
      if (lineData.value.length() == 8 && numPlays % 4 == 0)
      {
        if (numPlays > 0)
          plays += ":";
        plays += lineData.value;
        numPlays += 4;
      }
      else
      {
        if (numPlays > 0 && numPlays % 4 == 0)
          plays += ":";
        plays += lineData.value;
        numPlays++;
      }
    }
    else if (lineData.label == "mc")
    {
      if (! handTmp.hand.claim(lineData.value))
        Sheet::fail("Bad claim");
      handTmp.linenoMC = lineData.no;
      handTmp.lineMC = buffer.getLine(lineData.no);
      handTmp.claim = lineData.value;
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
    string c = Sheet::qxToHeaderContract(handTmp, clist);
    handTmp.hand.finishHand(c, plays, numPlays);
    hands.push_back(handTmp);
  }
}


unsigned Sheet::refLineNoToHandNo(const unsigned lineNo) const
{
  const unsigned l = hands.size();
  if (lineNo < hands[0].linenoQX)
    return BIGNUM; // Header

  for (unsigned i = 0; i < l-1; i++)
  {
    if (lineNo < hands[i+1].linenoQX)
      return i;
  }

  if (lineNo >= hands[l-1].linenoQX)
    return l-1;
  else
    return BIGNUM;
}


void Sheet::parseRefs()
{
  // refLines contains a list of line entries from ref file.
  // For each hand, refSource contains a list of ref line numbers
  // affecting that hand.
  
  for (auto &rl: refLines)
  {
    if (rl.isCommented())
      continue;

    const unsigned handNoFirst = Sheet::refLineNoToHandNo(rl.lineno());
    const unsigned handNoLast = (rl.rangeCount() <= 1 ?  handNoFirst : 
      Sheet::refLineNoToHandNo(rl.lineno() + rl.rangeCount() - 1));

    if (handNoFirst == BIGNUM || handNoLast == BIGNUM)
      continue;

    for (unsigned hno = handNoFirst; hno <= handNoLast; hno++)
      hands[hno].refSource.push_back(rl.line());
  }
}


bool Sheet::read(
  const string& fname)
{
  Buffer buffer;

  try
  {
    if (! buffer.read(fname, BRIDGE_FORMAT_LIN, refLines))
      return false;

    if (refLines.skip())
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
    Sheet::parse(buffer);
    Sheet::parseRefs();
  }
  catch (Bexcept& bex)
  {
    bex.print(cout);
    cout << "Came from " << fname << "\n\n";
    return false;
  }

  sort(header.links.begin(), header.links.end());
  return true;
}


unsigned Sheet::findHandNo(const string& label) const
{
  for (unsigned i = 0; i < hands.size(); i++)
  {
    if (hands[i].label == label)
      return i;
  }
  return BIGNUM;
}


string Sheet::strHeader() const
{
  stringstream ss;
  ss << header.headline << "\n\n";

  if (header.links.size() > 0)
    ss << strLinks();
  
  return ss.str();
}


string Sheet::strLinks() const
{
  stringstream ss;
  ss << "Links:\n";
  for (unsigned i = 0; i < header.links.size(); i++)
  {
    if (i == 0 || header.links[i] != header.links[i-1])
      ss << header.links[i] << "\n";
  }
  ss << "\n";

  return ss.str();
}


string Sheet::strHand(
  const SheetHandData& ho,
  const unsigned index) const
{
  stringstream ss;

  ss << "Board " << ho.label << "\n";
  ss << ho.hand.strNotes();
  ss << ho.hand.strChat();

  if (index != BIGNUM)
  {
    const unsigned l = hands.size();
    if (index < l-1)
    {
      ss << "--\n";
      ss << hands[index+1].hand.strChat();
      if (index < l-2)
      {
        ss << "--\n";
        ss << hands[index+2].hand.strChat();
      }
    }
  }

  // For mb errors, could check that auctionIsFlawed().

  if (ho.refSource.size() > 0)
  {
    ss << "\nActive ref lines: " << ho.label << "\n";
    for (auto &line: ho.refSource)
      ss << line << "\n";
  }

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

  for (auto &ho: hands)
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


string Sheet::handRange(const unsigned index) const
{
  const unsigned e = (index == hands.size()-1 ? 
    header.lineCount : 
    hands[index+1].linenoQX-1);

  if (hands[index].linenoQX == e)
    return STR(hands[index].linenoQX);
  else
    return STR(hands[index].linenoQX) + "-" + STR(e);
}


unsigned Sheet::tagNo(
  const string& line,
  const string& tag) const
{
  const unsigned p = line.find(tag);
  if (p == string::npos)
    return 0;
  if (p == 0)
    return 1;

  const unsigned c = static_cast<unsigned>
    (count(line.begin(), line.begin()+static_cast<int>(p), '|'));
  if (c % 2)
    return 0;

  return c/2;
}


string Sheet::suggestAuction(const unsigned index) const
{
  stringstream ss;
  ss << "CHOOSE auction\n";
  ss << Sheet::handRange(index);
  ss << " delete {ERR_LIN_HAND_AUCTION_WRONG(0,1,1)}\n";
  ss << "AUCTION options: NONE, WRONG, LIVE, ABBR (or manually)\n\n";

  return ss.str();
}


string Sheet::suggestPlay(const unsigned index) const
{
  stringstream ss;
  ss << "CHOOSE play\n";
  ss << Sheet::handRange(index);
  ss << " delete {ERR_LIN_HAND_PLAY_WRONG(0,1,1)}\n";
  ss << "PLAY options: WRONG, MISSING (or manually) \n\n";
  return ss.str();
}


string Sheet::suggestTricks(
  const SheetHandData& ho,
  const unsigned index) const
{
  stringstream ss;
  ss << "CHOOSE tricks/contract\n";

  unsigned i = 0;
  if (header.lineRS.size() > 1)
  {
    while (i+1 < header.lineRS.size() && header.indexmin[i+1] < index)
      i++;
  }

  const unsigned rsno = Sheet::tagNo(header.lineRS[i], "rs");
  const unsigned fno = 2*(ho.numberQX-bmin) + 
    (ho.roomQX == "o" ? 0u : 1u) + 1;

  ss << header.linenoRS[i] << " replaceLIN \"" <<
    rsno << "," << fno << ",rs," <<
    ho.hand.strContractHeader() << "," <<
    ho.hand.strContractAuction() << "\" {";
  ss << ho.hand.strContractTag() << "(1,1,1)}\n";

  const string tricksDiff = ho.hand.tricksAlt();
  if (tricksDiff != "Y" && ho.linenoMC != BIGNUM)
  {
    // Play was completed, so offer mc.
    const unsigned mcno = Sheet::tagNo(ho.lineMC, "mc");
    if (mcno == 0)
      THROW("No mc in mc line: " + ho.lineMC);
    ss << ho.linenoMC << " replaceLIN \"" <<
      mcno << ",mc," << ho.claim << 
      "," << ho.hand.tricksAlt() << "\" {ERR_LIN_MC_REPLACE(1,1,1)}\n";
  }

  ss << Sheet::handRange(index);
  ss << " delete {ERR_LIN_HAND_AUCTION_WRONG(0,1,1)}\n";

  return ss.str();
}


string Sheet::str() const
{
  stringstream ss;

  bool contractsDiffer = false;
  bool hasPlayFlaw = false;
  bool hasAuctionFlaw = false;
  for (auto &ho: hands)
  {
    if (ho.hand.contractsOrTricksDiffer())
      contractsDiffer = true;
    if (ho.hand.auctionIsFlawed())
      hasAuctionFlaw = true;
    if (ho.hand.playIsFlawed())
      hasPlayFlaw = true;
  }
  if (hasPlayFlaw)
    ss << Sheet::strPlays();

  if (! contractsDiffer && ! hasAuctionFlaw && ! hasPlayFlaw)
    return ss.str();

  if (! hasPlayFlaw)
    ss << Sheet::strHeader();

  ss << setw(6) << "Board " <<
    setw(6) << "Chdr" <<
    setw(6) << "Thdr" <<
    setw(6) << "Cauct" <<
    setw(6) << "Tplay" <<
    setw(6) << "Tclm" << "\n";

  stringstream notes;

  for (auto &ho: hands)
  {
    ss << setw(5) << ho.label << " " << ho.hand.str() << "\n";

    if (ho.hand.auctionIsFlawed() || 
        ho.hand.playIsFlawed() ||
        ho.hand.contractsOrTricksDiffer())
    {
      // hasDiffs = true;
      const unsigned index = Sheet::findHandNo(ho.label);
      notes << Sheet::strHand(ho, index);

      if (ho.hand.auctionIsFlawed())
        notes << Sheet::suggestAuction(index);
      
      if (ho.hand.playIsFlawed())
        notes << Sheet::suggestPlay(index);

      if (ho.hand.contractsOrTricksDiffer())
        notes << suggestTricks(ho, index);

      notes << "\n----------\n\n";
    }
  }

  return ss.str() + "\n" + notes.str();
}

