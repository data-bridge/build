/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <regex>
#include <algorithm>

#include "parse.h"
#include "validatePBN.h"


bool validatePBN(
  ValState& valState,
  ValProfile& prof)
{
  if (valState.dataOut.line == "*")
  {
    // Could be the Pavlicek bug where play is shortened.
    while (valState.dataRef.line != "*" && 
        valState.bufferRef.next(valState.dataRef))
    {
    }

    prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
    return true;
  }

  const unsigned lo = valState.dataOut.len;
  const unsigned lr = valState.dataRef.len;

  if (lo == 11 && lr == 11 &&
      valState.dataOut.type != BRIDGE_BUFFER_STRUCTURED &&
      valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED)
  {
    // Could be a short play line, "S4 -- -- --" (Pavlicek notation!).
    unsigned poso = static_cast<unsigned>
      (valState.dataOut.line.find('-'));
    
    if (poso > 0 && poso < lo)
    {
      if (valState.dataRef.line.substr(0, poso) == 
          valState.dataOut.line.substr(0, poso))
      {
        prof.log(BRIDGE_VAL_PLAY_SHORT, valState);
        return true;
      }
      else
        return false;
    }
  }

  if (valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED ||
      valState.dataOut.type != BRIDGE_BUFFER_STRUCTURED)
    return false;

  while (1)
  {
    if (valState.dataRef.label == valState.dataOut.label)
    {
      if (valState.dataOut.value == valState.dataRef.value)
        return true;

      if ((valState.dataRef.label == "West" || 
           valState.dataRef.label == "North" ||
           valState.dataRef.label == "East" || 
           valState.dataRef.label == "South") &&
           refContainsOutValue(valState))
      {
        prof.log(BRIDGE_VAL_NAMES_SHORT, valState);
        return true;
      }
      else if (valState.dataRef.label == "Site" &&
        refContainsOutValue(valState))
      {
        prof.log(BRIDGE_VAL_LOCATION, valState);
        return true;
      }
      else if (valState.dataRef.label == "Stage" &&
        refContainsOutValue(valState))
      {
        prof.log(BRIDGE_VAL_SESSION, valState);
        return true;
      }
      else
        return false;
    }

    ValError ve;
    if (valState.dataRef.label == "Event")
      ve = BRIDGE_VAL_EVENT;
    else if (valState.dataRef.label == "Date")
      ve = BRIDGE_VAL_DATE;
    else if (valState.dataRef.label == "Description")
      ve = BRIDGE_VAL_TITLE;
    else if (valState.dataRef.label == "Stage")
      ve = BRIDGE_VAL_SESSION;
    else if (valState.dataRef.label == "HomeTeam")
      ve = BRIDGE_VAL_TEAMS;
    else if (valState.dataRef.label == "VisitTeam")
      ve = BRIDGE_VAL_TEAMS;
    else
      break;

    if (! valState.bufferRef.next(valState.dataRef) ||
        valState.dataRef.type != BRIDGE_BUFFER_STRUCTURED)
      return false;

    prof.log(ve, valState);
  }

  return false;
}


bool titleInWrongPlace(
  const Chunk& chunkRef,
  const Chunk& chunkOut)
{
  if (chunkRef.isEmpty(BRIDGE_FORMAT_TITLE) &&
      chunkRef.isSet(BRIDGE_FORMAT_EVENT) &&
      chunkOut.get(BRIDGE_FORMAT_TITLE) == chunkRef.get(BRIDGE_FORMAT_EVENT) &&
      chunkOut.get(BRIDGE_FORMAT_EVENT) == "Bridge Base Online")
    return true;
  else
    return false;
}


bool datePunctuation(
  const string& dateRef,
  const string& dateOut)
{
  // Coming e.g. from LIN, date will be missing.
  if (dateOut == "")
    return true;

  if (dateRef.length() != dateOut.length())
    return false;

  string st = dateRef;
  replace(st.begin(), st.end(), '-', '.');
  if (dateOut == st)
    return true;

  // In JEC.PBN there is sometimes a rotation, 02.01.2013 vs.
  // 2013.02.01.

  if (dateRef.length() == 10 &&
      dateRef.substr(0, 5) == dateOut.substr(5) &&
      dateRef.substr(6) == dateOut.substr(0, 4) &&
      dateRef.at(5) == '.' &&
      dateOut.at(4) == '.')
    return true;
  else
    return false;
}


bool locationNone(
  const string& locRef,
  const string& locOut)
{
  if ((locRef == "?" || locRef == "BBO") && locOut == "")
    return true;
  else
    return false;
}


bool scoringName(
  const string& scoreRef,
  const string& scoreOut)
{
  if ((scoreRef == "P" || scoreRef == "MP") && 
      scoreOut == "Matchpoints")
    return true;
  else
    return false;
}


bool roomRefMissing(
  const string& roomRef,
  const string& roomOut)
{
  // Some Double Dummy Captain files have no room at all.
  if (roomRef == "" && roomOut == "Open")
    return true;
  else
    return false;
}


bool dealRotated(
  const string& dealRef,
  const string& dealOut)
{
  if (dealRef.length() != 69 || dealOut.length() != 69)
    return false;

  Player dealerRef, dealerOut;
  if (! char2player(dealRef.at(0), dealerRef))
    return false;
  if (! char2player(dealOut.at(0), dealerOut))
    return false;

  unsigned delta = (static_cast<unsigned>(dealerRef+4) -
    static_cast<unsigned>(dealerOut)) % 4;

  for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
  {
    const string hr = dealRef.substr(2+17*i, 16);
    const unsigned j = (i + delta) % 4;
    const string ho = dealOut.substr(2+17*j, 16);
    if (hr != ho)
      return false;
  }
  return true;
}


bool vulCapitalization(
  const string& vulRef,
  const string& vulOut)
{
  if ((vulRef == "BOTH" || vulRef == "Both") && vulOut == "All")
    return true;
  else if (vulRef == "NONE" && vulOut == "None")
    return true;
  else
    return false;
}


void dropSpaces(string& st)
{
  st.erase(remove(st.begin(), st.end(), ' '), st.end());
}


void singleDashes(string& st)
{
  regex re("--");
  smatch match;
  st = regex_replace(st, re, "-");

  // Drop trailing dashes.
  size_t p = st.length()-1;
  while (p >= 1 && (st.at(p) == '-' || st.at(p) == ' '))
    p--;

  st = st.substr(0, p+1);
}


bool auctionFormat(
  const string& auctionRef,
  const string& auctionOut)
{
  vector<string> linesRef, linesOut;
  str2lines(auctionRef, linesRef);
  str2lines(auctionOut, linesOut);

  const unsigned lRef = linesRef.size();
  const unsigned lOut = linesOut.size();

  if (lRef == 0 || lOut == 0)
    return false;

  // Dealer must be the same.
  if (linesRef[0] != linesOut[0])
    return false;

  // Notes must be identical.
  unsigned noteIndexRef = 1;
  while (noteIndexRef < lRef && linesRef[noteIndexRef].at(0) != '[')
    noteIndexRef++;

  unsigned noteIndexOut = 1;
  while (noteIndexOut < lOut && linesOut[noteIndexOut].at(0) != '[')
    noteIndexOut++;

  if (lRef - noteIndexRef != lOut - noteIndexOut)
    return false;

  for (unsigned i = noteIndexRef, j = noteIndexOut; i < lRef; i++, j++)
  {
    if (linesRef[i] != linesOut[j])
      return false;
  }

  // Bidding must be largely the same: Ignore spaces, and turn
  // trailing passes into AP (or the other way round).

  if (noteIndexRef == 1 || noteIndexOut == 1)
    return false;

  if (noteIndexRef == 2)
  {
    // May be all passes.
    string line = linesRef[1];
    dropSpaces(line);
    if (line == "PassPassPassPass" && linesOut[1] == "AP")
      return true;
  }

  string refCum = "";
  for (unsigned i = 1; i < noteIndexRef; i++)
  {
    string line = linesRef[i];
    dropSpaces(line);
    refCum += line;
  }

  string outCum = "";
  for (unsigned i = 1; i < noteIndexOut; i++)
  {
    string line = linesOut[i];
    if (i == noteIndexOut-1)
    {
      // Turn AP into 3 passes.
      if (line.substr(line.length()-2) == "AP")
      {
        line = line.substr(0, line.length()-2) + "PassPassPass";
      }
    }
    dropSpaces(line);
    outCum += line;
  }

  if (refCum == outCum)
    return true;
  else
    return false;
}


bool declarerPassout(
  const Chunk& chunkRef,
  const Chunk& chunkOut)
{
  if (chunkRef.isSet(BRIDGE_FORMAT_DECLARER) &&
      chunkOut.isEmpty(BRIDGE_FORMAT_DECLARER) &&
      (chunkRef.get(BRIDGE_FORMAT_CONTRACT) == "P" ||
       chunkRef.get(BRIDGE_FORMAT_CONTRACT) == "Pass") &&
      chunkOut.get(BRIDGE_FORMAT_CONTRACT) == "Pass")
    return true;
  else
    return false;
}


bool contractPassout(
  const string& contractRef,
  const string& contractOut)
{
  if (contractRef == "P" && contractOut == "Pass")
    return true;
  else
    return false;
}


bool playFormat(
  const string& playRef,
  const string& playOut)
{
  vector<string> linesRef, linesOut;
  str2lines(playRef, linesRef);
  str2lines(playOut, linesOut);

  unsigned lRef = linesRef.size();
  unsigned lOut = linesOut.size();

  if (lRef == 0 || lOut == 0)
    return false;

  if (linesRef[lRef-1] == "*" || linesRef[lRef-1] == " *")
    lRef--;
  if (linesOut[lOut-1] == "*" || linesOut[lOut-1] == " *")
    lOut--;

  // Opening leader must be the same.
  if (linesRef[0] != linesOut[0])
    return false;

  string refCum = "";
  for (unsigned i = 1; i < lRef; i++)
  {
    string line = linesRef[i];
    if (i == lOut-1)
      singleDashes(line);
    dropSpaces(line);
    refCum += line;
  }

  string outCum = "";
  for (unsigned i = 1; i < lOut; i++)
  {
    string line = linesOut[i];
    if (i == lOut-1)
      singleDashes(line);
    dropSpaces(line);
    outCum += line;
  }

  if (refCum == outCum)
    return true;
  else
    return false;
}


bool resultPassout(
  const string& resultRef,
  const string& resultOut)
{
  if (resultRef == "0" && resultOut == "")
    return true;
  else
    return false;
}


bool doubleDummyFormat(
  const string& ddRef,
  const string& ddOut)
{
  vector<string> linesRef, linesOut;
  str2lines(ddRef, linesRef);
  str2lines(ddOut, linesOut);

  unsigned lRef = linesRef.size();
  unsigned lOut = linesOut.size();

  if (lRef != 21 || lOut != 21)
    return false;

  if (linesRef[0].substr(0, 8) != "Declarer" ||
      linesOut[0].substr(0, 8) != "Declarer")
    return false;

  for (unsigned i = 1; i < 21; i++)
  {
    dropSpaces(linesRef[i]);
    dropSpaces(linesOut[i]);
  }

  sort(linesRef.begin()+1, linesRef.end());
  sort(linesOut.begin()+1, linesOut.end());

  for (unsigned i = 1; i < 21; i++)
  {
    if (linesRef[i] != linesOut[i])
      return false;
  }
  return true;
}


bool scoreFormat(
  const string& scoreRef,
  const string& scoreOut)
{
  const unsigned lRef = scoreRef.length();
  const unsigned lOut = scoreOut.length();

  if ((scoreRef == "NS 0" || scoreRef == "0") && 
      scoreOut == "")
    return true;

  if (lRef < 5 || lOut < 4)
    return false;

  // "NS -110" is the same as "EW 110".
  if (scoreRef.substr(0, 4) == "NS -" &&
      scoreOut.substr(0, 3) == "EW " &&
      scoreRef.substr(4) == scoreOut.substr(3))
    return true;
  else
    return false;
}


bool str2IMPScore(
  const string& source,
  float& score)
{
  if (str2float(source, score))
    return true;

  if (source.length() < 4)
    return false;

  int side;
  if (source.substr(0, 3) == "NS ")
    side = 1;
  else if (source.substr(0, 3) == "EW ")
    side = -1;
  else
    return false;

  string rest = source.substr(3);
  size_t p = rest.find(",");
  if (p != string::npos)
    rest.at(p) = '.';

  if (str2float(rest, score))
  {
    score *= side;
    return true;
  }
  else
    return false;
}


bool scoreIMPFormat(
  const string& scoreRef,
  const string& scoreOut)
{
  if (scoreRef == "" || scoreOut == "")
    return false;

  float valRef, valOut;
  if (! str2IMPScore(scoreRef, valRef))
    return false;
  if (! str2IMPScore(scoreOut, valOut))
    return false;

  if (valRef == valOut)
    return true;

  const float delta = valRef - valOut;
  if (delta < 0.001 && delta > -0.001)
    return true;
  else
    return false;
}


bool validatePBNChunk(
  const Chunk& chunkRef,
  const Chunk& chunkOut,
  ValState& valState,
  ValProfile& prof)
{
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
  {
    if (chunkRef.get(i) == chunkOut.get(i))
      continue;
    
    if ((i == BRIDGE_FORMAT_TITLE || i == BRIDGE_FORMAT_EVENT) && 
        titleInWrongPlace(chunkRef, chunkOut))
    {
      prof.log(BRIDGE_VAL_TITLE, valState);
    }
    else if (i == BRIDGE_FORMAT_DATE && 
        datePunctuation(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_DATE, valState);
    }
    else if (i == BRIDGE_FORMAT_LOCATION && 
        locationNone(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_LOCATION, valState);
    }
    else if (i == BRIDGE_FORMAT_SCORING &&
        scoringName(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_SCORING, valState);
    }
    else if (i == BRIDGE_FORMAT_ROOM &&
        roomRefMissing(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_ROOM, valState);
    }
    else if (i == BRIDGE_FORMAT_DEAL &&
        dealRotated(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_DEAL, valState);
    }
    else if (i == BRIDGE_FORMAT_VULNERABLE &&
        vulCapitalization(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_VUL, valState);
    }
    else if (i == BRIDGE_FORMAT_AUCTION &&
        auctionFormat(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else if (i == BRIDGE_FORMAT_DECLARER &&
        declarerPassout(chunkRef, chunkOut))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else if (i == BRIDGE_FORMAT_CONTRACT &&
        contractPassout(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else if (i == BRIDGE_FORMAT_PLAY &&
        playFormat(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else if (i == BRIDGE_FORMAT_RESULT &&
        resultPassout(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else if (i == BRIDGE_FORMAT_SCORE &&
        scoreFormat(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else if (i == BRIDGE_FORMAT_SCORE_IMP &&
        scoreIMPFormat(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else if (i == BRIDGE_FORMAT_DOUBLE_DUMMY &&
        doubleDummyFormat(chunkRef.get(i), chunkOut.get(i)))
    {
      prof.log(BRIDGE_VAL_DD, valState);
    }
    else
    {
      cout << LABEL_NAMES[i] << ":\n";
      cout << "Ref " << chunkRef.get(i) << "\n";
      cout << "Out " << chunkOut.get(i) << "\n" << endl;
      prof.log(BRIDGE_VAL_ERROR, valState);
      return true;
    }
  }
  return true;
}

