/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_OPENINGS_H
#define BRIDGE_OPENINGS_H

using namespace std;


enum PassParams
{
  // Effectively a subset of CompositeParams, but more compactly
  // numbered for memory efficiency.

  // TODO Currently adding one at the end that is not really spades,
  // but instead the table where we log the counts of each row (line)
  // in the manually curated table of pass probabilities and criteria.

  PASS_HCP = 0,
  PASS_SPADES = 1,
  PASS_CCCC_LIGHT = 2,
  PASS_QTRICKS = 3,
  PASS_CONTROLS = 4,
  PASS_HCP_SHORTEST = 5,
  PASS_HCP_LONGEST = 6,
  PASS_HCP_LONG12 = 7,
  PASS_SIZE = 8
};


enum Openings: unsigned
{
  OPENING_PASS = 0,
  OPENING_NOT_WEAK = 1,
  OPENING_1C_PRECISION = 2,
  OPENING_1C_POLISH = 3,
  OPENING_1C_NATURAL = 4,
  OPENING_1D_PRECISION = 5,
  OPENING_1D_NATURAL = 6,
  OPENING_1H_FIVE = 7,
  OPENING_1H_FOUR = 8,
  OPENING_1S_FIVE = 9,
  OPENING_1S_FOUR = 10,
  OPENING_1NT_1517 = 11,
  OPENING_1NT_1416 = 12,
  OPENING_1NT_1315 = 13,
  OPENING_1NT_1214 = 14,
  OPENING_1NT_1113 = 15,
  OPENING_1NT_1012 = 16,
  OPENING_1NT_911 = 17,
  OPENING_2C_STRONG = 18,
  OPENING_2C_MAJS_WEAK = 19,
  OPENING_2C_D_WEAK = 20,
  OPENING_2C_BAL = 21,
  OPENING_2D_WEAK = 22,
  OPENING_2D_MAJS_WEAK = 23,
  OPENING_2D_STRONG = 24,
  OPENING_2D_MULTI_SPADES = 25,
  OPENING_2D_MULTI_HEARTS = 26,
  OPENING_2D_MULTI_STRONG = 27,

  OPENING_2H_STRONG_HEARTS = 28,
  OPENING_2H_STRONG_SPADES = 29,
  OPENING_2H_STRONG_BAL = 30,
  OPENING_2H_STRONG_THREE_SUITER = 31,
  OPENING_2H_STRONG_MISC = 32,
  OPENING_2H_WEAK_HEARTS = 33,
  OPENING_2H_WEAK_WITH_MIN = 34,
  OPENING_2H_WEAK_SPADES = 35,
  OPENING_2H_WEAK_SPADES_MIN = 36,
  OPENING_2H_WEAK_SPADES_5332 = 37,
  OPENING_2H_WEAK_WITH_SPADES = 38,
  OPENING_2H_WEAK_MINS = 39,
  OPENING_2H_WEAK_MIN = 40,
  OPENING_2H_WEAK_5332 = 41,
  OPENING_2H_WEAK_45_MIN = 42,
  OPENING_2H_WEAK_BAL = 43,
  OPENING_2H_WEAK_MISC = 44,
  OPENING_2H_INTERMED_HEARTS = 45,
  OPENING_2H_INTERMED_MAJS = 46,
  OPENING_2H_INTERMED_45_MIN = 47,
  OPENING_2H_INTERMED_MINS = 48,
  OPENING_2H_INTERMED_SPADES_MIN = 49,
  OPENING_2H_INTERMED_SPADES = 50,
  OPENING_2H_INTERMED_THREE_SUITER_SHORT_D = 51,
  OPENING_2H_INTERMED_THREE_SUITER_SHORT_H = 52,
  OPENING_2H_INTERMED_MISC = 53,

  OPENING_2S_STRONG_SPADES = 34,
  OPENING_2S_STRONG_CLUBS = 35,
  OPENING_2S_STRONG_DIAMONDS = 36,
  OPENING_2S_STRONG_HEARTS = 37,
  OPENING_2S_STRONG_THREE_SUITER = 38,
  OPENING_2S_WEAK_SPADES = 39,
  OPENING_2S_WEAK_5332 = 40,
  OPENING_2S_WEAK_WITH_MIN = 41,
  OPENING_2S_WEAK_WITH_HEARTS = 42,
  OPENING_2S_WEAK_MINS = 43,
  OPENING_2S_WEAK_MINOR = 44,
  OPENING_2S_WEAK_HEARTS = 45,
  OPENING_2S_WEAK_44 = 46,
  OPENING_2S_WEAK_45 = 47,
  OPENING_2S_WEAK_HEARTS_MIN = 48,
  OPENING_2S_WEAK_HEARTS_OTHER = 49,
  OPENING_2S_INTERMED_SPADES = 50,
  OPENING_2S_INTERMED_MIN = 51,
  OPENING_2S_INTERMED_45 = 52,
  OPENING_2S_INTERMED_MINS = 53,
  OPENING_2S_INTERMED_HEARTS = 54,
  OPENING_2S_INTERMED_HEARTS_MIN = 54,
  OPENING_2S_INTERMED_THREE_SUITER = 55,
  OPENING_2S_INTERMED_SHORT_SPADES = 56,

  OPENING_2NT_STRONG_SBAL = 58,
  OPENING_2NT_STRONG_OTHER = 59,
  OPENING_2NT_OPEN_TWO_SUITER = 60,
  OPENING_2NT_OPEN_ONE_MIN = 61,
  OPENING_2NT_OPEN_OTHER = 62,
  OPENING_2NT_WEAK_MINS = 63,
  OPENING_2NT_WEAK_ONE_MIN = 64,
  OPENING_2NT_WEAK_ONE_SUITER = 65,
  OPENING_2NT_WEAK_TWO_SUITER = 66,
  OPENING_2NT_WEAK_OTHER = 67,

  OPENING_3C_WEAK_CLUBS = 68,
  OPENING_3C_WEAK_DIAMONDS = 68,
  OPENING_3C_WEAK_MINS = 68,
  OPENING_3C_WEAK_WITH_MAJOR = 68,
  OPENING_3C_WEAK_MAJORS = 68,
  OPENING_3C_WEAK_MAJ = 68,
  OPENING_3C_STRONG_MAJORS = 68,
  OPENING_3C_STRONG_TWO_SUITER = 68,
  OPENING_3C_STRONG_THREE_SUITER = 68,
  OPENING_3C_INTERMED_CLUB_OTHER = 68,
  OPENING_3C_INTERMED_TWO_SUITER = 68,
  OPENING_3C_INTERMED_MAJORS = 68,

  OPENING_3D_WEAK_DIAMONDS = 69,
  OPENING_3D_WEAK_MAJ = 69,
  OPENING_3D_WEAK_MAJORS = 69,
  OPENING_3D_WEAK_WITH_MAJ = 69,
  OPENING_3D_STRONG_SPADES_MIN = 69,
  OPENING_3D_STRONG_REDS = 69,
  OPENING_3D_INTERMED_REDS = 69,

  OPENING_3H_WEAK_HEARTS = 70,
  OPENING_3H_WEAK_SPADES = 70,
  OPENING_3H_WEAK_MAJORS = 70,
  OPENING_3H_WEAK_MIN = 70,
  OPENING_3H_WEAK_MINORS = 70,
  OPENING_3H_STRONG_SPADES_MIN = 70,
  OPENING_3H_STRONG_SPADES = 70,

  OPENING_3S_WEAK_SPADES = 71,
  OPENING_3S_WEAK_SPADES_MIN = 71,
  OPENING_3S_WEAK_BROKEN_DIAMONDS = 71,
  OPENING_3S_WEAK_BROKEN_CLUBS = 71,
  OPENING_3S_SOLID_SPADES = 71,
  OPENING_3S_SOLID_HEARTS = 71,
  OPENING_3S_SOLID_DIAMONDS = 71,
  OPENING_3S_SOLID_CLUBS = 71,
  OPENING_3S_STRONG_SPADES = 71,
  OPENING_3S_STRONG_SPADES_OTHER = 71,

  OPENING_3NT_WEAK_SOLID_MAJOR = 72,
  OPENING_3NT_WEAK_BROKEN_MAJOR = 72,
  OPENING_3NT_WEAK_SOLID_MINOR = 72,
  OPENING_3NT_WEAK_BROKEN_MINOR = 72,
  OPENING_3NT_WEAK_AKQxxx_MAJOR = 72,
  OPENING_3NT_WEAK_AKQxxx_MINOR = 72,
  OPENING_3NT_WEAK_MINORS = 72,
  OPENING_3NT_WEAK_MAJORS = 72,

  OPENING_3NT_STRONG_SOLID_MAJOR = 72,
  OPENING_3NT_STRONG_SOLID_MINOR = 72,
  OPENING_3NT_STRONG_AKQxxx_MAJOR = 72,
  OPENING_3NT_STRONG_AKQxxx_MINOR = 72,
  OPENING_3NT_STRONG_SPADES = 72,
  OPENING_3NT_STRONG_HEARTS = 72,
  OPENING_3NT_STRONG_MAJORS = 72,
  OPENING_3NT_STRONG_MINOR = 72,
  OPENING_3NT_STRONG_BAL = 72,
  OPENING_3NT_STRONG_OTHER = 72,

  OPENING_3NT_INTERMED_SOLID_MAJOR = 72,
  OPENING_3NT_INTERMED_SOLID_MINOR = 72,
  OPENING_3NT_INTERMED_AKQxxx_MAJOR = 72,
  OPENING_3NT_INTERMED_AKQxxx_MINOR = 72,
  OPENING_3NT_INTERMED_MINOR = 72,

  OPENING_4C_WEAK_CLUBS = 72,
  OPENING_4C_WEAK_DIAMONDS = 72,
  OPENING_4C_WEAK_WITH_MAJ = 72,
  OPENING_4C_WEAK_NAMYATS = 73,
  OPENING_4C_STRONG_CLUBS = 73,
  OPENING_4C_STRONG_NAMYATS = 73,
  OPENING_4C_INTERMED_CLUBS = 73,
  OPENING_4C_INTERMED_WITH_MAJ = 73,
  OPENING_4C_INTERMED_NAMYATS = 73,

  OPENING_4D_WEAK = 74,
  OPENING_4D_NAMYATS = 75,
  OPENING_4H_WEAK = 76,
  OPENING_4S_WEAK = 77,
  OPENING_4NT_MINS = 78,
  OPENING_FIVE_PLUS = 79,
  OPENING_UNCLASSIFIED = 80,

  OPENING_SIZE = 81
};

#endif

