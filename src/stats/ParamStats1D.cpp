/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <assert.h>

#include "ParamStats1D.h"


ParamStats1D::ParamStats1D()
{
  ParamStats1D::reset();
}


void ParamStats1D::reset()
{
  paramStats1D.clear();
  dimNames.clear();
}


void ParamStats1D::init(
  const unsigned dim1,
  const unsigned dim2,
  const unsigned dim3,
  const vector<string>& dimNamesIn,
  const vector<StatsInfo>& dimData)
{
  dimNames = dimNamesIn;

  paramStats1D.resize(dim1);
  for (size_t d1 = 0; d1 < dim1; d1++)
  {
    paramStats1D[d1].resize(dim2);
    for (size_t d2 = 0; d2 < dim2; d2++)
    {
      paramStats1D[d1][d2].resize(dim3);
      for (size_t d3 = 0; d3 < dim3; d3++)
        paramStats1D[d1][d2][d3].set(dimData[d3]);
    }
  }
}


void ParamStats1D::add(
  const unsigned d1,
  const unsigned d2,
  const vector<unsigned>& params,
  const bool flag)
{
  assert(d1 < paramStats1D.size());
  assert(d2 < paramStats1D[d1].size());

  for (unsigned par = 0; par < params.size(); par++)
    paramStats1D[d1][d2][par].add(params[par], flag);
}


void ParamStats1D::operator += (const ParamStats1D& ps2)
{
  assert(paramStats1D.size() == ps2.paramStats1D.size());
  assert(paramStats1D[0].size() == ps2.paramStats1D[0].size());
  assert(paramStats1D[0][0].size() == ps2.paramStats1D[0][0].size());

  for (size_t d1 = 0; d1 < paramStats1D.size(); d1++)
    for (size_t d2 = 0; d2 < paramStats1D[d1].size(); d2++)
      for (size_t d3 = 0; d3 < paramStats1D[d1][d2].size(); d3++)
        paramStats1D[d1][d2][d3] += ps2.paramStats1D[d1][d2][d3];
}


string ParamStats1D::str() const
{
  stringstream ss;

  for (size_t d1 = 0; d1 < paramStats1D.size(); d1++)
  {
    for (size_t d2 = 0; d2 < paramStats1D[d1].size(); d2++)
    {
      ss << dimNames[0] << d1 << "\n";
      ss << dimNames[1] << d2 << "\n";
      ss << string(18, '-') << "\n\n";

      for (size_t d3 = 0; d3 < paramStats1D[d1][d2].size(); d3++)
        ss << paramStats1D[d1][d2][d3].str() << "\n";
    }
  }

  return ss.str();
}

