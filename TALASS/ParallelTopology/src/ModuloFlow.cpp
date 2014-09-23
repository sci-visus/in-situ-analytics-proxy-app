/*
 * ModuloFlow.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include <algorithm>
#include <iostream>
#include "DistributedDefinitions.h"
#include "ModuloFlow.h"

ModuloFlow::ModuloFlow(uint32_t base_count, uint32_t factor) : mFactor(factor)
{
  mLevelCount.push_back(base_count);
  mAggregatedCount.push_back(0);

  // TODO: base count should be multiple of mFactor
  mBaseCount = base_count;

  while (mLevelCount.back() > 1) {
    uint32_t count = mLevelCount.back();

    mAggregatedCount.push_back(mAggregatedCount.back() + count);

    mLevelCount.push_back(count / mFactor);
    if (count % mFactor != 0)
      mLevelCount.back()++;
  }

  // This is needed for the search
  mAggregatedCount.push_back(mAggregatedCount.back() + mLevelCount.back());
}

std::vector<uint32_t> ModuloFlow::children(uint32_t id) const
{
  if (id < mLevelCount[0]) {
    return std::vector<uint32_t>();
    }

  uint32_t lvl = treeLevel(id);
  uint32_t k = id - this->mAggregatedCount[lvl]; // My id within the level

  std::vector<uint32_t> result;

  for (uint32_t i=k*mFactor;i<std::min(mLevelCount[lvl-1],(k+1)*mFactor);i++) {
      result.push_back(mAggregatedCount[lvl-1]+i);
  } 

  return result;
}

std::vector<uint32_t> ModuloFlow::parent(uint32_t id) const
{
  if (id >= mAggregatedCount.back()-1)
    return std::vector<uint32_t>();

  uint32_t lvl = treeLevel(id);

  uint32_t k = id - this->mAggregatedCount[lvl]; // My id within the level

  std::vector<uint32_t> result(1);

  result[0] = mAggregatedCount[lvl+1] + k/mFactor;

  return result;
}


uint32_t ModuloFlow::treeLevel(uint32_t id) const
{
  std::vector<uint32_t>::const_iterator it;

  it = std::upper_bound(mAggregatedCount.begin(),mAggregatedCount.end(),id);

  sterror(it == mAggregatedCount.end(),"Element id is too high. No such element in this control flow.");

  // it now points to the level after the one we are on
  return (it - mAggregatedCount.begin() - 1);
}


