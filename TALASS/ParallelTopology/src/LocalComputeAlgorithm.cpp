/*
 * LocalComputeAlgorithm.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "LocalComputeAlgorithm.h"

  //! A function value higher than anything reasonable
const FunctionType LocalComputeAlgorithm::sMaxFunction = 10e34;

  //! A function value lower than anything reasonable
const FunctionType LocalComputeAlgorithm::sMinFunction = -10e34;


LocalComputeAlgorithm* LocalComputeAlgorithm::make(LocalAlgorithmType type, bool invert)
{
  switch (type) {
    case LOCAL_SORTED_UF:
      return new LocalSortedUF(invert);
	case LOCAL_DENSE_MERGE_TREE_BUILDER:
		return new DenseMergeTreeBuilder(invert);
  }

  return NULL;
}


