/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <algorithm>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif

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

struct CardInfo
{
  unsigned no;
  unsigned bitValue;
  unsigned suit;
  unsigned rank;
};

static string PLAY_NO_TO_CARD[PLAY_NUM_CARDS];
static string PLAY_NO_TO_CARD_TXT[PLAY_NUM_CARDS];

static map<string, CardInfo> PLAY_CARD_TO_INFO; // All syntaxes

static CardInfo PLAY_NO_TO_INFO[PLAY_NUM_CARDS];

static unsigned TRICK_RANKS[BRIDGE_DENOMS][BRIDGE_SUITS][PLAY_NUM_CARDS];

static bool setPlayTables = false;

static bool bothAreSpaces(char lhs, char rhs);


Play::Play()
{
  Play::reset();
  if (! setPlayTables)
  {
    mtx.lock();
    if (! setPlayTables)
      Play::setTables();
    setPlayTables = true;
    mtx.unlock();
  }
}


Play::~Play()
{
}


void Play::reset()
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


void Play::setTables()
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


void Play::setDeclAndDenom(
  const Player declIn,
  const Denom denomIn)
{
  if (setDDFlag)
  {
    if (declIn == declarer && denomIn == denom)
      return;

    if (declIn < BRIDGE_NORTH || declIn > BRIDGE_WEST)
      THROW("Declarer out of range");
    if (denomIn < BRIDGE_SPADES || denomIn > BRIDGE_NOTRUMP)
      THROW("Denomination out of range");

    THROW("Declarer and denomination reset: new '" + 
      PLAYER_NAMES_LONG[declIn] + " " +
      DENOM_NAMES_LONG[denomIn] + "' vs set '" +
      PLAYER_NAMES_LONG[declarer] + " " +
      DENOM_NAMES_LONG[denom] + "'");
  }

  if (declIn >= BRIDGE_NORTH_SOUTH)
    THROW("Invalid declarer: " + STR(declIn));

  if (denomIn >  BRIDGE_NOTRUMP)
    THROW("Invalid denomination: " + STR(denomIn));

  setDDFlag = true;
  declarer = declIn;
  denom = denomIn;
  leads[0].leader = static_cast<Player>((declarer + 1) % 4);
}


void Play::setContract(const Contract& contract)
{
  if (! contract.isSet() || contract.isPassedOut())
    return;

  Play::setDeclAndDenom(contract.getDeclarer(), contract.getDenom());
}


void Play::setHoldingDDS(const unsigned h[][BRIDGE_SUITS])
{
  if (setDealFlag)
    THROW("Holding already set");

  setDealFlag = true;

  // No error checking.
  for (int p = 0; p < BRIDGE_PLAYERS; p++)
    for (int d = 0; d < BRIDGE_SUITS; d++)
      holding[p][d] = h[p][d];
}


unsigned Play::trickWinnerRelative() const
{
  unsigned winner = 0;
  const unsigned start = trickToPlay << 2;
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


void Play::addTrickPBN(const string& text)
{
  // In PBN the cards are given starting with the opening leader,
  // even for later tricks.

  string plays[BRIDGE_PLAYERS];
  unsigned count;
  if (! getWords(text, plays, 4, count))
    THROW("Not a valid PBN play line: " + text);

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
}


void Play::addPlay(const string& text)
{
  if (playOverFlag)
    THROW("Play is over");

  if (! setDealFlag)
    THROW("Holding not set");

  auto it = PLAY_CARD_TO_INFO.find(text);
  if (it == PLAY_CARD_TO_INFO.end())
    THROW("Invalid card: " + text);

  const CardInfo& info = it->second;

  Player leader = leads[trickToPlay].leader;
  Player player = static_cast<Player>
    ((static_cast<unsigned>(leader) + cardToPlay) % 4);

  if ((holding[player][info.suit] & info.bitValue) == 0)
    THROW("Card " + text + " not held (possibly held earlier)");

  if (cardToPlay > 0 && 
      info.suit != static_cast<unsigned>(leads[trickToPlay].suit) &&
      holding[player][leads[trickToPlay].suit] > 0)
    THROW("Revoke: " + text);

  // So now it is OK to play the card.

  if (len == lenMax)
  {
    lenMax += PLAY_SEQ_INCR;
    sequence.resize(lenMax);
  }

  // Add the card to the play list.
  sequence[len] = info.no;
  len++;

  // Remove the card from the holding.
  holding[player][info.suit] ^= info.bitValue;

  if (cardToPlay == 0)
    leads[trickToPlay].suit = static_cast<Denom>(info.suit);

  if (cardToPlay == 3)
  {
    // Trick over.
    unsigned relWinner = Play::trickWinnerRelative();
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
      leads[trickToPlay].leader = static_cast<Player>(absWinner);
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


static bool bothAreSpaces(char lhs, char rhs)
{
  // stackoverflow.com/questions/8362094/
  // replace-multiple-spaces-with-one-space-in-a-string
  return (lhs == rhs) && (lhs == ' ');
}


void Play::setPlaysPBN(const vector<string>& list)
{
  if (! setDDFlag)
    THROW("Declarer and denomination should be set by now");

  Player opldr;
  if (! char2player(list[0].at(0), opldr))
    THROW("Not an opening leader");

  if ((declarer + 1) % 4 != opldr)
    THROW("Wrong opening leader");

  for (unsigned i = 1; i < list.size(); i++)
  {
    // Compress adjacent spaces just to be sure.
    string s = list[i];
    auto new_end = unique(s.begin(), s.end(), bothAreSpaces);
    s.erase(new_end, s.end());

    Play::addTrickPBN(s);
  }
}


void Play::setPlaysRBN(const string& text)
{
  string str = text;
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
    stringstream ss;
    unsigned i = 0;
    unsigned b = 0;
    while (i < l)
    {
      if (b >= BRIDGE_PLAYERS)
        THROW("Too many plays in trick " + trick);

      ss.str("");
      char next = trick.at(i);
      if (next == PLAY_DENOMS[0] || next == PLAY_DENOMS[1] ||
          next == PLAY_DENOMS[2] || next == PLAY_DENOMS[3])
      {
        ss << next << trick.at(i+1);
        i += 2;
      }
      else
      {
        ss << suitLed << next;
        i++;
      }
      Play::addPlay(ss.str());
      b++;
    }
  }

  if (playOverFlag)
    Play::makeClaim(tricksDecl);
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
        str2lines(text, lines);
        Play::setPlaysPBN(lines);
        break;
      }

    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_REC:
      Play::setPlaysRBN(text);
      break;

    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Play::undoPlay()
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
  CardInfo& info = PLAY_CARD_TO_INFO[str];
  Player pUndone = static_cast<Player>
    ((static_cast<unsigned>(leads[trickToPlay].leader) + cardToPlay) % 4);
  holding[pUndone][info.suit] ^= info.bitValue;
}


bool Play::isOver() const
{
  return playOverFlag;
}


void Play::makeClaim(const unsigned tricks)
{
  if (claimMadeFlag)
  {
    if (tricksDecl == tricks)
      return;
    else
      THROW("Other claim already made");
  }
  else if (playOverFlag)
  {
    if (tricksDecl == tricks)
    {
      claimMadeFlag = true;
      return;
    }
    else
      THROW("Claim when play is already over");
  }
  else
  {
    playOverFlag = true;
    claimMadeFlag = true;
    tricksDecl = tricks;
  }
}


bool Play::hasClaim() const
{
  return claimMadeFlag;
}


unsigned Play::getTricks() const
{
  return tricksDecl;
}


void Play::getStateDDS(RunningDD& runningDD) const
{
  runningDD.tricksDecl = tricksDecl;
  runningDD.tricksDef = tricksDef;

  runningDD.dl.trump = denom;
  runningDD.dl.first = leads[trickToPlay].leader;

  const unsigned partial = len % 4;
  runningDD.declLeadFlag = 
    (((runningDD.dl.first + static_cast<int>(partial)) % 2) == 
      (declarer % 2));

  for (int p = 0; p < BRIDGE_PLAYERS; p++)
    for (int d = 0; d < BRIDGE_SUITS; d++)
      runningDD.dl.remainCards[p][d] = holding[p][d];

  for (int c = 0; c < 3; c++)
  {
    runningDD.dl.currentTrickSuit[c] = 0;
    runningDD.dl.currentTrickRank[c] = 0;
  }

  for (unsigned c = 0; c < partial; c++)
  {
    const CardInfo& ci = PLAY_NO_TO_INFO[sequence[4 * trickToPlay + c]];
    runningDD.dl.currentTrickSuit[c] = static_cast<int>(ci.suit);
    runningDD.dl.currentTrickRank[c] = static_cast<int>(ci.rank+2);
  }
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
  stringstream ss;
  for (unsigned t = 0; t < ((len+3) >> 2); t++)
  {
    ss << "pc|";
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned pos = 4*t + p;
      if (pos >= len)
        break;
      ss << PLAY_NO_TO_CARD[sequence[pos]];
    }
    ss << "|pg||\n";
  }
  return ss.str();
}


string Play::strLIN_VG() const
{
  if (len == 0)
    return "";

  stringstream ss;
  for (unsigned t = 0; t < ((len+3) >> 2); t++)
  {
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned pos = 4*t + p;
      if (pos >= len)
        break;
      ss << "pc|" << PLAY_NO_TO_CARD[sequence[pos]] << "|";
    }
    ss << "pg||\n";
  }
  return ss.str();
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

  stringstream ss;
  Player openingLeader = leads[0].leader;
  ss << "[Play \"" << PLAYER_NAMES_SHORT[openingLeader] << "\"]\n";
  for (unsigned t = 0; t < trickToPlay; t++)
  {
    unsigned offset = t << 2;
    for (unsigned c = 0; c < 4; c++)
    {
      unsigned p = offset + (static_cast<unsigned>(openingLeader) + 4u - 
        static_cast<unsigned>(leads[t].leader) + c) % 4;
      if (c > 0)
        ss << " ";
      ss << PLAY_NO_TO_CARD[sequence[p]];
    }
    ss << "\n";
  }

  // The incomplete last trick is different with Pavlicek:
  // He uses a double dash (--), not a single dash (-).
  // He uses trailing (double) dashes, whereas PBN probably does not.
  // He also uses a * even if all cards have been played (PBN does not).

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
        ss << " ";
      if (p < len)
      {
        ss << PLAY_NO_TO_CARD[sequence[p]];
        num--;
      }
      else
        ss << "--";
      // For PBN behavior:  
      // else if (num > 0) 
      //   ss << "- ";
    }
    ss << "\n";
  }

  // For PBN behavior:
  // if (claimMadeFlag)
  ss << "*\n";

  return ss.str();
}


string Play::strRBNCore() const
{
  stringstream ss;
  for (unsigned l = 0; l < len; l++)
  {
    if (l % 4 == 0)
      ss << PLAY_NO_TO_CARD[sequence[l]];
    else if (PLAY_NO_TO_INFO[sequence[l]].suit != 
        static_cast<unsigned>(leads[l >> 2].suit))
      ss << PLAY_NO_TO_CARD[sequence[l]];
    else
      ss << PLAY_CARDS[PLAY_NO_TO_INFO[sequence[l]].rank];

    if (l % 4 == 3 && l != len-1)
      ss << ":";
  }
  return ss.str();
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

  stringstream ss;
  if (len == 1)
  {
    ss << "Lead: " << PLAY_NO_TO_CARD_TXT[sequence[0]] << "\n";
    return ss.str();
  }

  ss << setw(5) << "Trick" <<
    setw(7) << "Lead" <<
    setw(7) << "2nd" <<
    setw(7) << "3rd" <<
    setw(7) << "4th" << "\n";

  for (unsigned t = 0; t < ((len+3) >> 2); t++)
  {
    ss << t+1 << ". " << PLAYER_NAMES_SHORT[leads[t].leader];
    if (t >= 9)
      ss << "   ";
    else
      ss << "    ";

    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned pp = 4*t + p;
      if (pp >= len)
        break;

      // Pavlicek?
      const int width = (p == 0 ? 8 : 7);

      // Full card when lead or when other suit than lead.
      const string st = (p == 0 || 
          PLAY_NO_TO_INFO[sequence[pp]].suit != 
          static_cast<unsigned>(leads[t].suit) ?
          PLAY_NO_TO_CARD_TXT[sequence[pp]] :
          PLAY_CARDS_TXT[PLAY_NO_TO_INFO[sequence[pp]].rank]);

        if (p == 3 || pp == len-1)
          ss << st;
        else
          ss << setw(width) << left << st;
    }
    ss << "\n";
  }

  return ss.str();
}


string Play::strEML() const
{
  stringstream ss;
  ss << " ";
  for (unsigned l = 0; l < (len+3) >> 2; l++)
    ss << setw(3) << l+1;
  ss << "\n";

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
    ss << ps[p].str() << "\n";
  return ss.str();
}


string Play::strREC() const
{
  if (len == 0)
    return "";

  stringstream ss;

  for (unsigned t = 0; t < ((len+3) >> 2); t++)
  {
    ss << setw(2) << right << t+1 << "  " << 
        setw(6) << left << PLAYER_NAMES_LONG[leads[t].leader];

    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      unsigned pp = 4*t + p;
      if (pp >= len)
        break;

      if (p == 0)
        ss << setw(3) << right << PLAY_NO_TO_CARD[sequence[pp]];
      else if (PLAY_NO_TO_INFO[sequence[pp]].suit != 
          static_cast<unsigned>(leads[t].suit))
      {
        ss << setw(3) << right << PLAY_NO_TO_CARD[sequence[pp]];
      }
      else
        ss << setw(3) << right << 
            PLAY_CARDS[PLAY_NO_TO_INFO[sequence[pp]].rank];

      if (p != 3 && pp != len-1)
        ss << ",";
    }
    ss << "\n";
  }

  return ss.str() + "\n";
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


string Play::strDeclarer(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PAR:
      return PLAYER_NAMES_SHORT[declarer];

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Play::strDenom(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_PAR:
      return STR(DENOM_NAMES_SHORT[denom]);

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

