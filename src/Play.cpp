/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <map>
#include <algorithm>
#include <thread>
#include <mutex>

#include "Play.h"
#include "Contract.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"

static mutex mtx;


#define PLAY_SEQ_INIT 28
#define PLAY_SEQ_INCR 24

#define PLAY_NUM_CARDS 52


static const char PLAY_CARDS[BRIDGE_TRICKS] =
{
  '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'
};

static const string PLAY_CARDS_TXT[BRIDGE_TRICKS] =
{
  "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"
};

static const char PLAY_DENOMS[2 * BRIDGE_SUITS] =
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

static string PLAY_NO_TO_CARD[PLAY_NUM_CARDS];
static string PLAY_NO_TO_CARD_TXT[PLAY_NUM_CARDS];

static map<string, cardInfoType> PLAY_CARD_TO_INFO; // All syntaxes

static cardInfoType PLAY_NO_TO_INFO[PLAY_NUM_CARDS];

static unsigned TRICK_RANKS[BRIDGE_DENOMS][BRIDGE_SUITS][PLAY_NUM_CARDS];

static bool setPlayTables = false;

static bool BothAreSpaces(char lhs, char rhs);


Play::Play()
{
  Play::Reset();
  if (! setPlayTables)
  {
    mtx.lock();
    if (! setPlayTables)
      Play::SetTables();
    setPlayTables = true;
    mtx.unlock();
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

      stringstream s2;
      s2 << PLAY_DENOMS[d % 4] << PLAY_CARDS_TXT[p];
      PLAY_NO_TO_CARD_TXT[no] = s2.str();

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
    return true;

  return Play::SetDeclAndDenom(contract.GetDeclarer(), contract.GetDenom());
}


bool Play::SetDeclAndDenom(
  const playerType declIn,
  const denomType denomIn)
{
  if (setDDFlag)
  {
    if (declIn == declarer && denomIn == denom)
      return true;

    THROW("Declarer and denomination reset, " + 
      STR(declIn) + STR(denomIn) + " " + STR(declarer) + STR(denomIn));
  }

  if (declIn >= BRIDGE_NORTH_SOUTH)
    THROW("Invalid declarer " + STR(declIn));

  if (denomIn >  BRIDGE_NOTRUMP)
    THROW("Invalid denomination " + STR(denomIn));

  setDDFlag = true;
  declarer = declIn;
  denom = denomIn;
  leads[0].leader = static_cast<playerType>((declarer + 1) % 4);
  return true;
}


void Play::SetHoldingDDS(
  const unsigned h[][BRIDGE_SUITS])
{
  if (setDealFlag)
    THROW("Holding already set");

  setDealFlag = true;

  // No error checking.
  for (int p = 0; p < BRIDGE_PLAYERS; p++)
    for (int d = 0; d < BRIDGE_SUITS; d++)
      holding[p][d] = h[p][d];
}


void Play::addPlay(const string& str)
{
  if (playOverFlag)
    THROW("Play is over");

  if (! setDealFlag)
    THROW("Holding not set");

  auto it = PLAY_CARD_TO_INFO.find(str);
  if (it == PLAY_CARD_TO_INFO.end())
    THROW("Invalid card " + str);

  const cardInfoType& INFO = PLAY_CARD_TO_INFO[str];

  playerType leader = leads[trickToPlay].leader;
  playerType player = static_cast<playerType>
    ((static_cast<unsigned>(leader) + cardToPlay) % 4);

  if ((holding[player][INFO.suit] & INFO.bitValue) == 0)
    THROW("Card " + str + " not held (possibly held earlier)");

  if (cardToPlay > 0 && 
      INFO.suit != static_cast<unsigned>(leads[trickToPlay].suit) &&
      holding[player][leads[trickToPlay].suit] > 0)
    THROW("Revoke " + str);

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
    unsigned absWinner = 
      (static_cast<unsigned>(leads[trickToPlay].leader) + relWinner) % 4;

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

    if ((static_cast<unsigned>(declarer) + absWinner) % 2 == 0)
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
    THROW("Not a valid PBN play line " + str);

  unsigned offset = static_cast<unsigned>
    ((leads[trickToPlay].leader + 4 - leads[0].leader) % 4);
  for (unsigned p = offset; p < offset+count; p++)
  {
    unsigned pp = p % 4;
    if (pp >= count)
      continue;
    if (plays[pp].at(0) != '-') // - and --
      Play::addPlay(plays[pp]);
  }

  return PLAY_NO_ERROR;
}


bool Play::AddAllRBN(const string& sIn)
{
  string str = sIn;
  if (str.length() < 2)
    THROW("String too short: " + str);

  toUpper(str);

  int seen = count(str.begin(), str.end(), ':');
  if (seen > BRIDGE_TRICKS-1)
    THROW("Too many colons in RBN " + str);

  vector<string> tricks(BRIDGE_TRICKS);
  tricks.clear();
  tokenize(str, tricks, ":");

  for (unsigned t = 0; t < tricks.size(); t++)
  {
    const string& trick = tricks[t];
    const unsigned l = trick.length();
    if (l < 2 || l > 8)
      THROW("Bad RBN trick " + trick);

    const char suitLed = trick.at(0); // Might be invalid
    stringstream s;
    unsigned i = 0;
    unsigned b = 0;
    while (i < l)
    {
      if (b >= BRIDGE_PLAYERS)
        THROW("Too many plays in trick " + trick);

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
      Play::addPlay(s.str());
      b++;
    }
  }

  if (playOverFlag)
  {
    if (Play::Claim(tricksDecl) != PLAY_CLAIM_NO_ERROR)
      THROW("Claim error");
  }


  return true;
}


playStatus Play::setPlay(
  const string& str,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      return Play::AddTrickPBN(str);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Play::setPlays(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_PBN:
      {
        vector<string> lines;
        ConvertMultilineToVector(text, lines);
        Play::SetPlaysPBN(lines);
        break;
      }

    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_REC:
      Play::AddAllRBN(text);
      break;

    default:
      THROW("Invalid format " + STR(format));
  }
}


static bool BothAreSpaces(char lhs, char rhs)
{
  // stackoverflow.com/questions/8362094/
  // replace-multiple-spaces-with-one-space-in-a-string
  return (lhs == rhs) && (lhs == ' ');
}


bool Play::SetPlaysPBN(const vector<string>& list)
{
  if (! setDDFlag)
    THROW("Declarer and denomination should be set by now");

  playerType opldr;
  if (! ParsePlayer(list[0].at(0), opldr))
    THROW("Not an opening leader");

  if ((declarer + 1) % 4 != opldr)
    THROW("Wrong opening leader");

  for (unsigned i = 1; i < list.size(); i++)
  {
    // Compress adjacent spaces just to be sure.
    string s = list[i];
    auto new_end = unique(s.begin(), s.end(), BothAreSpaces);
    s.erase(new_end, s.end());

    if (Play::AddTrickPBN(s) != PLAY_NO_ERROR)
      return false;
  }

  return true;
}


bool Play::UndoPlay()
{
  if (playOverFlag)
    THROW("Cannot undo play after play is over");

  if (len == 0)
    THROW("Play has not started yet");

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
    ((static_cast<unsigned>(leads[trickToPlay].leader) + cardToPlay) % 4);
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
  {
    if (tricksDecl == tricks)
    {
      claimMadeFlag = true;
      return PLAY_CLAIM_NO_ERROR;
    }
    else
      return PLAY_CLAIM_PLAY_OVER;
  }
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


unsigned Play::GetTricks() const
{
  return tricksDecl;
}


bool Play::operator == (const Play& play2) const
{
  // We don't require the holdings to be identical.

  if (setDDFlag != play2.setDDFlag)
    DIFF("DD not set in same way");
  if (setDDFlag && (declarer != play2.declarer || denom != play2.denom))
    DIFF("DD difference");
  if (setDealFlag != play2.setDealFlag)
    DIFF("Deal difference");
  if (len != play2.len)
    DIFF("Length difference");
  if (trickToPlay != play2.trickToPlay || cardToPlay != play2.cardToPlay)
    DIFF("Progress difference");
  if (playOverFlag != play2.playOverFlag)
    DIFF("Play-over difference");
  if (claimMadeFlag != play2.claimMadeFlag)
    DIFF("Claim status difference");
  if (tricksDecl != play2.tricksDecl || tricksDef != play2.tricksDef)
    DIFF("Claim difference");

  for (unsigned n = 0; n < len; n++)
    if (sequence[n] != play2.sequence[n])
      DIFF("Sequence difference");
    
  // Leads are implicitly identical when the plays are identical.
  return true;
}


bool Play::operator != (const Play& play2) const
{
  return ! (* this == play2);
}


string Play::strLIN() const
{
  stringstream ss;
  for (unsigned l = 0; l < len; l++)
  {
    ss << "pc|" << PLAY_NO_TO_CARD[sequence[l]] << "|";
    if (l % 4 == 3)
      ss << "pg||";
  }
  return ss.str();
}


string Play::strLIN_RP() const
{
  if (len == 0)
    return "";

  stringstream s;
  for (unsigned t = 0; t <= ((len-1) >> 2); t++)
  {
    s << "pc|";
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned pos = 4*t + p;
      if (pos >= len)
        break;
      s << PLAY_NO_TO_CARD[sequence[pos]];
    }
    s << "|pg||\n";
  }
  return s.str();
}


string Play::strLIN_VG() const
{
  if (len == 0)
    return "";

  stringstream s;
  for (unsigned t = 0; t <= ((len-1) >> 2); t++)
  {
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned pos = 4*t + p;
      if (pos >= len)
        break;
      s << "pc|" << PLAY_NO_TO_CARD[sequence[pos]] << "|";
    }
    s << "pg||\n";
  }
  return s.str();
}


string Play::strLIN_TRN() const
{
  stringstream ss;
  for (unsigned l = 0; l < len; l++)
  {
    ss << "pc|" << PLAY_NO_TO_CARD[sequence[l]] << "|";
    if (l % 4 == 3 && (l != len-1 || len == PLAY_NUM_CARDS))
      ss << "pg||";
  }
  return ss.str();
}


string Play::strPBN() const
{
  if (len == 0)
    return "";
  stringstream s;

  playerType openingLeader = leads[0].leader;
  s << "[Play \"" << PLAYER_NAMES_SHORT[openingLeader] << "\"]\n";
  for (unsigned t = 0; t < trickToPlay; t++)
  {
    unsigned offset = t << 2;
    for (unsigned c = 0; c < 4; c++)
    {
      unsigned p = offset + (static_cast<unsigned>(openingLeader) + 4u - 
        static_cast<unsigned>(leads[t].leader) + c) % 4;
      if (c > 0)
        s << " ";
      s << PLAY_NO_TO_CARD[sequence[p]];
    }
    s << "\n";
  }

  if (len > trickToPlay << 2)
  {
    unsigned offset = trickToPlay << 2;
    unsigned num = len - offset;
    for (unsigned c = 0; c < 4; c++)
    {
      unsigned p = 
        offset + (static_cast<unsigned>(openingLeader) + 4u - 
          static_cast<unsigned>(leads[trickToPlay].leader) + c) % 4;
      if (c > 0)
        s << " ";
      if (p < len)
      {
        s << PLAY_NO_TO_CARD[sequence[p]];
        num--;
      }
      // TODO
      // PBN uses single dash, and probably not trailing dashes.
      else // if (num > 0)
        s << "--";
        // s << "- ";
    }
    s << "\n";
  }

  // TODO
  // PBN only uses * if not all cards have been played.
  // if (claimMadeFlag)
    s << "*\n";

  return s.str();
}


string Play::strRBNCore() const
{
  stringstream s;
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
  return s.str();
}


string Play::strRBN() const
{
  if (len == 0)
    return "";
  else
    return "P " + Play::strRBNCore() + "\n";
}


string Play::strRBX() const
{
  if (len == 0)
    return "";
  else
    return "P{" + Play::strRBNCore() + "}";
}


string Play::strTXT() const
{
  if (len == 0)
    return "";

  stringstream s;
  if (len == 1)
  {
    s << "Lead: " << PLAY_NO_TO_CARD_TXT[sequence[0]] << "\n";
    return s.str();
  }


  s << setw(5) << "Trick" <<
       setw(7) << "Lead" <<
       setw(7) << "2nd" <<
       setw(7) << "3rd" <<
       setw(7) << "4th" << "\n";

  for (unsigned t = 0; t <= ((len-1) >> 2); t++)
  {
    s << t+1 << ". " << PLAYER_NAMES_SHORT[leads[t].leader];
    if (t >= 9)
      s << "   ";
    else
      s << "    ";

    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned pp = 4*t + p;
      if (pp >= len)
        break;

      if (p == 0)
      {
        if (pp == len-1)
          s << PLAY_NO_TO_CARD_TXT[sequence[pp]];
        else
          s << setw(8) << left << PLAY_NO_TO_CARD_TXT[sequence[pp]];
      }
      else if (PLAY_NO_TO_INFO[sequence[pp]].suit != 
          static_cast<unsigned>(leads[t].suit))
      {
        if (p == 3 || pp == len-1)
          s << PLAY_NO_TO_CARD_TXT[sequence[pp]];
        else
          s << setw(7) << left << PLAY_NO_TO_CARD_TXT[sequence[pp]];
      }
      else
      {
        if (p == 3 || pp == len-1)
          s << PLAY_CARDS_TXT[PLAY_NO_TO_INFO[sequence[pp]].rank];
        else
          s << setw(7) << left <<
              PLAY_CARDS_TXT[PLAY_NO_TO_INFO[sequence[pp]].rank];
      }
    }
    s << "\n";
  }

  return s.str();
}


string Play::strEML() const
{
  stringstream s;
  s << " ";
  for (unsigned l = 0; l < (len+3) >> 2; l++)
    s << setw(3) << l+1;
  s << "\n";

  stringstream ps[BRIDGE_PLAYERS];
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    ps[p] << PLAYER_NAMES_SHORT[(p+3) % 4];

  for (unsigned l = 0; l < len; l++)
  {
    unsigned t = l >> 2;
    unsigned pEML = (static_cast<unsigned>(leads[t].leader) + 1u + l) % 4;
    if (t > 0 && l % 4 == 0)
      ps[pEML] << "-";
    else
      ps[pEML] << " ";

    if (l % 4 == 0)
      ps[pEML] << setw(2) << PLAY_NO_TO_CARD[sequence[l]];
    else if (PLAY_NO_TO_INFO[sequence[l]].suit != 
        static_cast<unsigned>(leads[l >> 2].suit))
      ps[pEML] << setw(2) << PLAY_NO_TO_CARD[sequence[l]];
    else
      ps[pEML] << setw(2) << PLAY_CARDS[PLAY_NO_TO_INFO[sequence[l]].rank];
  }

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    s << ps[p].str() << "\n";
  return s.str();
}


string Play::strREC() const
{
  if (len == 0)
    return "";

  stringstream s;

  for (unsigned t = 0; t <= ((len-1) >> 2); t++)
  {
    s << setw(2) << right << t+1 << "  " << 
        setw(6) << left << PLAYER_NAMES_LONG[leads[t].leader];

    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned pp = 4*t + p;
      if (pp >= len)
        break;

      if (p == 0)
        s << setw(3) << right << PLAY_NO_TO_CARD[sequence[pp]];
      else if (PLAY_NO_TO_INFO[sequence[pp]].suit != 
          static_cast<unsigned>(leads[t].suit))
      {
        s << setw(3) << right << PLAY_NO_TO_CARD[sequence[pp]];
      }
      else
        s << setw(3) << right << 
            PLAY_CARDS[PLAY_NO_TO_INFO[sequence[pp]].rank];

      if (p != 3 && pp != len-1)
        s << ",";
    }
    s << "\n";
  }

  return s.str() + "\n";
}


string Play::str(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return Play::strLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Play::strLIN_RP();

    case BRIDGE_FORMAT_LIN_VG:
      return Play::strLIN_VG();

    case BRIDGE_FORMAT_LIN_TRN:
      return Play::strLIN_TRN();

    case BRIDGE_FORMAT_PBN:
      return Play::strPBN();

    case BRIDGE_FORMAT_RBN:
      return Play::strRBN();

    case BRIDGE_FORMAT_RBX:
      return Play::strRBX();

    case BRIDGE_FORMAT_TXT:
      return Play::strTXT();

    case BRIDGE_FORMAT_EML:
      return Play::strEML();

    case BRIDGE_FORMAT_REC:
      return Play::strREC();

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Play::strLead(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_EML:
      if (len == 0)
        return "Opening Lead:";
      else
        return "Opening Lead: " + PLAY_NO_TO_CARD[sequence[0]];

    case BRIDGE_FORMAT_REC:
      if (len == 0)
        return "Opening lead:   ";
      else
        return "Opening lead: " + PLAY_NO_TO_CARD[sequence[0]];

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Play::strClaimLIN() const
{
  if (! claimMadeFlag)
    return "";

  return "mc|" + STR(tricksDecl) + "|";
  
}


string Play::strClaim(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return Play::strClaimLIN() + "pg||\n";

    case BRIDGE_FORMAT_LIN_RP:
      if (claimMadeFlag)
        return Play::strClaimLIN() + "pg||\n\n";
      else
        return "\n";

    case BRIDGE_FORMAT_LIN_VG:
      if (len == PLAY_NUM_CARDS)
        return "\n\n";
      else
        return Play::strClaimLIN() + "pg||\n\n";

    case BRIDGE_FORMAT_LIN_TRN:
      if (len == PLAY_NUM_CARDS)
        return "\n";
      else
        return Play::strClaimLIN() + "pg||\n";

    default:
      THROW("Invalid format: " + STR(format));
  }
}

