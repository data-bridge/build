/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_VALUATION_H
#define BRIDGE_VALUATION_H

#include <array>
#include <string>

#include "Term.h"

#include "../include/bridge.h"

class DistMatcher;

using namespace std;

/*
   Performs valuation of a deal given its numerical pattern.
   A suit holding is a number in the range 0 .. 8191 (2^13 - 1).

   A number of lookup tables are created at the beginning, some for
   suits and some for distributions.  That still leaves several "composite"
   evaluation criteria that take both into account in some way.

   Some of the criteria are standard, some more experimental.

   SUIT_SCORE_TABLE[holding] contains the following partial scores:
   -------------------------
   
   * HCP, which is 4-3-2-1 points.

   * AHCP, which is 4-3-2-1 points except 4-2-1-0 for stiff honors.

   * CCCC, which is the Kaplan-Rubens 4 C's score multiplied by 20
     in order to get integers (suit component only).

   * ZAR, Zar points (ZP).  The suit-related part, which is counted
     here, is HCP + CONTROLS, except that stiff honors are counted
     as 5-2-1-0.  The distribution-related part is separate.

   * FL, which is the German "Figuren-Länge-Punkte" (HCP and length
     points together; used before a fit it found).

   * CONTROLS, so AKJ52 has 2+1 = 3 controls.

   * PLAY_TRICKS, playing tricks multiplied by 2 to get integers.

   * QUICK_TRICKS, quick tricks multiplied by 2 to get integers.

   * LOSERS, losers.

   * TOPS1..5 (cumulative), so AKJ52 has (1, 2, 2, 3, 3).
   
   * LENGTH

   * EFF_LENGTH, an attempt to measure the "effective" length of a suit
     by its quality, rather than only the number of cards.  Probably
     useful for preempts.
     Adjustments only apply to lengths 1, 4, 5, 6 and 7.
     1: Stiff A/K counts as length 2.
     4: No HCP counts as 3, SPOT_SUM >= 37 counts as 5.
     5: SPOT_SUM < 32 counts as 4, 
        32..37 as 5, 
        38..42 as 5.5,
        >= 43 as 6.
     6: SPOT_SUM3 < 30 counts as 5,
        30..32 as 6,
        >= 33 as 7.
     7: SPOT_SUM3 < 30 counts as 6,
        at least 2 of 3 tops counts as 7.5.

   * SPOT_SUM, so AK52 = 14 + 13 + 4 + 1 = 32.

   * SPOT_SUM3, which is the same for the (up to) three highest cards.

   DIST_SCORE_TABLE[distIndex] contains the following partial scores:
   ---------------------------

   * MDIFF, number of spades minus number of hearts (can be negative).

   * MABSDIFF, the absolute value of LEN_SH.

   * MM, number of spades and hearts concatenated to an integer, e.g. 45.

   * MMAX, the larger of the number of spades and hearts.

   * MMIN, the smaller of the number of spades and hearts.
   
   * mMAX, the larger of the number of diamonds and clubs.
   
   * mMIN, the smaller of the number of diamonds and clubs.
   
   * MPROD, the product of the number of spades and hearts.

   * PROD, the product of the four suit lengths.

   * L1..L4, the length of the longest..shortest suit.

   * VOID, 1 if there is at least a void, 0 otherwise.

   * LONGEST1, LONGEST2: The DDS index of the longest and second-longest
     suits (the highest-ranking first if they are of equal length).

   * ZAR, the contribution to Zar points from the distribution.
     2*L1 + L2 - L4, but +1 for 4-3-3-3.

   distIndex is derived from a tuple of length via a map.

   compScore contains composite scores combining holding and distribution:
   ---------

   * HCP,
   * AHCP,
   * CCCC,
   * ZAR,
   * FL,
   * CONTROLS,
   * PLAY_TRICKS,
   * QUICK_TRICKS,
   * LOSERS,
   * TOPS1..5: Similar to the SUIT_SCORE versions, added up.

   * EFF_MDIFF,
   * EFF_MABSDIFF,
   * EFF_MMAX,
   * EFF_MMIN,
   * EFF_mMAX,
   * EFF_mMIN,
   * EFF_L1,
   * EFF_L2,
   * EFF_L3,
   * EFF_L4: Similar to the DIST_SCORE versions, but based on the
     effective lengths.

   * MCONC, the percentage (in the range 0..100) of the HCP concentrated
     in the Majors.

   * TWOCONC, the percentage (in the range 0..100) of the HCP 
     concentrated in the two longest suits.

   * SHORTCONC, the number of HCP in the shortest suit that is not
     a void.  If there are two equal-length shortest suits, this 
     parameter is not meaningful.

   * OUTTOPS1..5: Similar to TOPS1..5 in SUIT_SCORE, but outside of
     the longest suit.

   * BAL, 0 if the hand is considered balanced, otherwise the "distance"
     from being balanced (see code, completely heuristic).

   * UNBAL, 0 if the hand is considered unbalanced (i.e. if BAL > 0),
     otherwise the "distance" from being unbalanced.

   * SBAL, 0 if the hand is considered semi-balanced, other the "distance"
     as above.  Always 0 if BAL is 0.  SBAL also includes 5431 with a
     stiff ace or king (except if 5-4 in Majors), 5-4-2-2 (except if
     5-4 in Majors), and 6-3-2-2 (except if 2-2 in Ms or 6cM).
   
   * UNSBAL, 0 if the hand is not semi-balanced, otherwise the "distance"
     as above.
 */



// It's an enum abuse that the first parameters have the same
// values in ValSuitParams and CompositeParams...

enum ValSuitParams
{
  VS_HCP = 0,
  VS_AHCP = 1,
  VS_CCCC = 2,
  VS_ZAR = 3,
  VS_FL = 4,

  VS_CONTROLS = 5,
  VS_PLAY_TRICKS = 6,
  VS_QUICK_TRICKS = 7,
  VS_LOSERS = 8,

  VS_TOP1 = 9,
  VS_TOP2 = 10,
  VS_TOP3 = 11,
  VS_TOP4 = 12,
  VS_TOP5 = 13,

  VS_LENGTH = 14,
  VS_EFF_LENGTH = 15,

  VS_SPOT_SUM = 16,
  VS_SPOT_SUM3 = 17,

  VS_SIZE = 18
};

typedef array<int, VS_SIZE> SuitListArray;
typedef array<int, BRIDGE_TRICKS> CardArray;

enum ValDistParams
{
  VD_MDIFF = 0,
  VD_MABSDIFF = 1,
  VD_MMAX = 2,
  VD_MMIN = 3,
  VD_mMAX = 4,
  VD_mMIN = 5,
  VD_MPROD = 6,
  VD_PROD = 7,

  VD_L1 = 8,
  VD_L2 = 9,
  VD_L3 = 10,
  VD_L4 = 11,
  VD_VOID = 12,
  VD_LONGEST1 = 13,
  VD_LONGEST2 = 14,
  VD_SHORTEST = 15,

  VD_BAL = 16,
  VD_SBAL = 17,

  VD_ZAR = 18,
  VD_SIZE = 19
};

typedef array<int, VD_SIZE> DistListArray;

enum CompositeParams
{
  VC_HCP = 0,
  VC_AHCP = 1,
  VC_CCCC = 2,
  VC_ZAR = 3,
  VC_FL = 4,

  VC_CONTROLS = 5,
  VC_PLAY_TRICKS = 6,
  VC_QUICK_TRICKS = 7,
  VC_LOSERS = 8,

  VC_OUTTOPS1 = 9,
  VC_OUTTOPS2 = 10,
  VC_OUTTOPS3 = 11,
  VC_OUTTOPS4 = 12,
  VC_OUTTOPS5 = 13,

  VC_BAL = 14,
  VC_UNBAL = 15,
  VC_SBAL = 16,
  VC_UNSBAL = 17,

  VC_EFF_MDIFF = 18,
  VC_EFF_MABSDIFF = 19,
  VC_EFF_MMAX = 20,
  VC_EFF_MMIN = 21,
  VC_EFF_mMAX = 22,
  VC_EFF_mMIN = 23,

  VC_EFF_L1 = 24,
  VC_EFF_L2 = 25,
  VC_EFF_L3 = 26,
  VC_EFF_L4 = 27,

  VC_MCONC = 28,
  VC_TWOCONC = 29,

  VC_HCP_SHORTEST = 30,
  VC_HCP_LONGEST = 31,
  VC_HCP_LONG12 = 32,
  VC_SPADES = 33,

  VC_SIZE = 34
};

typedef array<int, VC_SIZE> CompositeArray;


class Valuation
{
  private:

    bool setFlag;
    bool detailFlag;

    array<SuitListArray *, BRIDGE_PLAYERS> suitValues;
    DistListArray * distValues;
    CompositeArray compValues;


    void setSuitTables();

    void setSuitLength(
      SuitListArray& list,
      CardArray& cards,
      const unsigned holding);

    void setSuitHCP(
      SuitListArray& list,
      const CardArray& cards);

    void setSuitTops(
      SuitListArray& list,
      const CardArray& cards);

    void setSuitCCCC(
      SuitListArray& list,
      const CardArray& cards);

    void setSuitZar(
      SuitListArray& list,
      const CardArray& cards);

    void setSuitFL(SuitListArray& list);

    void setSuitPlayTricks(
      SuitListArray& list,
      const CardArray& cards);

    void setSuitQuickTricks(
      SuitListArray& list,
      const CardArray& cards);

    void setSuitLosers(
      SuitListArray& list,
      const CardArray& cards);

    void setSuitEffLength(SuitListArray& list);

    void setDistTables();

    void lookup(const unsigned cards[]);

    void calcDetails();

    void setCompBalanced();

    string strHeader(const string& text) const;
    string strEntry(
      const int value,
      const int scale) const;

  public:

    Valuation();

    void reset();

    void setTables();

    void evaluate(
      const unsigned cards[],
      const bool fullFlag = false);

    int distance(const Term& term) const;

    unsigned getCompositeParam(const CompositeParams cparam) const;

    string str() const;

    int handDist() const;

    void getLengths(vector<unsigned>& lenghths) const;

    bool distMatch(const DistMatcher& distMatcher) const;
};

#endif
