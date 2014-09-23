/*
 * ParallelControl.cpp
 *
 *  Created on: July 16, 2012
 *      Author: landge1
 */

#include <iostream>
#include <cstdlib>
#include "ParallelControl.h"
#include "MergeTree.h"
#include "mpi.h"

#define MODULO  0
#define SPATIAL 1

ParallelControl::ParallelControl(bool invert) :
  mInvert(invert), mLocalAlgorithm(NULL), mGatherAlgorithm(NULL),
      mScatterAlgorithm(NULL), mGatherFlow(NULL), mScatterFlow(NULL) {
}

ParallelControl::~ParallelControl() {
  if (mLocalAlgorithm != NULL)
    delete mLocalAlgorithm;

  if (mGatherAlgorithm != NULL)
    delete mGatherAlgorithm;

  if (mScatterAlgorithm != NULL)
    delete mScatterAlgorithm;

}


void ParallelControl::SetMPIrank(uint32_t rank) {
  mRank = rank;
}

void ParallelControl::localAlgorithm(LocalAlgorithmType local) {
  mLocalAlgorithm = LocalComputeAlgorithm::make(local, mInvert);
}

void ParallelControl::gatherAlgorithm(GatherAlgorithmType gather) {
  mGatherAlgorithm = GatherAlgorithm::make(gather, mInvert);
}

void ParallelControl::scatterAlgorithm(ScatterAlgorithmType scatter) {
  mScatterAlgorithm = ScatterAlgorithm::make(scatter, mInvert);
}

uint64_t ParallelControl::getGraphID(Patch* patch) {
  // Naive implementation every rank gets only one patch and we 
  // assign it as the id
  // TODO: Get actual PatchID from somewhere
  return mRank;
}

std::vector<uint32_t> ParallelControl::getTreeIDs(RankID rank, uint32_t mode) {

  std::vector<uint32_t> treeIDs;
  
  switch(mode) {
    case MODULO: 
    {
      uint32_t numTrees;
      uint32_t factor;
      uint32_t baseCount;
      uint32_t numLevelsInFlow;

      treeIDs.push_back(rank);

      if (rank % 2 != 0)
        return treeIDs;

      numTrees = mGatherFlow->size();
      factor = mGatherFlow->factor();
      baseCount = mGatherFlow->baseCount(); 
      numLevelsInFlow = mGatherFlow->numLevels();

      uint32_t k = factor;
      std::vector<uint32_t> sink;
  
      sink.push_back(rank);
      //while (k <= baseCount) {
      for (int i=0; i<numLevelsInFlow-1; i++) {
       if (rank % k == 0) {
         sink = mGatherFlow->sinks(sink[0]);
         treeIDs.push_back(sink[0]);
       }
       else
         break;
       k = k*factor;
      }

      return treeIDs;
      break;
    }

    case SPATIAL:
    {
      treeIDs.push_back(rank);
      std::vector<uint32_t> sink;
      uint32_t tree_id = rank;
      uint32_t bounding_box_id;

      do {
        sink = mGatherFlow->sinks(tree_id);
        bounding_box_id = mGatherFlow->getBlockId(sink[0]);

        if (bounding_box_id == rank) {
          treeIDs.push_back(sink[0]);
          tree_id = sink[0];
        }
        else
          break;
      } while (sink[0] < mGatherFlow->size() - 1);
      break;
    }
    default:
    {
      std::cout << "No flow selected to determine tree IDs for MPI process\n";
    }
  }
}

int ParallelControl::initialize(const std::vector<Patch*>& patches,
    uint8_t block_bits) {
  uint32_t i, k;
  std::vector<uint32_t> sinks;
  std::vector<uint32_t> treeIDs;
  enum directionType {DOWNSTREAM, UPSTREAM};

  mPatches = patches;

  uint64_t GraphID;

  // Every MPI rank computes some trees from the ModuloFlow depending on its
  // rank. We now get the treeIDs that need to be computed by this MPI rank.
  treeIDs = getTreeIDs(mRank, SPATIAL);
  //treeIDs = getTreeIDs(mRank, MODULO);

  /*  
  std::cout << "Tree IDs for MPI rank:: " << mRank << " :: " ;
  std::vector<uint32_t>::iterator it;
  //for (it = treeIDs.begin(); it != treeIDs.end(); it++){
  for (int i =0 ; i< treeIDs.size() ; i++) {
    std::cout << treeIDs[i]<< "\t";
  }
  std::cout << "\n";
  */

  // Need to associate patch with a graphID
  // A unique ID is given to every patch
  // This will probably be guided from the indices of the patches
  GraphID = this->getGraphID(patches[0]);
  
  // Create a tree for every treeID that gets computed by this MPI rank
  for (i=0; i < treeIDs.size(); i++) 
    mTrees.push_back(new SegmentedMergeTree(treeIDs[i], block_bits));

  //std::cout << "Initializing Parallel Control Rank :: "<< mRank << std::endl;
  mGatherCommunicator.connect(mRank, mGatherFlow, treeIDs, SPATIAL);
  mGatherCommunicator.setFlow(mGatherFlow);
  mScatterCommunicator.connect(mRank, mGatherFlow, treeIDs, SPATIAL);

  //mGatherCommunicator.connect(mRank, mGatherFlow, treeIDs, MODULO);
  //mGatherCommunicator.setFlow(mGatherFlow);
  //mScatterCommunicator.connect(mRank, mGatherFlow, treeIDs, MODULO);

  for (i=0; i < treeIDs.size(); i++) {
    mInputStreams.push_back(new ParallelSortedInputStream(treeIDs[i], DOWNSTREAM, 
                                  &mGatherCommunicator,
                                  (mGatherFlow->sources(treeIDs[i]).size())));

    mOutputStreams.push_back(new ParallelOutputStream(
                                   mGatherFlow->sinks(treeIDs[i]), 
                                   &mGatherCommunicator));

    //mLocalTreeOutputStream = new ParallelOutputStream(
    //                                 mGatherFlow->sources(mTrees[i]->id()),
    //                                 &mGatherCommunicator);
  }

 // mLocalTreeInputStream = new ParallelInputStream(mTrees[0]->id(), UPSTREAM,
  //                                                  &mGatherCommunicator, 
  //                                                  mGatherFlow->numLevels());

  return 1;
}

int ParallelControl::initialize(uint8_t block_bits) {
  uint32_t i, k;
  std::vector<uint32_t> sinks;
  std::vector<uint32_t> treeIDs;
  enum directionType {DOWNSTREAM, UPSTREAM};

  uint64_t GraphID;

  // Every MPI rank computes some trees from the ModuloFlow depending on its
  // rank. We now get the treeIDs that need to be computed by this MPI rank.
  treeIDs = getTreeIDs(mRank, SPATIAL);
  //treeIDs = getTreeIDs(mRank, MODULO);

  // Need to associate patch with a graphID
  // A unique ID is given to every patch
  // This will probably be guided from the indices of the patches
  GraphID = mRank;
  
  // Create a tree for every treeID that gets computed by this MPI rank
  for (i=0; i < treeIDs.size(); i++) 
    mTrees.push_back(new SegmentedMergeTree(treeIDs[i], block_bits));

  std::cout << "Initializing Parallel Control Rank :: "<< mRank << std::endl;
  mGatherCommunicator.connect(mRank, mGatherFlow, treeIDs, SPATIAL);
  mGatherCommunicator.setFlow(mGatherFlow);
  mScatterCommunicator.connect(mRank, mGatherFlow, treeIDs, SPATIAL);

  //mGatherCommunicator.connect(mRank, mGatherFlow, treeIDs, MODULO);
  //mGatherCommunicator.setFlow(mGatherFlow);
  //mScatterCommunicator.connect(mRank, mGatherFlow, treeIDs, MODULO);

  for (i=0; i < treeIDs.size(); i++) {
    mInputStreams.push_back(new ParallelSortedInputStream(treeIDs[i], DOWNSTREAM, 
                                  &mGatherCommunicator,
                                  (mGatherFlow->sources(treeIDs[i]).size())));

    mOutputStreams.push_back(new ParallelOutputStream(
                                   mGatherFlow->sinks(treeIDs[i]), 
                                   &mGatherCommunicator));

    //mLocalTreeOutputStream = new ParallelOutputStream(
    //                                 mGatherFlow->sources(mTrees[i]->id()),
    //                                 &mGatherCommunicator);
  }

 // mLocalTreeInputStream = new ParallelInputStream(mTrees[0]->id(), UPSTREAM,
  //                                                  &mGatherCommunicator, 
  //                                                  mGatherFlow->numLevels());

  return 1;
}

int ParallelControl::localCompute(FunctionType low_threshold, 
                                  FunctionType high_threshold) {
  uint32_t i;
  std::vector<uint32_t> sinks;
  std::vector<uint32_t> sSinks;
  std::vector<uint32_t> sources;
  TopoOutputStream* upstream;
  enum directionType {DOWNSTREAM, UPSTREAM};
  

  // Phase 1: compute the local trees for every patch allocated to the rank
  // Right now every rank gets a single patch
  for (i = 0; i < mPatches.size(); i++) {

    sinks = mGatherFlow->sinks(mTrees[i]->id());

    //printf("gothere %d Tree ID : %d Sink :: %d Rank :: %d\n", i,
    //        mTrees[i]->id(), sinks[0], mRank);

    std::cout << "Computing Local Tree... TREE ID :: " << mTrees[i]->id() 
              << std::endl;
    mLocalAlgorithm->apply(mPatches[i], 0,
        *static_cast<SegmentedMergeTree*> (mTrees[i]),
        mOutputStreams[i],low_threshold,high_threshold);
  }

  return 1;
}

int ParallelControl::joinTrees()
{ 
  TopoOutputStream* upstream;
  enum directionType {DOWNSTREAM, UPSTREAM};
 
  // Once the local compute is done, we merge all the trees that are available
  // on the inputStream for this Parallel Control instance
  for (int i = 1; i < mGatherFlow->numLevels(); i++) {
  
    //std::cout << "Local Tree fixing.. Round :: " << i 
    //          << " Rank :: " << mRank 
    //          << std::endl;

    // Every local tree will receive a singe augmented tree for every iteration
    mLocalTreeInputStream = new ParallelSortedInputStream(mTrees[0]->id(), UPSTREAM,
                                                    &mGatherCommunicator, 1); 
                                                  //outstanding =1 as we wil 
                                                  //receive only one EMPTY token
    if (mTrees.size()> i) {
      std::cout << "\nComputing Gather Tree... TREE ID :: " << mTrees[i]->id()
                << " Rank:: " << mRank << std::endl;

      // The augmented tree will be sent to the leaves 
      //mScatterFlow->scatterSinks(mTrees[i]->id());
      //sSinks = mScatterFlow->getScatterSinks();

      // Every gather tree will send the augmented merge tree to the leaves
      // This will be used for the upstream data transfer
      mLocalTreeOutputStream = new ParallelOutputStream(
                                     mGatherFlow->sources(mTrees[i]->id()),
                                     &mGatherCommunicator);
      

      //std::cout << "Printing Scatter Communicator...Rank :: \n" << mRank;
      //mScatterCommunicator.printMap();
      //std::cout << "Printing Gather Communicator...Rank :: \n" << mRank;
      
      // Apply the gather algorithm
      mGatherAlgorithm->apply(*mTrees[i], mInputStreams[i], mOutputStreams[i],
                              mLocalTreeOutputStream, mGatherFlow);

      std::cout << "\nGATHER COMPLETED!! TREE ID :: " << mTrees[i]->id() 
                << " RANK:: " 
                << mRank << std::endl;
    }

  // Apply the scatter Algorithm
  //std::cout << "\nSCATTER CALLED :: RANK :: " << mRank 
  //          << " NUM_LEVEL:: "<< i 
  //          << std::endl;
  //printf("InputStream[0] :: %p\n", mInputStreams[0]); 
      //if (mRank == 0 || mRank == 2)
      //mGatherCommunicator.printMap();

  //mScatterAlgorithm->apply(*mTrees[0],  mLocalTreeInputStream,
  //                          mLocalTreeOutputStream); 

  //std::cout << "Local Tree fixed. Round :: " << i 
  //          << " Rank :: " << mRank << std::endl;   
  mGatherCommunicator.clearRequests();
  delete mLocalTreeInputStream;

  }
  
  //std::cout << "Exiting ParallelControl.. Rank:: " << mRank << std::endl;
  return 1;
}

int ParallelControl::generateSegmentation(SegmentedMergeTree& mTree) {

  // Assuming one patch per rank at the moment
  const FunctionType* tField = mPatches[0]->field(0);
  SubBox& tDomain = mPatches[0]->domain();
  GlobalIndexType currentGID, segID;
  TreeNode* node;
  FunctionType tVal;
  GlobalIndexType* segBuffer = new
                               GlobalIndexType[2*mTree.getSegmentationSize()+1];

  int count = 1;

  for (LocalIndexType lindex=0; lindex< mTree.getSegmentationSize(); lindex++) {

    currentGID = tDomain.globalIndex(lindex);
    segID = mTree.getSegmentationID(lindex);
    tVal = tField[tDomain.fieldIndex(lindex)];

    if (segID == -1) {
      //std::cout << "GID:: " << currentGID 
      //          << " SEG ID:: " << segID
      //          << " Tree Id:: " << mTree.id()
      //         << std::endl;
      continue;
    }

    // Find the segID in the local tree 
    node = mTree.findElement(segID);

    // if node exists, traverse downwards to root to get node immediately higher
    // than currentGID
    if (node != NULL) {
      while (node->down()!= NULL) { 
        if ((tVal == node->value()) ? (currentGID > node->id()) : 
                                      (tVal > node->value())) 
           break;
        segID = node->id();
        node = node->down();
      }
    }
    else { // segID does not exist in tree. Then search in record
      segID = mTree.getParentFromRecord(segID);

      // From existing node in tree find the correct segID
      node = mTree.findElement(segID);
      if (node == NULL)
         std::cout << "SegID does not exist\n" ;     
      
      while (node->down()!= NULL) {
        if ((tVal == node->value()) ? (currentGID > node->id()) : 
                                          (tVal > node->value()))
          break;
        segID = node->id();
        node = node->down();
      }
    }

    mTree.setSegmentationID(lindex, segID);
    segBuffer[count++] = currentGID;
    segBuffer[count++] = segID;

    node = mTree.findElement(segID);
    if (node == NULL)
      std::cout << "SegID does not exist\n" ;     
    //std::cout << "GID:: " << currentGID
    //          << " SEG ID:: " << segID
    //          << " Tree Id:: " << mTree.id()
    //          << std::endl;
  }

  segBuffer[0] = count-1;
  //std::cout << "SIZE :: " << count << "SegBuffer :: " << segBuffer[0] << "\n";
  //std::cout << "Segmentation Recorded.. \n";
  char filename[32];
  sprintf(filename, "Seg_tree_%d_%s", mRank, mOutputFileName);
  FILE *fp = fopen(filename, "w");
  fwrite(&mRank, sizeof(int ), 1, fp);
  fwrite(segBuffer, sizeof(GlobalIndexType), count, fp);
  fclose(fp);

  delete segBuffer;
  return 1;
}


int ParallelControl::save() {

  generateSegmentation(*static_cast<SegmentedMergeTree*>(mTrees[0]));

  char filename[32];
  sprintf(filename, "Seg_tree_%d_%s", mRank, mOutputFileName);

  FILE* fp = fopen(filename, "a");
  mTrees[0]->writeToFileBinary(fp);

  int i =0;
  for (i=0; i < mInputStreams.size(); i++) {
    delete mInputStreams[i];
  }

  for (i=0; i < mOutputStreams.size(); i++) {
    delete mOutputStreams[i];
  }
  for (i=0; i < mTrees.size(); i++) {
    delete mTrees[i];
  }

  return 1;
}
