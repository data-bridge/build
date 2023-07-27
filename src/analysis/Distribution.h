/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DISTRIBUTION_H
#define BRIDGE_DISTRIBUTION_H

#include <map>
#include <vector>
#include <string>

using namespace std;

enum Distributions: unsigned;


class Distribution
{
  private:

    map<string, unsigned> name_to_number;

    Distributions number4(const vector<unsigned>& params) const;
    Distributions number5major(const vector<unsigned>& params) const;
    Distributions number5minor(const vector<unsigned>& params) const;
    Distributions number6major(const vector<unsigned>& params) const;
    Distributions number6minor(const vector<unsigned>& params) const;
    Distributions number7major(const vector<unsigned>& params) const;
    Distributions number7minor(const vector<unsigned>& params) const;
    Distributions number8major(const vector<unsigned>& params) const;
    Distributions number8minor(const vector<unsigned>& params) const;
    Distributions number9major(const vector<unsigned>& params) const;
    Distributions number9minor(const vector<unsigned>& params) const;


  public:

    Distribution();

    void reset();

    Distributions number(const vector<unsigned>& params) const;

    unsigned name2number(const string& name) const;
};

#endif
