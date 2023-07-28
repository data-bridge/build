/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <assert.h>

#include "ParamStats2D.h"


ParamStats2D::ParamStats2D()
{
  ParamStats2D::reset();
}


void ParamStats2D::reset()
{
  paramStats2D.clear();
  dimNames.clear();
  emptyFlag = true;
}


void ParamStats2D::init(
  const unsigned dim1,
  const unsigned dim2,
  const unsigned dim3,
  const vector<string>& dimNamesIn,
  const vector<StatsInfo>& dimData)
{
  dimNames = dimNamesIn;

  paramStats2D.resize(dim1);
  for (size_t d1 = 0; d1 < dim1; d1++)
  {
    paramStats2D[d1].resize(dim2);
    for (size_t d2 = 0; d2 < dim2; d2++)
    {
      paramStats2D[d1][d2].resize(dim3 * (dim3-1) / 2);

      size_t d_run = 0;
      for (size_t d3a = 0; d3a + 1 < dim3; d3a++)
      {
        for (size_t d3b = d3a + 1; d3b < dim3; d3b++)
        {
          paramStats2D[d1][d2][d_run].set1(dimData[d3a]);
          paramStats2D[d1][d2][d_run].set2(dimData[d3b]);

          d_run++;
        }
      }
    }
  }
}


void ParamStats2D::add(
  const unsigned d1,
  const unsigned d2,
  const vector<unsigned>& params,
  const bool flag)
{
  assert(d1 < paramStats2D.size());
  assert(d2 < paramStats2D[d1].size());

  size_t d_run = 0;
  for (unsigned par1 = 0; par1 + 1 < params.size(); par1++)
  {
    for (unsigned par2 = par1 + 1; par2 < params.size(); par2++)
    {
      paramStats2D[d1][d2][d_run].add(params[par1], params[par2], flag);
      d_run++;
    }
  }
  emptyFlag = false;
}


bool ParamStats2D::empty() const
{
  return emptyFlag;
}


void ParamStats2D::operator += (const ParamStats2D& ps2)
{
  assert(paramStats2D.size() == ps2.paramStats2D.size());
  assert(paramStats2D[0].size() == ps2.paramStats2D[0].size());
  assert(paramStats2D[0][0].size() == ps2.paramStats2D[0][0].size());

  for (size_t d1 = 0; d1 < paramStats2D.size(); d1++)
    for (size_t d2 = 0; d2 < paramStats2D[d1].size(); d2++)
      for (size_t d3 = 0; d3 < paramStats2D[d1][d2].size(); d3++)
        paramStats2D[d1][d2][d3] += ps2.paramStats2D[d1][d2][d3];
}


string ParamStats2D::str() const
{
  stringstream ss;

  for (size_t d1 = 0; d1 < paramStats2D.size(); d1++)
  {
    for (size_t d2 = 0; d2 < paramStats2D[d1].size(); d2++)
    {
      ss << left << setw(13) << dimNames[0] << ": " << d1 << "\n";
      ss << setw(13) << dimNames[1] << ": " << d2 << "\n";
      ss << string(16, '-') << "\n\n";

      size_t d_run = 0;
      for (unsigned par1 = 0; 
        par1 + 1 < paramStats2D[d1][d2].size(); par1++)
      {
        for (unsigned par2 = par1 + 1; 
          par2 < paramStats2D[d1][d2].size(); par2++)
        {
          ss << paramStats2D[d1][d2][d_run].str() << "\n";
          d_run++;
        }
      }
    }
  }

  return ss.str();
}

