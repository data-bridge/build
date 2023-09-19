/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_COMPOSITES_H
#define BRIDGE_COMPOSITES_H

#include <array>
#include <vector>
#include <string>

using namespace std;

enum CompositeParams: unsigned
{
  VC_HCP = 0,
  VC_AHCP = 1,
  VC_CCCC = 2,
  VC_CCCC_LIGHT = 3,
  VC_ZAR = 4,
  VC_FL = 5,

  VC_CONTROLS = 6,
  VC_PLAY_TRICKS = 7,
  VC_QUICK_TRICKS = 8,
  VC_LOSERS = 9,

  VC_OUTTOPS1 = 10,
  VC_OUTTOPS2 = 11,
  VC_OUTTOPS3 = 12,
  VC_OUTTOPS4 = 13,
  VC_OUTTOPS5 = 14,

  VC_BAL = 15,
  VC_UNBAL = 16,
  VC_SBAL = 17,
  VC_UNSBAL = 18,

  VC_EFF_MDIFF = 19,
  VC_EFF_MABSDIFF = 20,
  VC_EFF_MMAX = 21,
  VC_EFF_MMIN = 22,
  VC_EFF_mMAX = 23,
  VC_EFF_mMIN = 24,

  VC_EFF_L1 = 25,
  VC_EFF_L2 = 26,
  VC_EFF_L3 = 27,
  VC_EFF_L4 = 28,

  VC_MCONC = 28,
  VC_TWOCONC = 30,

  VC_HCP_SHORTEST = 31,
  VC_HCP_LONGEST = 32,
  VC_HCP_LONG12 = 33,
  VC_SPADES = 34,

  VC_SIZE = 35
};

struct CompBundle
{
  string name;
  string text;
  string textShort;
  int scale;
  int distance;
};

const vector<CompBundle> CompInfo =
{
  {"HCP", "HCP", "HCP", 1, 20},
  {"AHCP", "Adjusted HCP", "AHCP", 1, 20},
  {"CCCC", "CCCC points", "CCCC", 20, 1},
  {"CCCC_LIGHT", "CCCC light", "CCCClight", 4, 1},
  {"ZAR", "Zar points", "ZP", 1, 10},
  {"FL", "FL points", "FL", 1, 20},

  {"CONTROLS", "Controls", "controls", 1, 60},
  {"PLAY_TRICKS", "Play tricks", "Playtricks", 2, 40},
  {"QUICK_TRICKS", "Quick tricks", "Quicktricks", 2, 40},
  {"LOSERS", "Losers", "Losers", 2, 20},

  {"OUTTOPS1", "Outside tops1", "Outtops1", 1, 60},
  {"OUTTOPS2", "Outside tops2", "Outtops2", 1, 60},
  {"OUTTOPS3", "Outside tops3", "Outtops3", 1, 60},
  {"OUTTOPS4", "Outside tops4", "Outtops4", 1, 40},
  {"OUTTOPS5", "Outside tops5", "Outtops5", 1, 40},

  {"BAL", "Balanced", "BAL", 1, 1},
  {"UNBAL", "Unbalanced", "UNBAL", 1, 1},
  {"SBAL", "Semi-balanced", "SBAL", 1, 1},
  {"UNSBAL", "Not semi-BAL", "notSBAL", 1, 1},

  {"EFF_MDIFF", "Eff. S-H", "EffSH", 2, 40},
  {"EFF_MABSDIFF", "Eff. abs(S-H)", "EffSHdiff", 2, 40},
  {"EFF_MMAX", "Eff. max(S,H)", "EffSHmax", 2, 40},
  {"EFF_MMIN", "Eff. min(S,H)", "EffSHmin", 2, 40},
  {"EFF_mMAX", "Eff. max(D,C)", "EffDCmax", 2, 40},
  {"EFF_mMIN", "Eff. min(D,C)", "EffDCmin", 2, 40},

  {"EFF_L1", "Eff. 1st len", "Eff1", 2, 40},
  {"EFF_L2", "Eff. 2nd len", "Eff2", 2, 40},
  {"EFF_L3", "Eff. 3rd len", "Eff3", 2, 40},
  {"EFF_L4", "Eff. 4th len", "Eff4", 2, 40},

  {"MCONC", "Major conc.", "Mconc", 1, 1},
  {"TWOCONC", "Top-2 conc.", "Top2conc", 1, 1},

  {"HCP_SHORTEST", "Short HCP", "ShortHCP", 1, 1},
  {"HCP_LONGEST", "Long HCP", "LongHCP", 1, 1},
  {"HCP_LONG12", "Long12 HCP", "Long12HCP", 1, 1},
  {"SPADES", "Spades", "spades", 1, 1}
};

typedef array<int, VC_SIZE> CompositeArray;

#endif
