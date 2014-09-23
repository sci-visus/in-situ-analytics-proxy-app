/*
 * GatherAlgorithm.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "GatherAlgorithm.h"
#include "StreamingGatherAlgorithm.h"
#include "ParallelGatherAlgorithm.h"

GatherAlgorithm* GatherAlgorithm::make(GatherAlgorithmType type, bool invert)
{
  switch (type) {
    case GATHER_STREAMING_SORT:
      return new StreamingGatherAlgorithm(invert);
    case GATHER_PARALLEL_SORT:
      return new ParallelGatherAlgorithm(invert);
  }

  return NULL;
}
