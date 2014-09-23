/*
 * ParallelControl.h
 *
 *  Created on: July 16, 2012
 *      Author: landge1
 */

#ifndef PARALLELCONTROL_H_
#define PARALLELCONTROL_H_

#include <vector>
#include <cstdlib>
#include "ControlFlow.h"
#include "ParallelFlow.h"
#include "TopoControl.h"
#include "LocalComputeAlgorithm.h"
#include "GatherAlgorithm.h"
#include "ScatterAlgorithm.h"
#include "ParallelInputStream.h"
#include "ParallelSortedInputStream.h"
#include "ParallelOutputStream.h"
#include "ParallelCommunicator.h"
#include "Patch.h"
#include "ModuloScatter.h"
#include "SpatialFlow.h"

class MergeTree;

//! The controller for a single threaded computation
class ParallelControl: public TopoControl {
public:
  //! Typedef to satisfy the compiler
  typedef FlexArray::BlockedArray<TreeNode, LocalIndexType> BlockedArray;

  //! Default constructor
  ParallelControl(bool invert = false);

  //! Destructor
  ~ParallelControl();

  //! Set the MPI rank
  void SetMPIrank(uint32_t rank);

  //! Set the local compute algorithm
  void localAlgorithm(LocalAlgorithmType local);

  //! Set the gather algorithm
  void gatherAlgorithm(GatherAlgorithmType gather);

  //! Set the scatter algorithm
  void scatterAlgorithm(ScatterAlgorithmType scatter);

  //! Set the gather flow
  void gatherFlow(ControlFlow* gather) {
    mGatherFlow = gather;
  }

  //! Set the scatter algorithm
  void scatterFlow(ControlFlow* scatter) {
    mScatterFlow = scatter;
  }

  //! Get GraphID for a patch
  uint64_t getGraphID(Patch* patch);

  //! Get Trees IDs to be computed by this MPI rank
  std::vector<uint32_t> getTreeIDs(RankID rank);

  //! Get Trees IDs to be computed by this MPI rank
  std::vector<uint32_t> getTreeIDs(RankID rank, uint32_t mode);

  void SetOutputFileName(const char* fileName) {
    strcpy(mOutputFileName, fileName);
  }

  //! Initialize the computation with the given patches
  int initialize(const std::vector<Patch*>& patches, uint8_t block_bits =
      BlockedArray::sBlockBits);

  int initialize(uint8_t block_bits =  BlockedArray::sBlockBits);
  //! Compute the tree, segmentation, and statistics
  int localCompute(FunctionType low_threshold=-10e34, 
                   FunctionType high_threshold=10e34);

  //! Dummy function in case of proxy app as we split the compute functionality
  //! into localCompute and joinTrees.
  int compute(FunctionType low_threshold=-10e34,
              FunctionType high_threshold=10e34) {
    return 1;
  }

  //! Join the trees and perform local fixing
  int joinTrees();

  //! Dump the results
  int save();

  //! Generate the final segmentation
  int generateSegmentation(SegmentedMergeTree& mTree);

private:

  //! Are we computing merge or split trees
  bool mInvert;

  //! MPI rank
  uint32_t mRank;

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

  //! A set of input streams for all trees being computed by this MPI rank
  std::vector<ParallelSortedInputStream*> mInputStreams;
  //std::vector<ParallelInputStream*> mInputStreams;

  //! A set of output streams for all trees being computed by this MPI rank
  std::vector<ParallelOutputStream*> mOutputStreams;

  //! An input stream for the augmented merge tree used for fixing the local tree
  //ParallelInputStream *mLocalTreeInputStream;
  ParallelSortedInputStream *mLocalTreeInputStream;

  //! An output stream for sending the augmented merge tree used for fixing the local tree
  ParallelOutputStream *mLocalTreeOutputStream;

  //! The downstream/gather communicator
  ParallelCommunicator mGatherCommunicator;

  //! The upstream/scatter communicator
  ParallelCommunicator mScatterCommunicator;

  //! The list of references to patches
  std::vector<Patch*> mPatches;

  //! The list of local trees
  std::vector<MergeTree*> mTrees;

  char mOutputFileName[64];
};

#endif /* PARALLELCONTROL_H_ */
