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


static void writeTricksMC(
  const Buffer& buffer,
  const Counts& counts,
  const string& fname,
  const string& tricks,
  const RefLevel refLevel)
{
  unsigned lineno = counts.lno[BRIDGE_FORMAT_RESULT];
  string fixed;
  fixTricksMC(buffer.getLine(lineno), tricks, fixed);
  if (refLevel != REF_LEVEL_NONE)
  {
    appendFile(fname, lineno, "replace", fixed);
    cout << "Wrote mc with " << tricks << " tricks\n";
  }
  else
  {
    cout << "XX2 Wanted to write mc with " << tricks << " tricks\n";
    cout << lineno << " replace \"" << fixed << "\"\n";
  }
}


static void writeTricksRS(
  Segment * segment,
  const Buffer& buffer,
  const string& fname,
  const unsigned tricks,
  const RefLevel refLevel)
{
  string fixed = fixTricksRS(segment, tricks);
  unsigned rsNo = buffer.firstRS();
  if (refLevel != REF_LEVEL_NONE)
  {
    appendFile(fname, rsNo, "replace", fixed);
    cout << "Wrote rs with " << tricks << " tricks\n";
  }
  else
  {
    cout << "XX1 Wanted to write rs with " << tricks << " tricks\n";
    cout << rsNo << " replace \"" << fixed << "\"\n";
  }
}


static void fixDeclarerContract(
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


static void fixDenomContract(
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


static string fixDeclarerRS(
  Segment * segment, 
  const string& declarer)
{
  string contractHeader = segment->contractFromHeader();
  fixDeclarerContract(contractHeader, declarer);

  if (! segment->setContractInHeader(contractHeader))
    THROW("Could not rewrite header contract");

  return "rs|" + segment->strContracts(BRIDGE_FORMAT_PAR) + + "|";
}


static string fixDenomRS(
  Segment * segment, 
  const string& denom)
{
  string contractHeader = segment->contractFromHeader();
  fixDenomContract(contractHeader, denom);

  if (! segment->setContractInHeader(contractHeader))
    THROW("Could not rewrite header contract");

  return "rs|" + segment->strContracts(BRIDGE_FORMAT_PAR) + + "|";
}


static void writeDeclarerRS(
  Segment * segment,
  const Buffer& buffer,
  const string& fname,
  const string& declarer,
  const RefLevel refLevel)
{
  const string fixed = fixDeclarerRS(segment, declarer);
  unsigned rsNo = buffer.firstRS();
  if (refLevel != REF_LEVEL_NONE)
  {
    appendFile(fname, rsNo, "replace", fixed);
    cout << "Wrote rs with " << declarer << " as declarer\n";
  }
  else
  {
    cout << "Wanted to write rs with " << declarer << " as declarer\n";
    cout << rsNo << " replace \"" << fixed << "\"\n";
  }
}


static void writeDenomRS(
  Segment * segment,
  const Buffer& buffer,
  const string& fname,
  const string& denom,
  const RefLevel refLevel)
{
  const string fixed = fixDenomRS(segment, denom);
  unsigned rsNo = buffer.firstRS();
  if (refLevel != REF_LEVEL_NONE)
  {
    appendFile(fname, rsNo, "replace", fixed);
    cout << "Wrote rs with " << denom << " as denom\n";
  }
  else
  {
    cout << "Wanted to write rs with " << denom << " as denom\n";
    cout << rsNo << " replace \"" << fixed << "\"\n";
  }
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
    // Play wins if it is complete.
    writeTricksRS(segment, buffer, nameRef, runningDD.tricksDecl,
      options.refLevel);
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
      writeTricksMC(buffer, counts, nameRef, headerRes, options.refLevel);
    }
    else if (chunkRes == ddRes)
    {
      // Play wins if it agrees with double-dummy.
      writeTricksRS(segment, buffer, nameRef, ddRes, options.refLevel);
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
        // Play wins if it is closer to double-dummy.
        cout << "XX0 Play result is closer to DD\n";
        // writeTricksRS(segment, buffer, nameRef, ddRes, options.refLevel);
      }
      else if (distHdr < distPlay)
      {
        // Header wins if it is closer to double-dummy.
        cout << "XX1 Header result is closer to DD\n";
        // writeTricksMC(buffer, counts, nameRef, headerRes, options.refLevel);
      }
      else
        cout << "XX2 DD has equal distance\n";

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

  string nameRef = changeExt(group.name(), ".ref");
  if (nameRef == "")
  {
    cout << "YY0 Couldn't make a ref name: " << group.name() << endl;
    return;
  }

  if (declAuction == declHeader)
  {
    if (declAuction == declLead)
    {
      writeDenomRS(segment, buffer, nameRef, denomAuction,
        options.refLevel);
    }
    else if (declLead == "")
      cout << "YY2 Would like to fix denom, probably in header\n";
    else
      cout << "YY3 Would like to fix denom, but odd\n";
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
        // If lead and auction agree, we fix the header.
        writeDeclarerRS(segment, buffer, nameRef, declAuction,
          options.refLevel);
      }
      else if (declLead == "")
        cout << "YY5 Would like to fix declarer, probably in header\n";
      else
        cout << "YY6 Would like to fix declarer, auction very wrong?\n";
    }
    else if (declLead == declHeader)
      cout << "YY7 Would like to fix decl typo, likely in auction\n";
    else if (declLead == declAuction)
    {
      // If lead and auction agree, we fix the header.
      writeDeclarerRS(segment, buffer, nameRef, declAuction,
        options.refLevel);
    }
    else if (declLead == "")
      cout << "YY9 Would like to fix declarer, probably in auction\n";
    else
      cout << "YYA Would like to fix declarer, but odd auction\n";
  }
  else if (declLead == declHeader)
    cout << "YYB Would like to fix denom, likely in auction\n";
  else if (declLead == declAuction)
    cout << "YYC Would like to fix denom, likely in header\n";
  else
    cout << "YYD Not sure what is going on\n";
}

