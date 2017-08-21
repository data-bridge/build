/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_PLAYTRACE_H
#define BRIDGE_PLAYTRACE_H

#include <vector>
#include <list>

#include "PlayScore.h"
#include "bconst.h"

using namespace std;


struct PlayErrorRecord
{
  PlayError error;
  Player player;
  unsigned trickNo; // Starting from 1
  unsigned posNo; // Starting from 1
  unsigned was;
  unsigned is;
};

class PlayTrace
{
  private:

    unsigned len;
    vector<unsigned> tricks;
    vector<Player> playedBy;
    list<PlayErrorRecord> playErrors;
    bool playErrorSet;

    unsigned hexchar2unsigned(const char c) const;
    char unsigned2hexchar(const unsigned u) const;

    void setTricks(const string& st);

    void setPlayedBy(const vector<Player>& playedByIn);

    void fillError(
      PlayErrorRecord& per,
      const unsigned i,
      const PlayError e);

    void deriveErrors();
    void deriveErrors(const vector<PlayError>& playedByIn);

  public:

    PlayTrace();

    ~PlayTrace();

    void reset();

    list<PlayErrorRecord>::const_iterator begin() const
      { return playErrors.begin(); }
    list<PlayErrorRecord>::const_iterator end() const
      { return playErrors.end(); }

    void set(
      const int number,
      const int * tricksIn,
      const vector<Player>& playedByIn);

    void set(
      const int number,
      const int * tricksIn,
      const vector<Player>& playedByIn,
      const vector<PlayError>& playErrorIn);

    void set(
      const string& strCompact,
      const vector<Player>& playedByIn);

    void set(
      const string& strCompact,
      const vector<Player>& playedByIn,
      const vector<PlayError>& playErrorIn);

    bool operator == (const PlayTrace& pt2) const;

    bool operator != (const PlayTrace& pt2) const;

    string str() const;
    string strCompact() const;
};

#endif

