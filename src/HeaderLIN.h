/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_HEADERLIN_H
#define BRIDGE_HEADERLIN_H

#include <string>
#include "bconst.h"

using namespace std;


class HeaderLIN
{
  private:

    vector<LINData> LINdata;
    unsigned len;
    bool playersListFlag;


    bool isShortPass(const string& st) const;

    string getEffectivePlayer(
      const size_t start,
      const size_t offset,
      const size_t step,
      const vector<string>& token) const;

    void checkPlayersTrailing(
      const size_t first,
      const size_t lastIncl,
      const vector<string>& token) const;

    string strContractsCore(const Format format) const;

    string strPlayersLIN() const;

    unsigned getIntBoardNo(const unsigned extNo) const;


  public:

    HeaderLIN();

    ~HeaderLIN();

    void reset();

    void setResultsList(
      const string& text,
      const Format format);

    void setPlayersList(
      const string& text,
      const string& scoring,
      const Format format);

    void setPlayersHeader(
      const string& text,
      const string& scoring,
      const Format format);

    void setScoresList(
      const string& text,
      const string& scoring,
      const Format format);

    void setBoardsList(
      const string& text,
      const Format format);

    LINData const * getEntry(const unsigned intNo) const;

    bool isSet() const;

    bool hasPlayerList() const;

    string strPlayers(
      const unsigned intNo,
      const unsigned no) const;

    string strBoard(const unsigned intNo) const;
    string strContracts(const unsigned intNo) const;
    string strContractsList() const;
};

#endif
