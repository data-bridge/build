/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iomanip>
#include <sstream>

#include "heurfix.h"
#include "ddsIF.h"

#include "parse.h"
#include "Bexcept.h"


static void fixTricksMC(
  const string& text,
  const string& tricks,
  string& fixed)
{
  size_t tp0 = text.find("mc|");
  if (tp0 == string::npos || tp0+3 >= text.length())
    return;

  size_t tp1 = text.find("|", tp0+3);
  if (tp1 == string::npos)
    return;

  fixed = text;
  fixed.erase(tp0+3, tp1 - (tp0+3));
  fixed.insert(tp0+3, tricks);
}


static void fixTricksContract(
  string& contractHeader, 
  const unsigned tricks)
{
  const unsigned l = contractHeader.length();
  if (l < 4)
    THROW("Contract too short: " + contractHeader);

  const char c = contractHeader.at(0);
  if (c < '1' || c > '7')
    THROW("Malformed contract: " + contractHeader);

  const unsigned tricksNeeded = 7u + static_cast<unsigned>(c - '1');

  if (contractHeader.at(l-1) == '=')
    contractHeader.pop_back();
  else if (contractHeader.at(l-2) == '+' || contractHeader.at(l-2) == '-')
  {
    contractHeader.pop_back();
    contractHeader.pop_back();
  }
  else
    THROW("Malformed contract: " + contractHeader);

  if (tricks == tricksNeeded)
    contractHeader += "=";
  else if (tricks > tricksNeeded)
    contractHeader += "+" + STR(tricks - tricksNeeded);
  else
    contractHeader += "-" + STR(tricksNeeded - tricks);
}


static string fixTricksRS(
  Segment * segment, 
  const unsigned tricks)
{
  string contractHeader = segment->contractFromHeader();
  fixTricksContract(contractHeader, tricks);

  if (! segment->setContractInHeader(contractHeader))
    THROW("Could not rewrite header contract");

  return "rs|" + segment->strContracts(BRIDGE_FORMAT_PAR) + + "|";
}


static void adjustContractDeclarer(
  string& contractHeader, 
  const string& declarer)
{
  const unsigned l = contractHeader.length();
  if (l < 4)
    THROW("Contract too short: " + contractHeader);

  if (declarer.length() != 1)
    THROW("Declarer length not 1: " + declarer);

  contractHeader.replace(2, 1, declarer);
}


static void storeAuctionContractDeclWins(
  const Group& group, 
  Segment * segment, 
  const unsigned lineno,
  const string& declarer)
{
  string fname = changeExt(group.name(), ".ref");
  if (fname == "")
  {
    cout << "Wanted to write rs fix file " << fname << " with " << 
      declarer << " as declarer\n";
    return;
  }

  string contractHeader = segment->contractFromHeader();
  adjustContractDeclarer(contractHeader, declarer);

  if (! segment->setContractInHeader(contractHeader))
    THROW("Could not rewrite header contract");

  const string resHeader = "rs|" +
    segment->strContracts(BRIDGE_FORMAT_PAR) + + "|";

  // Actual tricks win if the hand is played out completely.
  appendFile(fname, lineno, "replace", resHeader);
}


static void adjustContractDenom(
  string& contractHeader, 
  const string& denom)
{
  const unsigned l = contractHeader.length();
  if (l < 4)
    THROW("Contract too short: " + contractHeader);

  if (denom.length() != 1)
    THROW("Denomination length not 1: " + denom);

  contractHeader.replace(1, 1, denom);
}


static void storeAuctionContractDenomWins(
  const Group& group, 
  Segment * segment, 
  const unsigned lineno,
  const string& denom)
{
  string fname = changeExt(group.name(), ".ref");
  if (fname == "")
  {
    cout << "Wanted to write rs fix file " << fname << " with " << 
      denom << " as denomination\n";
    return;
  }

  string contractHeader = segment->contractFromHeader();
  adjustContractDenom(contractHeader, denom);

  // TODO: Common among all adjust functions.

  if (! segment->setContractInHeader(contractHeader))
    THROW("Could not rewrite header contract");

  const string resHeader = "rs|" +
    segment->strContracts(BRIDGE_FORMAT_PAR) + + "|";

  // Actual tricks win if the hand is played out completely.
  appendFile(fname, lineno, "replace", resHeader);
}


void heurFixTricks(
  Group& group,
  Segment * segment,
  Board * board,
  const Buffer& buffer,
  const vector<string>& chunk,
  const Counts& counts,
  const Options& options)
{
  cout << board->strDeal(BRIDGE_FORMAT_TXT) << endl;
  cout << board->strContract(BRIDGE_FORMAT_TXT) << endl;
  cout << board->strPlay(BRIDGE_FORMAT_TXT) << endl;

  const string headerRes = board->strResult(BRIDGE_FORMAT_PAR, false);

  RunningDD runningDD;
  board->getStateDDS(runningDD);

  string nameRef = changeExt(group.name(), ".ref");
  if (nameRef == "")
  {
    cout << "XX0 Couldn't make a ref name: " << group.name() << endl;
    return;
  }

  if (board->playIsOver())
  {
    string fixed = fixTricksRS(segment, runningDD.tricksDecl);
    unsigned rsNo = buffer.firstRS();
    if (options.refLevel != REF_LEVEL_NONE)
    {
      // Actual tricks win if the hand is played out completely.
      appendFile(nameRef, rsNo, "replace", fixed);
      cout << "Wrote play result with " << runningDD.tricksDecl <<
        " tricks\n";
    }
    else
    {
      cout << "XX1 Wanted to write rs with " << runningDD.tricksDecl <<
        " tricks\n";
      cout << rsNo << " replace \"" << fixed << "\"\n";
    }
  }
  else
  {
    unsigned ddRes, hdrRes, chunkRes;
    try
    {
      ddRes = tricksDD(runningDD);
    }
    catch (Bexcept bex)
    {
      UNUSED(bex);
    }

    if (! str2unsigned(headerRes, hdrRes) || 
        ! str2unsigned(chunk[BRIDGE_FORMAT_RESULT], chunkRes))
    {
      cout << "Bad trick strings: " << headerRes << ", " <<
        chunk[BRIDGE_FORMAT_RESULT] << endl;
    }
    else if (hdrRes == ddRes)
    {
      // Header wins if it agrees with double-dummy.
      unsigned lineno = counts.lno[BRIDGE_FORMAT_RESULT];
      string fixed;
      fixTricksMC(buffer.getLine(lineno), headerRes, fixed);
      if (options.refLevel != REF_LEVEL_NONE)
      {
        appendFile(nameRef, lineno, "replace", fixed);
        cout << "Wrote mc with " << headerRes << " tricks\n";
      }
      else
      {
        cout << "XX2 Wanted to write mc with " << headerRes << " tricks\n";
        cout << lineno << " replace \"" << fixed << "\"\n";
      }
    }
    else if (chunkRes == ddRes)
    {
      // Play wins if it agrees with double-dummy.
      string fixed = fixTricksRS(segment, ddRes);
      unsigned rsNo = buffer.firstRS();
      if (options.refLevel != REF_LEVEL_NONE)
      {
        appendFile(nameRef, rsNo, "replace", fixed);
        cout << "Wrote play result with " << ddRes << " tricks\n";
      }
      else
      {
        cout << "XX3 Wanted to write rs with " << ddRes << " tricks\n";
        cout << rsNo << " replace \"" << fixed << "\"\n";
      }
    }
    else
    {
      // Hodge-podge.

      cout << "Header " << setw(2) << right << headerRes << 
        " mc " << setw(2) << right << chunk[BRIDGE_FORMAT_RESULT] << 
        " vs. DD " << setw(2) << right << ddRes << 
        " (min " << setw(2) << right << runningDD.tricksDecl << 
        ", max " << setw(2) << right << 13-runningDD.tricksDef << 
        "), chunk " << counts.chunkno-1 << endl;

      if (hdrRes > 13 - runningDD.tricksDef)
        cout << "Header tricks unreachable" << endl;

      if (chunkRes > 13 - runningDD.tricksDef)
        cout << "Play tricks unreachable" << endl;

      const unsigned distHdr = 
        (ddRes >= hdrRes ? ddRes-hdrRes : hdrRes-ddRes);
      const unsigned distPlay = 
        (ddRes >= chunkRes ? ddRes-chunkRes : chunkRes-ddRes);

      if (distHdr > distPlay)
      {
        cout << "Play result is closer to DD\n";
        string fixed = fixTricksRS(segment, ddRes);
        unsigned rsNo = buffer.firstRS();
        if (options.refLevel == REF_LEVEL_ALL)
        {
          appendFile(nameRef, rsNo, "replace", fixed);
          cout << "Wrote play result with " << ddRes << " tricks\n";
        }
        else
        {
          cout << "XX4 Wanted to write rs with " << ddRes << " tricks\n";
          cout << rsNo << " replace \"" << fixed << "\"\n";
        }
      }
      else if (distHdr < distPlay)
      {
        cout << "Header result is closer to DD\n";
        string fixed;
        unsigned lineno = counts.lno[BRIDGE_FORMAT_RESULT];
        if (options.refLevel == REF_LEVEL_ALL)
        {
          fixTricksMC(buffer.getLine(lineno), headerRes, fixed);
          appendFile(nameRef, lineno, "replace", fixed);
          cout << "Wrote mc with " << headerRes << " tricks\n";
        }
        else
        {
          cout << "Wanted to write mc with " << headerRes << " tricks\n";
          cout << lineno << " replace \"" << fixed << "\"\n";
        }
      }
      else
        cout << "DD has equal distance\n";

      cout << board->strDealRemain(BRIDGE_FORMAT_TXT) << endl;
    }
  }
}


void heurFixPlayDD(
  Group& group,
  Segment * segment,
  Board * board,
  const Buffer& buffer,
  const vector<string>& chunk,
  const Options& options)
{
  cout << board->strDeal(BRIDGE_FORMAT_TXT) << endl;
  cout << board->strContract(BRIDGE_FORMAT_TXT) << endl;
  cout << segment->contractFromHeader() << endl;

  const string declHeader = board->strDeclarer(BRIDGE_FORMAT_PAR);
  const string denomHeader = board->strDenom(BRIDGE_FORMAT_PAR);

  const string declAuction = board->strDeclarerPlay(BRIDGE_FORMAT_PAR);
  const string denomAuction = board->strDenomPlay(BRIDGE_FORMAT_PAR);
  string declLead = "";
  if (chunk[BRIDGE_FORMAT_PLAY].length() >= 2)
  {
    string lead = chunk[BRIDGE_FORMAT_PLAY].substr(0, 2);
    toUpper(lead);
    const Player leader = board->holdsCard(lead);
    if (leader != BRIDGE_PLAYER_SIZE)
    {
      const Player pLead = static_cast<Player>((leader + 3) % 4);
      declLead = PLAYER_NAMES_SHORT[pLead];
    }
  }

  cout << "Hdr  " << declHeader << " " << denomHeader << endl;
  cout << "Auct " << declAuction << " " << denomAuction << endl;
  if (declLead == "")
    cout << "Lead none\n";
  else
    cout << "Lead " << declLead << "\n";

  if (declAuction == declHeader)
  {
    if (declAuction == declLead)
    {
      if (options.refLevel != REF_LEVEL_NONE)
      {
        unsigned rsNo = buffer.firstRS();
        storeAuctionContractDenomWins(group, segment, 
          rsNo, denomAuction);
        cout << "E1 Fixed denom typo in header\n";
      }
      else
      {
        cout << "E1 Wanted to fix denom typo in header\n";
      }
    }
    else if (declLead == "")
      cout << "E2 Would like to fix denom typo, probably in header\n";
    else
      cout << "E3 Would like to fix denom typo, but odd\n";
  }
  else if (denomAuction == denomHeader)
  {
    if ((declHeader == "N" && declAuction == "S") ||
        (declHeader == "E" && declAuction == "W") ||
        (declHeader == "S" && declAuction == "N") ||
        (declHeader == "W" && declAuction == "E"))
    {
      if (declLead == declAuction)
      {
        if (options.refLevel != REF_LEVEL_NONE)
        {
          unsigned rsNo = buffer.firstRS();
          storeAuctionContractDeclWins(group, segment, 
            rsNo, declAuction);
          cout << "E4 Fixed decl typo in header\n";
        }
        else
          cout << "E4 Wanted to fix decl typo in header\n";
      }
      else if (declLead == "")
        cout << "E5 Would like to fix decl typo, probably in header\n";
      else
        cout << "E6 Would like to fix decl typo, auction very wrong?\n";
    }
    else if (declLead == declHeader)
      cout << "E7 Would like to fix decl typo, likely in auction\n";
    else if (declLead == declAuction)
    {
      if (options.refLevel != REF_LEVEL_NONE)
      {
        unsigned rsNo = buffer.firstRS();
        storeAuctionContractDeclWins(group, segment, 
          rsNo, declAuction);
        cout << "E8 Fixed declarer typo in header\n";
      }
      else
        cout << "E8 Wanted to fix declarer typo in header\n";
    }
    else if (declLead == "")
      cout << "E9 Would like to fix decl typo, probably in auction\n";
    else
      cout << "EA Would like to fix decl typo, but odd auction\n";
  }
  else if (declLead == declHeader)
    cout << "EB Would like to fix denom typo, likely in auction\n";
  else if (declLead == declAuction)
    cout << "EC Would like to fix denom typo, likely in header\n";
  else
    cout << "ED Not sure what is going on\n";
}

