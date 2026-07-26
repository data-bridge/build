/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_LEADTRIPLE_H
#define BRIDGE_LEADTRIPLE_H


struct LeadTriple
{
  int suit;
  int rank;
  int score;

  bool operator==(const LeadTriple& lt2) const
  {
    return (suit == lt2.suit &&
        rank == lt2.rank &&
        score == lt2.score);
  };
};

#endif
