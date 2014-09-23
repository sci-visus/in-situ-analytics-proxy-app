/*
 * ParallelFlow.cpp
 *
 *  Created on: July 16, 2012
 *      Author: landge1
 */

#include <algorithm>
#include <iostream>
#include "DistributedDefinitions.h"
#include "ParallelFlow.h"

ParallelFlow::ParallelFlow(uint32_t base_count, uint32_t factor, uint32_t rank) :
  mFactor(factor) {
  mFactor = factor;
  mNumLeafElements = base_count;
  mRank = rank;
}

std::vector<uint32_t> ParallelFlow::children(uint32_t id) const {
  std::vector<uint32_t> result;
  uint32_t k = mFactor;
  uint32_t child;

  if (mRank % mFactor == 0) {
    for (int i = 1; i < mFactor; i++)
      result.push_back(mRank + i);
  }

  while (mRank + k < mNumLeafElements) {
    if (mRank % k == 0 && mRank != k) {
      child = mRank + k;
      //std::cout << "Child: " << mRank+k << " RANK: " << mRank << std::endl;
      result.push_back(child);
    }
    k = k * mFactor;
  }

  return result;
}

std::vector<uint32_t> ParallelFlow::parent(uint32_t id) const {
  std::vector<uint32_t> result;
  uint32_t parent = 0;
  uint32_t k=1;

  if (id == 0) {
    result.push_back(parent);
    return result;
  }

  while (k < mNumLeafElements) {
    k = k * mFactor;
  }

  k = k / mFactor;

  if (id % mFactor != 0) {
    parent = id - (id % mFactor);
  } else {
    while (k > 1) {
      if (id % k == 0) {
        parent = id - k;
        break;
      }
      k = k / mFactor;
    }
  }
  result.push_back(parent);
  return result;
}

uint32_t ParallelFlow::treeLevel(uint32_t id) const {
  return 1;
}
