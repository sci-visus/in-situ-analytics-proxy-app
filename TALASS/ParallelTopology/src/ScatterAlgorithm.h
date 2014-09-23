/*
 * ScatterAlgorithm.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef SCATTERALGORITHM_H_
#define SCATTERALGORITHM_H_

#include <vector>
#include "TopoInputStream.h"
#include "SortedInputStream.h"
#include "TopoOutputStream.h"
#include "MergeTree.h"
#include "Algorithm.h"

enum ScatterAlgorithmType {
  SCATTER_FIX_LOCAL = 0,
  SCATTER_SORTED_FIX_LOCAL = 1,
};

//! The API for an algorithm to correct a tree from a single input
class ScatterAlgorithm : public Algorithm
{
public:

  //! The factory function
  static ScatterAlgorithm* make(ScatterAlgorithmType type, bool invert);

  //! Default constructor
  ScatterAlgorithm(bool invert=false) : Algorithm(invert) {}

  //! Destructor
  virtual ~ScatterAlgorithm() {}

  virtual int apply(MergeTree& tree, TopoInputStream* input,
                    TopoOutputStream* outputs)  = 0;
  
  virtual int apply(MergeTree& tree, SortedInputStream* input,
                    TopoOutputStream* outputs)  = 0;

};

#endif /* SCATTERALGORITHM_H_ */
