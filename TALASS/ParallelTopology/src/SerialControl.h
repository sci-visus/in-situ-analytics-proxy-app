/*
 * SerialControl.h
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#ifndef SERIALCONTROL_H_
#define SERIALCONTROL_H_

#include <vector>
#include "ControlFlow.h"
#include "TopoControl.h"
#include "LocalComputeAlgorithm.h"
#include "MergeTreeBuilder.h"
#include "MergeTreeBuilder.h"
#include "GatherAlgorithm.h"
#include "ScatterAlgorithm.h"
#include "SerialInputStream.h"
#include "SerialOutputStream.h"
#include "SerialCommunicator.h"
#include "Patch.h"

class MergeTree;

//! The controller for a single threaded computation
class SerialControl : public TopoControl
{
public:
  //! Typedef to satisfy the compiler
  typedef FlexArray::BlockedArray<TreeNode,LocalIndexType> BlockedArray;

  //! Default constructor
  SerialControl(bool invert = false);

  //! Destructor
  ~SerialControl();

  //! Set the local compute algorithm
  void localAlgorithm(LocalAlgorithmType local);

  //! Set the gather algorithm
  void gatherAlgorithm(GatherAlgorithmType gather);

  //! Set the scatter algorithm
  void scatterAlgorithm(ScatterAlgorithmType scatter);

  //! Set the gather flow
  void gatherFlow(ControlFlow* gather) {mGatherFlow = gather;}

  //! Set the scatter algorithm
  void scatterFlow(ControlFlow* scatter) {mScatterFlow = scatter;}

  //! Initialize the computation with the given patches
  int initialize(const std::vector<Patch*>& patches, uint8_t block_bits=BlockedArray::sBlockBits);

  //! Compute the tree, segmentation, and statistics
  int compute(FunctionType low_threshold=-10e34, FunctionType high_threshold=10e34);

  //! Dump the results
  int save();

private:

  //! Are we computing merge or split trees
  bool mInvert;

  //! The local compute algorithm
  LocalComputeAlgorithm* mLocalAlgorithm;

  //! The gather algorithm
  GatherAlgorithm* mGatherAlgorithm;

  //! The scatter algorithm
  ScatterAlgorithm* mScatterAlgorithm;

  //! The gather control flow
  ControlFlow* mGatherFlow;

  //! The scatter control flow
  ControlFlow* mScatterFlow;

  //! A set of input streams for all trees
  std::vector<SerialInputStream*> mInputStreams;

  //! A set of output streams for all trees
  std::vector<SerialOutputStream*> mOutputStreams;

  //! The single communicator
  SerialCommunicator mCommunicator;

  //! The list of references to patches
  std::vector<Patch*> mPatches;

  //! The list of local trees
  std::vector<MergeTree*> mTrees;
};


#endif /* SERIALCONTROL_H_ */
