/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TEAM_H
#define BRIDGE_TEAM_H

#include <string>

enum Format: unsigned;

using namespace std;

class Teams;


enum Carry
{
  BRIDGE_CARRY_NONE = 0,
  BRIDGE_CARRY_INT = 1,
  BRIDGE_CARRY_FLOAT = 2
};


class Team
{
  private:

    string name;
    Carry carry;
    int carryi;
    float carryf;


    void extractCarry(const string& text);

    void setPBN(const string& text);
    void setTXT(const string& text);

    string strCarry(const bool forceFlag = false) const;

    string strLIN() const;
    string strPBN(const string& label) const;
    string strTXT() const;
    string strTXT(const int score) const;
    string strEML() const; // Second TXT format


  public:

    friend Teams;

    Team();

    void reset();

    void set(
      const string& text,
      const Format format);
      
    void setPair(
      const string& text,
      const string& value);
      
    bool hasCarry() const;

    unsigned getCarry() const;
      
    bool operator == (const Team& t2) const;

    bool operator != (const Team& t2) const;

    string str(
      const Format format,
      const string& label = "") const;

    string str(
      const Format format,
      const int score) const;
};

#endif

