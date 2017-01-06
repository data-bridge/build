/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


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


bool datePunctuation(
  const string& dateRef,
  const string& dateOut)
{
  if (dateRef.length() != dateOut.length())
    return false;

  string st = dateRef;
  replace(st.begin(), st.end(), '-', '.');
  return (dateOut == st);
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
  if (vulRef == "BOTH" && vulOut == "All")
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


bool playFormat(
  const string& playRef,
  const string& playOut)
{
  vector<string> linesRef, linesOut;
  str2lines(playRef, linesRef);
  str2lines(playOut, linesOut);

  const unsigned lRef = linesRef.size();
  const unsigned lOut = linesOut.size();

  if (lRef == 0 || lOut == 0)
    return false;

  // Opening leader must be the same.
  if (linesRef[0] != linesOut[0])
    return false;

  string refCum = "";
  for (unsigned i = 1; i < lRef; i++)
  {
    string line = linesRef[i];
    dropSpaces(line);
    refCum += line;
  }

  string outCum = "";
  for (unsigned i = 1; i < lOut; i++)
  {
    string line = linesOut[i];
    dropSpaces(line);
    outCum += line;
  }

  if (refCum == outCum)
    return true;
  else
    return false;
}


bool validatePBNChunk(
  const vector<string>& chunkRef,
  const vector<string>& chunkOut,
  ValState& valState,
  ValProfile& prof)
{
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
  {
    if (chunkRef[i] == chunkOut[i])
      continue;
    
    if (i == BRIDGE_FORMAT_DATE && 
        datePunctuation(chunkRef[i], chunkOut[i]))
    {
      prof.log(BRIDGE_VAL_DATE, valState);
    }
    else if (i == BRIDGE_FORMAT_SCORING &&
        scoringName(chunkRef[i], chunkOut[i]))
    {
      prof.log(BRIDGE_VAL_SCORING, valState);
    }
    else if (i == BRIDGE_FORMAT_ROOM &&
        roomRefMissing(chunkRef[i], chunkOut[i]))
    {
      prof.log(BRIDGE_VAL_ROOM, valState);
    }
    else if (i == BRIDGE_FORMAT_DEAL &&
        dealRotated(chunkRef[i], chunkOut[i]))
    {
      prof.log(BRIDGE_VAL_DEAL, valState);
    }
    else if (i == BRIDGE_FORMAT_VULNERABLE &&
        vulCapitalization(chunkRef[i], chunkOut[i]))
    {
      prof.log(BRIDGE_VAL_VUL, valState);
    }
    else if (i == BRIDGE_FORMAT_AUCTION &&
        auctionFormat(chunkRef[i], chunkOut[i]))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else if (i == BRIDGE_FORMAT_PLAY &&
        playFormat(chunkRef[i], chunkOut[i]))
    {
      prof.log(BRIDGE_VAL_AUCTION, valState);
    }
    else
    {
      cout << LABEL_NAMES[i] << ":\n";
      cout << "Ref " << chunkRef[i] << "\n";
      cout << "Out " << chunkOut[i] << "\n" << endl;
      prof.log(BRIDGE_VAL_ERROR, valState);
      return true;
    }
  }
  return true;
}

