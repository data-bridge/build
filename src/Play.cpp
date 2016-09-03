/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Play.h"
#include "Debug.h"
#include <map>
#include "parse.h"
#include "portab.h"

extern Debug debug;


#define PLAY_SEQ_INIT 28
#define PLAY_SEQ_INCR 24

#define PLAY_NUM_CARDS 52


const char PLAY_CARDS[BRIDGE_TRICKS] =
{
  '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'
};

const char PLAY_DENOMS[2 * BRIDGE_SUITS] =
{
  'S', 'H', 'D', 'C', 's', 'h', 'd', 'c'
};

struct cardInfoType
{
  unsigned no;
  unsigned bitValue;
  unsigned suit;
  unsigned rank;
};

string PLAY_NO_TO_CARD[PLAY_NUM_CARDS];

map<string, cardInfoType> PLAY_CARD_TO_INFO; // All syntaxes

cardInfoType PLAY_NO_TO_INFO[PLAY_NUM_CARDS];

unsigned TRICK_RANKS[BRIDGE_DENOMS][BRIDGE_SUITS][PLAY_NUM_CARDS];

bool setPlayTables = false;


Play::Play()
{
  Play::Reset();
  if (! setPlayTables)
  {
    setPlayTables = true;
    Play::SetTables();
  }
}


Play::~Play()
{
}


void Play::Reset()
{
  setDDFlag = false;

  setDealFlag = false;

  len = 0;
  lenMax = PLAY_SEQ_INIT;
  sequence.resize(lenMax);
  trickToPlay = 0;
  cardToPlay = 0; // So next card is #0 of trick #0
  playOverFlag = false;
  claimMadeFlag = false;
  tricksDecl = 0;
  tricksDef = 0;

}


void Play::SetTables()
{
  for (unsigned d = 0; d < 2 * BRIDGE_DENOMS; d++)
  {
    for (unsigned p = 0; p < BRIDGE_TRICKS; p++)
    {
      stringstream s;
      s << PLAY_DENOMS[d % 4] << PLAY_CARDS[p];
      string str = s.str();

      unsigned no = BRIDGE_TRICKS * (d % 4) + p;

      PLAY_NO_TO_CARD[no] = str;

      PLAY_CARD_TO_INFO[str].no = no;
      PLAY_CARD_TO_INFO[str].bitValue = 1u << (p + 2); // DDS encoding
      PLAY_CARD_TO_INFO[str].suit = d % 4;
      PLAY_CARD_TO_INFO[str].rank = p;

      PLAY_NO_TO_INFO[no] = PLAY_CARD_TO_INFO[str];
    }
  }

  // TRICK_RANKS[denomination][led suit][card no] is the value of
  // card in that context, so 0 if a discard, normal rank in the suit
  // led, higher if a trump.

  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
  {
    for (unsigned lead = 0; lead < BRIDGE_SUITS; lead++)
    {
      for (unsigned n = 0; n < PLAY_NUM_CARDS; n++)
        TRICK_RANKS[d][lead][n] = 0;

      // Suit of lead.
      for (unsigned c = 0; c < BRIDGE_TRICKS; c++)
        TRICK_RANKS[d][lead][BRIDGE_TRICKS*lead + c] = c;

      if (d == BRIDGE_NOTRUMP)
        continue;

      // Trumps.
      for (unsigned c = 0; c < BRIDGE_TRICKS; c++)
        TRICK_RANKS[d][lead][BRIDGE_TRICKS*d + c] = c + BRIDGE_TRICKS;
    }
  }
}


bool Play::SetContract(const Contract& contract)
{
  if (! contract.ContractIsSet() || contract.IsPassedOut())
    return false;

  return Play::SetDeclAndDenom(contract.GetDeclarer(), contract.GetDenom());
}


bool Play::SetDeclAndDenom(
  const playerType declIn,
  const denomType denomIn)
{
  if (setDDFlag)
  {
    LOG("Dealer and denomination already set");
    return false;
  }

  if (declIn >= BRIDGE_NORTH_SOUTH)
  {
    LOG("Invalid declarer " + STR(declIn));
    return false;
  }

  if (denomIn >  BRIDGE_NOTRUMP)
  {
    LOG("Invalid denomination " + STR(denomIn));
    return false;
  }

  setDDFlag = true;
  declarer = declIn;
  denom = denomIn;
  leads[0].leader = static_cast<playerType>((declarer + 1) % 4);
  return true;
}


bool Play::SetHoldingDDS(
  const unsigned h[][BRIDGE_SUITS])
{
  if (setDealFlag)
  {
    LOG("Holding already set");
    return false;
  }
  setDealFlag = true;

  // No error checking.
  for (int p = 0; p < BRIDGE_PLAYERS; p++)
    for (int d = 0; d < BRIDGE_SUITS; d++)
      holding[p][d] = h[p][d];

  return true;
}


playStatus Play::AddPlay(const string& str)
{
  if (playOverFlag)
  {
    LOG("Play is over");
    return PLAY_OVER;
  }

  if (! setDealFlag)
  {
    LOG("Holding not set");
    return PLAY_HOLDING_NOT_SET;
  }

  map<string, cardInfoType>::iterator it = PLAY_CARD_TO_INFO.find(str);
  if (it == PLAY_CARD_TO_INFO.end())
  {
    LOG("Invalid card " + str);
    return PLAY_CARD_INVALID;
  }
  const cardInfoType& INFO = PLAY_CARD_TO_INFO[str];

  playerType leader = leads[trickToPlay].leader;
  playerType player = static_cast<playerType>((leader + cardToPlay) % 4);

  if ((holding[player][INFO.suit] & INFO.bitValue) == 0)
  {
    LOG("Card " + str + " not held (possibly held earlier)");
    return PLAY_CARD_NOT_HELD;
  }

  if (cardToPlay > 0 && 
      INFO.suit != static_cast<unsigned>(leads[trickToPlay].suit) &&
      holding[player][leads[trickToPlay].suit] > 0)
  {
    LOG("Revoke " + str);
    return PLAY_REVOKE;
  }

  // So now it is OK to play the card.

  if (len == lenMax)
  {
    lenMax += PLAY_SEQ_INCR;
    sequence.resize(lenMax);
  }

  // Add the card to the play list.
  sequence[len] = INFO.no;
  len++;

  // Remove the card from the holding.
  holding[player][INFO.suit] ^= INFO.bitValue;

  if (cardToPlay == 0)
    leads[trickToPlay].suit = static_cast<denomType>(INFO.suit);

  if (cardToPlay == 3)
  {
    // Trick over.
    unsigned relWinner = Play::TrickWinnerRelative();
    unsigned absWinner = (leads[trickToPlay].leader + relWinner) % 4;

    cardToPlay = 0;
    trickToPlay++;

    if (trickToPlay == BRIDGE_TRICKS)
    {
      // Play over.
      playOverFlag = true;
    }
    else
    {
      leads[trickToPlay].leader = static_cast<playerType>(absWinner);
    }

    if ((declarer + absWinner) % 2 == 0)
    {
      tricksDecl++;
      leads[trickToPlay-1].wonByDeclarer = true;
    }
    else
    {
      tricksDef++;
      leads[trickToPlay-1].wonByDeclarer = false;
    }
  }
  else
  {
    cardToPlay++;
  }

  return PLAY_NO_ERROR;
}


unsigned Play::TrickWinnerRelative() const
{
  unsigned winner = 0;
  unsigned start = trickToPlay << 2;
  const unsigned * rp = TRICK_RANKS[denom][leads[trickToPlay].suit];
  unsigned highest = rp[sequence[start]];

  for (unsigned p = 1; p < BRIDGE_PLAYERS; p++)
  {
    if (rp[sequence[start+p]] > highest)
    {
      winner = p;
      highest = rp[sequence[start+p]];
    }
  }

  return winner;
}


playStatus Play::AddTrickPBN(const string& str)
{
  // In PBN the cards are given starting with the opening leader,
  // even for later tricks.

  string plays[BRIDGE_PLAYERS];
  unsigned count;
  if (! getWords(str, plays, 4, count))
  {
    LOG("Not a valid PBN play line " + str);
    return PLAY_INVALID_PBN;
  }

  unsigned offset = static_cast<unsigned>
    ((leads[trickToPlay].leader + 4 - leads[0].leader) % 4);
  for (unsigned p = offset; p < offset+count; p++)
  {
    unsigned pp = p % 4;
    if (pp >= count)
      continue;
    if (plays[pp] != "-")
    {
      playStatus ps = Play::AddPlay(plays[pp]);
      if (ps != PLAY_NO_ERROR)
        return ps;
    }
  }

  return PLAY_NO_ERROR;
}


playStatus Play::AddAllRBN(const string& sIn)
{
  if (sIn.length() < 2)
  {
    LOG("String too short: " + sIn);
    return PLAY_INVALID_RBN;
  }

  if (sIn.at(0) != 'P' || sIn.at(1) != ' ')
  {
    LOG("Must start with P");
    return PLAY_INVALID_RBN;
  }

  const string str = sIn.substr(2, string::npos);

  int seen = count(str.begin(), str.end(), ':');
  if (seen > BRIDGE_TRICKS-1)
  {
    LOG("Too many colons in RBN " + str);
    return PLAY_INVALID_RBN;
  }

  vector<string> tricks(BRIDGE_TRICKS);
  tricks.clear();
  tokenize(str, tricks, ":");

  for (unsigned t = 0; t < tricks.size(); t++)
  {
    const string& trick = tricks[t];
    const unsigned l = trick.length();
    if (l < 2 || l > 8)
    {
      LOG("Bad RBN trick " + trick);
      return PLAY_INVALID_RBN;
    }

    const char suitLed = trick.at(0); // Might be invalid
    stringstream s;
    unsigned i = 0;
    unsigned b = 0;
    while (i < l)
    {
      if (b >= BRIDGE_PLAYERS)
      {
        LOG("Too many plays in trick " + trick);
        return PLAY_INVALID_RBN;
      }

      s.str("");
      char next = trick.at(i);
      if (next == PLAY_DENOMS[0] || next == PLAY_DENOMS[1] ||
          next == PLAY_DENOMS[2] || next == PLAY_DENOMS[3])
      {
        s << next << trick.at(i+1);
        i += 2;
      }
      else
      {
        s << suitLed << next;
        i++;
      }
      playStatus ps = Play::AddPlay(s.str());
      if (ps != PLAY_NO_ERROR)
        return ps;
      b++;
    }
  }

  return PLAY_NO_ERROR;
}


playStatus Play::AddPlays(
  const string& str,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("Currently unimplemented format " + STR(f));
      return PLAY_INVALID_FORMAT;

    case BRIDGE_FORMAT_PBN:
      return Play::AddTrickPBN(str);

    case BRIDGE_FORMAT_RBN:
      return Play::AddAllRBN(str);

    case BRIDGE_FORMAT_TXT:
      LOG("Currently unimplemented format " + STR(f));
      return PLAY_INVALID_FORMAT;

    default:
      LOG("Invalid format " + STR(f));
      return PLAY_INVALID_FORMAT;
  }
}


bool Play::UndoPlay()
{
  if (playOverFlag)
  {
    LOG("Cannot undo play after play is over");
    return false;
  }

  if (len == 0)
  {
    LOG("Play has not started yet");
    return false;
  }

  if (cardToPlay == 0)
  {
    // Undo last card of previous trick, including winner etc.
    cardToPlay = 3;
    trickToPlay--;
    if (leads[trickToPlay].wonByDeclarer)
      tricksDecl--;
    else
      tricksDef--;
  }
  else
    cardToPlay--;

  unsigned cardUndone = sequence[len-1];
  len--;

  string str = PLAY_NO_TO_CARD[cardUndone];
  cardInfoType& info = PLAY_CARD_TO_INFO[str];
  playerType pUndone = static_cast<playerType>
    ((leads[trickToPlay].leader + cardToPlay) % 4);
  holding[pUndone][info.suit] ^= info.bitValue;

  return true;
}


bool Play::PlayIsOver() const
{
  return playOverFlag;
}


claimStatus Play::Claim(const unsigned tricks)
{
  if (claimMadeFlag)
  {
    if (tricksDecl == tricks)
      return PLAY_CLAIM_NO_ERROR;
    else
      return PLAY_CLAIM_ALREADY;
  }
  else if (playOverFlag)
    return PLAY_CLAIM_PLAY_OVER;
  else
  {
    playOverFlag = true;
    claimMadeFlag = true;
    tricksDecl = tricks;
    return PLAY_CLAIM_NO_ERROR;
  }
}

bool Play::ClaimIsMade() const
{
  return claimMadeFlag;
}


bool Play::operator == (const Play& p2) const
{
  // We don't require the holdings to be identical.

  if (setDDFlag != p2.setDDFlag)
  {
    LOG("DD not set in same way");
    return false;
  }
  else if (setDDFlag && (declarer != p2.declarer || denom != p2.denom))
  {
    LOG("DD difference");
    return false;
  }
  else if (setDealFlag != p2.setDealFlag)
  {
    LOG("Deal difference");
    return false;
  }
  else if (len != p2.len)
  {
    LOG("Length difference");
    return false;
  }
  else if (trickToPlay != p2.trickToPlay || cardToPlay != p2.cardToPlay)
  {
    LOG("Progress difference");
    return false;
  }
  else if (playOverFlag != p2.playOverFlag)
  {
    LOG("Play-over difference");
    return false;
  }
  else if (claimMadeFlag != p2.claimMadeFlag)
  {
    LOG("Claim status difference");
    return false;
  }
  else if (tricksDecl != p2.tricksDecl || tricksDef != p2.tricksDef)
  {
    LOG("Claim difference");
    return false;
  }

  for (unsigned n = 0; n < len; n++)
    if (sequence[n] != p2.sequence[n])
    {
      LOG("Sequence difference");
      return false;
    }
    
  // leads are implicitly identical when the plays are identical.
  return true;
}


bool Play::operator != (const Play& p2) const
{
  return ! (* this == p2);
}


string Play::AsLIN() const
{
  stringstream s;
  for (unsigned l = 0; l < len; l++)
  {
    s << "pc|" << PLAY_NO_TO_CARD[sequence[l]] << "|";
    if (l % 4 == 3)
      s << "pg||";
  }

  return s.str();
}


string Play::AsPBN() const
{
  if (len == 0)
    return "";
  stringstream s;

  playerType openingLeader = leads[0].leader;
  s << "[Play \"" << PLAYER_NAMES_LONG[openingLeader] << "\"]\n";
  for (unsigned t = 0; t < trickToPlay; t++)
  {
    unsigned offset = t << 2;
    for (unsigned c = 0; c < 4; c++)
    {
      unsigned p = offset + (openingLeader + 4 - leads[t].leader + c) % 4;
      s << PLAY_NO_TO_CARD[sequence[p]] << " ";
    }
    s << "\n";
  }

  if (len > trickToPlay << 2)
  {
    unsigned offset = trickToPlay << 2;
    bool dashing = true;
    for (unsigned c = 0; c < 4; c++)
    {
      unsigned p = 
        offset + (openingLeader + 4 - leads[trickToPlay].leader + c) % 4;
      if (p < len)
      {
        dashing = false;
        s << PLAY_NO_TO_CARD[sequence[p]] << " ";
      }
      else if (dashing)
        s << " - ";
    }
    s << "\n";
  }

  if (claimMadeFlag)
    s << "*\n";

  return s.str();
}


string Play::AsRBN() const
{
  stringstream s;
  s << "P ";
  for (unsigned l = 0; l < len; l++)
  {
    if (l % 4 == 0)
      s << PLAY_NO_TO_CARD[sequence[l]];
    else if (PLAY_NO_TO_INFO[sequence[l]].suit != 
        static_cast<unsigned>(leads[l >> 2].suit))
      s << PLAY_NO_TO_CARD[sequence[l]];
    else
      s << PLAY_CARDS[PLAY_NO_TO_INFO[sequence[l]].rank];

    if (l % 4 == 3 && l != len-1)
      s << ":";
  }
  s << "\n";
  return s.str();

}


string Play::AsTXT() const
{
  if (len == 0)
    return "";
  stringstream s;

  s << setw(6) << "Leader" <<
       setw(4) << "C1" <<
       setw(4) << "C2" <<
       setw(4) << "C3" <<
       setw(4) << "C4" <<
       setw(11) << "Winner" <<
       setw(6) << "Decl" <<
       setw(5) << "Def" << "\n";

  unsigned tDecl = 0;
  unsigned tDef = 0;

  for (unsigned l = 0; l < len; l++)
  {
    unsigned c = l % 4;
    unsigned t = l >> 2;
    if (c == 0)
      s << setw(6) << PLAYER_NAMES_LONG[leads[t].leader];

    s << setw(4) << PLAY_NO_TO_CARD[sequence[l]];

    if (c == 3)
    {
      if (leads[t].wonByDeclarer)
      {
        s << setw(11) << "declarer";
        tDecl++;
      }
      else
      {
        s << setw(11) << "defenders";
        tDef++;
      }
      s << setw(6) << tDecl << setw(5) << tDef << "\n";
    }
  }
  if (len % 4 != 3)
    s << "\n";

  if (claimMadeFlag)
    s << "Claim: " << tricksDecl << " tricks";
  s << "\n\n";

  return s.str();
}


string Play::AsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Play::AsLIN();

    case BRIDGE_FORMAT_PBN:
      return Play::AsPBN();

    case BRIDGE_FORMAT_RBN:
      return Play::AsRBN();

    case BRIDGE_FORMAT_TXT:
      return Play::AsTXT();

    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}


string Play::ClaimAsLIN() const
{
  if (!claimMadeFlag)
    return "";

  stringstream s;
  s << "mc|" << tricksDecl << "|";

  return s.str();
  
}


string Play::ClaimAsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Play::ClaimAsLIN();

    case BRIDGE_FORMAT_PBN:
      LOG("Currently unimplemented format " + STR(f));
      return "";

    case BRIDGE_FORMAT_RBN:
      LOG("Currently unimplemented format " + STR(f));
      return "";

    case BRIDGE_FORMAT_TXT:
      LOG("Currently unimplemented format " + STR(f));
      return "";

    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}

