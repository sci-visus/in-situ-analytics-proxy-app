/*
 * ScatterAlgorithm.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "ScatterAlgorithm.h"
#include "FixLocalTreeAlgorithm.h"
#include "SortedFixLocalTreeAlgorithm.h"

ScatterAlgorithm* ScatterAlgorithm::make(ScatterAlgorithmType type, bool invert)
{
  switch (type) {
    case SCATTER_FIX_LOCAL:
      return new FixLocalTreeAlgorithm(invert);
    case SCATTER_SORTED_FIX_LOCAL:
      return new SortedFixLocalTreeAlgorithm(invert);
  }

  return NULL;
}
