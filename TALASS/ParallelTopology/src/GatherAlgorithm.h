/*
 * GatherAlgorithm.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef GATHERALGORITHM_H_
#define GATHERALGORITHM_H_

#include <vector>
#include "TopoInputStream.h"
#include "SortedInputStream.h"
#include "TopoOutputStream.h"
#include "MergeTree.h"
#include "Algorithm.h"
#include "ControlFlow.h"

enum GatherAlgorithmType {
  GATHER_STREAMING_SORT = 0,
  GATHER_PARALLEL_SORT,
};


//! The API for an algorithm to combine multiple trees into one
class GatherAlgorithm : public Algorithm
{
public:

  //! The factory function
  static GatherAlgorithm* make(GatherAlgorithmType type, bool invert);

  //! Default constructor
  GatherAlgorithm(bool invert=false) : Algorithm(invert) {}

  //! Destructor
  virtual ~GatherAlgorithm() {}

  //! Apply the algorithm
  /*! Apply the gather algorithm which will take as input an arbitrary
   *  number of trees and produce a single output tree. The inputs should
   *  consist of a number of boundary trees and the output will be another
   *  boundary tree. Furthermore, all relevant information regarding the
   *  changes this merge implies to any of the upstream trees will be written
   *  to the corresponding output stream
   *
   * @param tree: The resulting union tree (should be empty at first call)
   * @param inputs: The input stream delivering tokens from all k input trees
   * @param downstream: The output stream for the resulting boundary tree
   * @param upstream: The output stream for the scatter information
   * @return 1 if successful; 0 otherwise
   */
  virtual int apply(MergeTree& tree, TopoInputStream* inputs,
                    TopoOutputStream* downstream,
                    TopoOutputStream* upstream,
                    const ControlFlow* gather_flow) = 0;

  virtual int apply(MergeTree& tree, SortedInputStream* inputs,
                    TopoOutputStream* downstream,
                    TopoOutputStream* upstream,
                    const ControlFlow* gather_flow) = 0;
};

#endif /* GATHERALGORITHM_H_ */
