/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DISTRIBUTIONS_H
#define BRIDGE_DISTRIBUTIONS_H

#include <vector>
#include <string>

using namespace std;


enum Distributions: unsigned
{
  DIST_44MAJ32 = 0,
  DIST_4MAJ4MIN32 = 1,
  DIST_44MIN32 = 2,

  DIST_5MAJ332 = 3,
  DIST_5MIN332 = 4,

  DIST_54MAJ31 = 5,
  DIST_5MAJ4MIN31 = 6,
  DIST_4MAJ5MIN31 = 7,
  DIST_54MIN31 = 8,

  DIST_54MAJ22 = 9,
  DIST_5MAJ4MIN22 = 10,
  DIST_4MAJ5MIN22 = 11,
  DIST_54MIN22 = 12,

  DIST_4MAJ333 = 13,
  DIST_4MIN333 = 14,

  DIST_6MAJ322 = 15,
  DIST_6MIN322 = 16,

  DIST_64MAJ21 = 17,
  DIST_6MAJ4MIN21 = 18,
  DIST_4MAJ6MIN21 = 19,
  DIST_64MIN21 = 20,

  DIST_6MAJ331 = 21,
  DIST_6MIN331 = 22,

  DIST_55MAJ21 = 23,
  DIST_5MAJ5MIN21 = 24,
  DIST_55MIN21 = 25,

  DIST_44MAJ41 = 26,
  DIST_44MIN41 = 27,

  DIST_7MAJ321 = 28,
  DIST_7MIN321 = 29,

  DIST_64MAJ30 = 30,
  DIST_6MAJ4MIN30 = 31,
  DIST_4MAJ6MIN30 = 32,
  DIST_64MIN30 = 33,

  DIST_54MAJ40 = 34,
  DIST_50MAJ44 = 35,
  DIST_44MAJ50 = 36,
  DIST_40MAJ54 = 37,

  DIST_55MAJ30 = 38,
  DIST_53MAJ50 = 39,
  DIST_50MAJ53 = 40,
  DIST_55MIN30 = 41,

  DIST_65MAJ11 = 42,
  DIST_6MAJ5MIN11 = 43,
  DIST_5MAJ6MIN11 = 44,
  DIST_65MIN11 = 45,

  DIST_65MAJ20 = 46,
  DIST_6MAJ5MIN20 = 47,
  DIST_5MAJ6MIN20 = 48,
  DIST_65MIN20 = 49,

  DIST_7MAJ222 = 50,
  DIST_7MIN222 = 51,

  DIST_74MAJ11 = 52,
  DIST_7MAJ4MIN11 = 53,
  DIST_4MAJ7MIN11 = 54,
  DIST_74MIN11 = 55,

  DIST_74MAJ20 = 56,
  DIST_7MAJ4MIN20 = 57,
  DIST_4MAJ7MIN20 = 58,
  DIST_74MIN20 = 59,

  DIST_7MAJ330 = 60,
  DIST_7MIN330 = 61,

  DIST_8MAJ221 = 62,
  DIST_8MIN221 = 63,

  DIST_8MAJ311 = 64,
  DIST_8MIN311 = 65,

  DIST_75MAJ = 66,
  DIST_7MAJ5MIN = 67,
  DIST_5MAJ7MIN = 68,
  DIST_75MIN = 69,

  DIST_8MAJ320 = 70,
  DIST_8MIN320 = 71,

  DIST_66MAJ = 72,
  DIST_6MAJ6MIN = 73,
  DIST_66MIN = 74,

  DIST_84MAJ = 75,
  DIST_8MAJ4MIN = 76,
  DIST_4MAJ8MIN = 77,
  DIST_84MIN = 78,

  DIST_9MAJ211 = 79,
  DIST_9MIN211 = 80,

  DIST_9MAJ310 = 81,
  DIST_9MIN310 = 82,

  DIST_9MAJ220 = 83,
  DIST_9MIN220 = 84,

  DIST_76MAJ = 85,
  DIST_7MAJ6MIN = 86,
  DIST_6MAJ7MIN = 87,
  DIST_76MIN = 88,

  DIST_85MAJ = 89,
  DIST_8MAJ5MIN = 90,
  DIST_5MAJ8MIN = 91,
  DIST_85MIN = 92,

  DIST_10MAJ210 = 93,
  DIST_10MIN210 = 94,

  DIST_94MAJ = 95,
  DIST_9MAJ4MIN = 96,
  DIST_4MAJ9MIN = 97,
  DIST_94MIN = 98,

  DIST_10MAJ111 = 99,
  DIST_10MIN111 = 100,

  DIST_10MAJ300 = 101,
  DIST_10MIN300 = 102,

  DIST_11MAJ110 = 103,
  DIST_11MIN110 = 104,

  DIST_11MAJ200 = 105,
  DIST_11MIN200 = 106,

  DIST_12MAJ = 107,
  DIST_12MIN = 108,

  DIST_13MAJ = 109,
  DIST_13MIN = 110,

  DIST_SIZE = 111
};


const vector<string> DISTRIBUTION_NAMES = 
{
  "44MAJ32", "4MAJ4MIN32", "44MIN32",
  "5MAJ332", "5MIN332",
  "54MAJ31", "5MAJ4MIN31", "4MAJ5MIN31", "54MIN31",
  "54MAJ22", "5MAJ4MIN22", "4MAJ5MIN22", "54MIN22",
  "4MAJ333", "4MIN333",
  "6MAJ322", "6MIN322",
  "64MAJ21", "6MAJ4MIN21", "4MAJ6MIN21", "64MIN21",
  "6MAJ331", "6MIN331",
  "55MAJ21", "5MAJ5MIN21", "55MIN21",
  "44MAJ41", "44MIN41",
  "7MAJ321", "7MIN321",
  "64MAJ30", "6MAJ4MIN30", "4MAJ6MIN30", "64MIN30",
  "54MAJ40", "50MAJ44", "44MAJ50", "40MAJ54",
  "55MAJ30", "53MAJ50", "50MAJ53", "55MIN30",
  "65MAJ11", "6MAJ5MIN11", "5MAJ6MIN11", "65MIN11",
  "65MAJ20", "6MAJ5MIN20", "5MAJ6MIN20", "65MIN20",
  "7MAJ222", "7MIN222",
  "74MAJ11", "7MAJ4MIN11", "4MAJ7MIN11", "74MIN11",
  "74MAJ20", "7MAJ4MIN20", "4MAJ7MIN20", "74MIN20",
  "7MAJ330", "7MIN330",
  "8MAJ221", "8MIN221",
  "8MAJ311", "8MIN311",
  "75MAJ", "7MAJ5MIN", "5MAJ7MIN", "75MIN",
  "8MAJ320", "8MIN320",
  "66MAJ", "6MAJ6MIN", "66MIN",
  "84MAJ", "8MAJ4MIN", "4MAJ8MIN", "84MIN",
  "9MAJ211", "9MIN211",
  "9MAJ310", "9MIN310",
  "9MAJ220", "9MIN220",
  "76MAJ", "7MAJ6MIN", "6MAJ7MIN", "76MIN00",
  "85MAJ", "8MAJ5MIN", "5MAJ8MIN", "85MIN",
  "10MAJ210", "10MIN210",
  "94MAJ", "9MAJ4MIN", "4MAJ9MIN", "94MIN",
  "10MAJ111", "10MIN111",
  "10MAJ300", "10MIN300",
  "11MAJ110", "11MIN110",
  "11MAJ200", "11MIN200",
  "12MAJ100", "12MIN100",
  "13MAJ000", "13MIN000"
};

#endif
