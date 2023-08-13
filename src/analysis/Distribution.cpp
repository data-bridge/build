/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <algorithm>
#include <cassert>

#include "Distribution.h"
#include "Distributions.h"

enum DPARAMS
{
  DPAR_MAX_LEN = 0,
  DPAR_MIN_LEN = 1,
  DPAR_MAX_MAJOR = 2,
  DPAR_MIN_MAJOR = 3,
  DPAR_MAX_MINOR = 4,
  DPAR_MIN_MINOR = 5,
  DPAR_SIZE = 6
};


Distribution::Distribution()
{
  Distribution::reset();
}


void Distribution::reset()
{
  assert(DISTRIBUTION_NAMES.size() == DIST_SIZE);
  name_to_number.clear();

  for (unsigned i = 0; i < DIST_SIZE; i++)
    name_to_number[DISTRIBUTION_NAMES[i]] = i;
}


Distributions Distribution::number4(const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 1)
    return DIST_4441;
  else if (params[DPAR_MIN_LEN] == 2)
    return DIST_4432;
  else if (params[DPAR_MAX_MAJOR] == 4)
    return DIST_4MAJ333;
  else
    return DIST_4MIN333;
}

  
Distributions Distribution::number5major(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 0)
  {
    if (params[DPAR_MIN_MAJOR] == 5)
      return DIST_55MAJ30;
    else if (params[DPAR_MIN_MAJOR] == 4)
      return DIST_54MAJ40;
    else if (params[DPAR_MIN_MAJOR] == 3)
      return DIST_53MAJ50;
    else if (params[DPAR_MAX_MINOR] == 5)
      return DIST_50MAJ53;
    else if (params[DPAR_MAX_MINOR] == 4)
      return DIST_50MAJ44;
    else
    {
      assert(false);
      return DIST_SIZE;
    }
  }
  else if (params[DPAR_MIN_LEN] == 1)
  {
    if (params[DPAR_MIN_MAJOR] == 5)
      return DIST_55MAJ21;
    else if (params[DPAR_MAX_MINOR] == 5)
      return DIST_5MAJ5MIN21;
    else if (params[DPAR_MIN_MAJOR] == 4)
      return DIST_54MAJ31;
    else
      return DIST_5MAJ4MIN31;
  }
  else if (params[DPAR_MIN_LEN] == 2)
  {
    if (params[DPAR_MAX_MINOR] == 2)
      return DIST_54MAJ22;
    else if (params[DPAR_MAX_MINOR] == 4)
      return DIST_5MAJ4MIN22;
    else
      return DIST_5MAJ332;
  }
  else
  {
    assert(false);
    return DIST_SIZE;
  }
}


Distributions Distribution::number5minor(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 0)
  {
    if (params[DPAR_MIN_MAJOR] == 4)
      return DIST_44MAJ50;
    else if (params[DPAR_MIN_MINOR] == 5)
      return DIST_55MIN30;
    else if (params[DPAR_MIN_MINOR] == 4)
      return DIST_40MAJ54;
    else
    {
      assert(false);
      return DIST_SIZE;
    }
  }
  else if (params[DPAR_MIN_LEN] == 1)
  {
    if (params[DPAR_MIN_MINOR] == 5)
      return DIST_55MIN21;
    else if (params[DPAR_MAX_MAJOR] == 4)
      return DIST_4MAJ5MIN31;
    else
      return DIST_54MIN31;
  }
  else if (params[DPAR_MIN_LEN] == 2)
  {
    if (params[DPAR_MAX_MAJOR] == 4)
      return DIST_4MAJ5MIN22;
    else if (params[DPAR_MIN_MINOR] == 4)
      return DIST_54MIN22;
    else
      return DIST_5MIN332;
  }
  else
  {
    assert(false);
    return DIST_SIZE;
  }
}

  
Distributions Distribution::number6major(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 2)
    return DIST_6MAJ322;
  else if (params[DPAR_MIN_LEN] == 1)
  {
    if (params[DPAR_MAX_MINOR] == 5)
      return DIST_61MAJ51;
    else if (params[DPAR_MAX_MINOR] == 3)
      return DIST_6MAJ331;
    else if (params[DPAR_MAX_MINOR] == 1)
      return DIST_65MAJ11;
    else
      return DIST_6MAJ421;
  }
  else if (params[DPAR_MAX_MINOR] == 6)
    return DIST_6MAJ6MIN10;
  else if (params[DPAR_MAX_MINOR] == 5)
    return DIST_6MAJ5MIN20;
  else if (params[DPAR_MAX_MINOR] == 2)
    return DIST_65MAJ20;
  else if (params[DPAR_MAX_MINOR] == 1)
    return DIST_66MAJ10;
  else
    return DIST_6MAJ430;
}


Distributions Distribution::number6minor(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 2)
    return DIST_6MIN322;
  else if (params[DPAR_MIN_LEN] == 1)
  {
    if (params[DPAR_MAX_MAJOR] == 5)
      return DIST_51MAJ61;
    else if (params[DPAR_MAX_MAJOR] == 1)
      return DIST_65MIN11;
    else if (params[DPAR_MAX_MAJOR] == 3)
      return DIST_6MIN331;
    else
      return DIST_6MIN421;
  }
  else if (params[DPAR_MAX_MAJOR] == 5)
    return DIST_5MAJ6MIN20;
  else if (params[DPAR_MAX_MAJOR] == 2)
    return DIST_65MIN20;
  else if (params[DPAR_MAX_MAJOR] == 1)
    return DIST_66MIN10;
  else
    return DIST_6MIN430;
}


Distributions Distribution::number7major(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 2)
    return DIST_7MAJ222;
  else if (params[DPAR_MIN_LEN] == 1)
  {
    if (params[DPAR_MAX_MINOR] == 4)
      return DIST_71MAJ41;
    else if (params[DPAR_MAX_MINOR] == 1)
      return DIST_74MAJ11;
    else
      return DIST_7MAJ321;
  }
  else if (params[DPAR_MAX_MINOR] == 6)
    return DIST_7MAJ6MIN;
  else if (params[DPAR_MAX_MINOR] == 5)
    return DIST_7MAJ5MIN10;
  else if (params[DPAR_MAX_MINOR] == 4)
    return DIST_7MAJ4MIN20;
  else if (params[DPAR_MAX_MINOR] == 3)
    return DIST_7MAJ330;
  else if (params[DPAR_MAX_MINOR] == 2)
    return DIST_74MAJ20;
  else if (params[DPAR_MAX_MINOR] == 1)
    return DIST_75MAJ10;
  else
    return DIST_76MAJ00;
}


Distributions Distribution::number7minor(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 2)
    return DIST_7MIN222;
  else if (params[DPAR_MIN_LEN] == 1)
  {
    if (params[DPAR_MAX_MAJOR] == 4)
      return DIST_71MIN41;
    else if (params[DPAR_MAX_MAJOR] == 1)
      return DIST_74MIN11;
    else
      return DIST_7MIN321;
  }
  if (params[DPAR_MAX_MAJOR] == 6)
    return DIST_6MAJ7MIN;
  else if (params[DPAR_MAX_MAJOR] == 5)
    return DIST_5MAJ7MIN10;
  else if (params[DPAR_MAX_MAJOR] == 4)
    return DIST_4MAJ7MIN20;
  else if (params[DPAR_MAX_MAJOR] == 3)
    return DIST_7MIN330;
  else if (params[DPAR_MAX_MAJOR] == 2)
    return DIST_74MIN20;
  else if (params[DPAR_MAX_MAJOR] == 1)
    return DIST_75MIN10;
  else
    return DIST_76MIN00;
}


Distributions Distribution::number8major(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 1)
  {
    if (params[DPAR_MAX_MINOR] == 2)
      return DIST_8MAJ221;
    else
      return DIST_8MAJ311;
  }
  else if (params[DPAR_MAX_MINOR] == 5)
    return DIST_8MAJ5MIN;
  else if (params[DPAR_MAX_MINOR] == 4)
    return DIST_8MAJ4MIN10;
  else if (params[DPAR_MAX_MINOR] == 1)
    return DIST_84MAJ10;
  else if (params[DPAR_MAX_MINOR] == 0)
    return DIST_85MAJ00;
  else
    return DIST_8MAJ320;
}


Distributions Distribution::number8minor(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 1)
  {
    if (params[DPAR_MAX_MAJOR] == 2)
      return DIST_8MIN221;
    else
      return DIST_8MIN311;
  }
  else if (params[DPAR_MAX_MAJOR] == 5)
    return DIST_5MAJ8MIN;
  else if (params[DPAR_MAX_MAJOR] == 4)
    return DIST_4MAJ8MIN10;
  else if (params[DPAR_MAX_MAJOR] == 1)
    return DIST_84MIN10;
  else if (params[DPAR_MAX_MAJOR] == 0)
    return DIST_85MIN00;
  else
    return DIST_8MIN320;
}


Distributions Distribution::number9major(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 1)
    return DIST_9MAJ211;
  else if (params[DPAR_MAX_MINOR] == 4)
    return DIST_9MAJ4MIN;
  else if (params[DPAR_MAX_MINOR] == 2)
    return DIST_9MAJ220;
  else if (params[DPAR_MAX_MINOR] == 0)
    return DIST_94MAJ00;
  else
    return DIST_9MAJ310;
}


Distributions Distribution::number9minor(
  const vector<unsigned>& params) const
{
  if (params[DPAR_MIN_LEN] == 1)
    return DIST_9MIN211;
  else if (params[DPAR_MAX_MAJOR] == 4)
    return DIST_4MAJ9MIN;
  else if (params[DPAR_MAX_MAJOR] == 2)
    return DIST_9MIN220;
  else if (params[DPAR_MAX_MAJOR] == 0)
    return DIST_94MIN00;
  else
    return DIST_9MIN310;
}


Distributions Distribution::number(const vector<unsigned>& lengths) const
{
  vector<unsigned> params;
  params.resize(DPAR_SIZE);
  params[DPAR_MAX_LEN] = * max_element(lengths.begin(), lengths.end());
  params[DPAR_MIN_LEN] = * min_element(lengths.begin(), lengths.end());
  params[DPAR_MAX_MAJOR] = max(lengths[0], lengths[1]);
  params[DPAR_MIN_MAJOR] = min(lengths[0], lengths[1]);
  params[DPAR_MAX_MINOR] = max(lengths[2], lengths[3]);
  params[DPAR_MIN_MINOR] = min(lengths[2], lengths[3]);

  if (params[DPAR_MAX_LEN] == 4)
    return Distribution::number4(params);
  else if (params[DPAR_MAX_LEN] == 5)
    if (params[DPAR_MAX_MAJOR] == 5)
      return Distribution::number5major(params);
    else
      return Distribution::number5minor(params);
  else if (params[DPAR_MAX_LEN] == 6)
    if (params[DPAR_MAX_MAJOR] == 6)
      return Distribution::number6major(params);
    else
      return Distribution::number6minor(params);
  else if (params[DPAR_MAX_LEN] == 7)
    if (params[DPAR_MAX_MAJOR] == 7)
      return Distribution::number7major(params);
    else
      return Distribution::number7minor(params);
  else if (params[DPAR_MAX_LEN] == 8)
    if (params[DPAR_MAX_MAJOR] == 8)
      return Distribution::number8major(params);
    else
      return Distribution::number8minor(params);
  else if (params[DPAR_MAX_LEN] == 9)
    if (params[DPAR_MAX_MAJOR] == 9)
      return Distribution::number9major(params);
    else
      return Distribution::number9minor(params);
  else if (params[DPAR_MAX_LEN] == 10)
  {
    if (params[DPAR_MAX_MAJOR] == 10)
      if (params[DPAR_MIN_LEN] == 1)
        return DIST_10MAJ111;
      else if (params[DPAR_MAX_MINOR] == 1 || params[DPAR_MAX_MINOR] == 2)
        return DIST_10MAJ210;
      else
        return DIST_10MAJ300;
    else if (params[DPAR_MIN_LEN] == 1)
      return DIST_10MIN111;
    else if (params[DPAR_MAX_MAJOR] == 1 || params[DPAR_MAX_MAJOR] == 2)
      return DIST_10MIN210;
    else
      return DIST_10MIN300;
  }
  else if (params[DPAR_MAX_LEN] == 11)
  {
    if (params[DPAR_MAX_MAJOR] == 11)
      if (params[DPAR_MAX_MINOR] == 1)
        return DIST_11MAJ110;
      else
        return DIST_11MAJ200;
    else if (params[DPAR_MAX_MAJOR] == 1)
      return DIST_11MIN110;
    else
      return DIST_11MIN200;
  }
  else if (params[DPAR_MAX_LEN] == 12)
  {
    if (params[DPAR_MAX_MAJOR] == 12)
      return DIST_12MAJ100;
    else
      return DIST_12MIN100;
  }
  else if (params[DPAR_MAX_LEN] == 13)
  {
    if (params[DPAR_MAX_MAJOR] == 13)
      return DIST_13MAJ000;
    else
      return DIST_13MIN000;
  }
  else
  {
    assert(false);
    return DIST_SIZE;
  }
}


string Distribution::name(const vector<unsigned>& lengths) const
{
  return DISTRIBUTION_NAMES[Distribution::number(lengths)];
}


unsigned Distribution::name2number(const string& name) const
{
  auto it = name_to_number.find(name);
  if (it == name_to_number.end())
    assert(false);

  return it->second;
}

