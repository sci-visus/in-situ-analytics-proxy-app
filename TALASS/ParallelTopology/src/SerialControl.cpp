/*
 * SerialControl.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "SerialControl.h"
#include "MergeTree.h"
#include <iostream>
#include <cstdlib>

SerialControl::SerialControl(bool invert) : mInvert(invert),
mLocalAlgorithm(NULL), mGatherAlgorithm(NULL), mScatterAlgorithm(NULL),
mGatherFlow(NULL), mScatterFlow(NULL)
{
}

SerialControl::~SerialControl()
{
  if (mLocalAlgorithm != NULL)
    delete mLocalAlgorithm;

  if (mGatherAlgorithm != NULL)
    delete mGatherAlgorithm;

  if (mScatterAlgorithm != NULL)
    delete mScatterAlgorithm;

}


void SerialControl::localAlgorithm(LocalAlgorithmType local)
{
  mLocalAlgorithm = LocalComputeAlgorithm::make(local,mInvert);
}

void SerialControl::gatherAlgorithm(GatherAlgorithmType gather)
{
  mGatherAlgorithm = GatherAlgorithm::make(gather,mInvert);
}

void SerialControl::scatterAlgorithm(ScatterAlgorithmType scatter)
{
  mScatterAlgorithm = ScatterAlgorithm::make(scatter,mInvert);
}


int SerialControl::initialize(const std::vector<Patch*>& patches, uint8_t block_bits)
{
  uint32_t i,k;
  std::vector<uint32_t> sinks;

  mPatches = patches;

  // A serial control has all patches locally
  std::cout << "num patches " << patches.size() << std::endl;
  for (i=0;i<patches.size();i++) {
    // and we create a segmented tree for each of them
 //   std::cout << "making tree " << i << std::endl;
    mTrees.push_back(new SegmentedMergeTree(i, block_bits));
  }

  std::cout << "mTrees.size " << mTrees.size() << std::endl;
  i = 0;
  while (i < mTrees.size()) {
    std::cout << "trees.size = " << mTrees.size() << std::endl;
    std::cout << "i = " << i << std::endl;
    std::cout << "mTrees[i]->id = " << mTrees[i]->id() << std::endl;

    // Which trees do we need downstream

    sinks = mGatherFlow->sinks(mTrees[i]->id());

    for (k=0;k<sinks.size();k++) {

      // If these have not been create
      if (sinks[k] >= mTrees.size())
        mTrees.push_back(new MergeTree(mTrees.size(), block_bits)); // do so
    }
    i++;
  }

  std::cout << "mTrees.size " << mTrees.size() << std::endl;
  // Now create all the input/output streams.
  mInputStreams.reserve(mTrees.size());
  mOutputStreams.reserve(mTrees.size());
  for (i=0;i<mTrees.size();i++) {
    mInputStreams[i] = new SerialInputStream(mTrees[i]->id(),mGatherFlow->sources(mTrees[i]->id()).size());
    mCommunicator.connect(mTrees[i]->id(),mInputStreams[i]);
    mOutputStreams[i] = new SerialOutputStream(mGatherFlow->sinks(mTrees[i]->id()),&mCommunicator);
  }

  return 1;
}

int SerialControl::compute(FunctionType low_threshold, FunctionType high_threshold)
{
   printf("SerialControl::compute() called\n");
  uint32_t i;
  std::vector<uint32_t> sinks;
  std::vector<uint32_t> sources;

  // Phase 1: For all patches compute the local tree
  for (i=0;i<mPatches.size();i++) {

     fprintf(stderr,"Computing patch %d  [%d,%d,%d] x [%d,%d,%d]\n", mTrees[i]->id(),
             mPatches[i]->domain().lowCorner()[0],mPatches[i]->domain().lowCorner()[1],mPatches[i]->domain().lowCorner()[2],
             mPatches[i]->domain().highCorner()[0],mPatches[i]->domain().highCorner()[1],mPatches[i]->domain().highCorner()[2]);

     //if ((i == 88) || (i == 89))
     mLocalAlgorithm->apply(mPatches[i], 0,
                            *static_cast<SegmentedMergeTree*>(mTrees[i]),
                            ///*mOutputStreams[sinks[0]]*/ NULL);
                            //mOutputStreams[sinks[0]]);
                            mOutputStreams[i],low_threshold,high_threshold);
     //break;
#if 1
    char fileName[32] = {0};
    sprintf(fileName, "output_%d.dot", i);
    FILE* fp = fopen(fileName, "w");
    mTrees[i]->writeToFile(fp);
#endif
     delete mTrees[i];
  }

  //return 1;
  //i = mPatches.size();

  fprintf(stderr,"Done with Local Compute ! \n\n");
  // For the rest of the trees call gather
  for (;i<mTrees.size();i++) {
  //for (i=460;i<mTrees.size();i++) {


    // Which tree do we need downstream
    sinks = mGatherFlow->sinks(mTrees[i]->id());

    sources = mGatherFlow->sources(mTrees[i]->id());

    // Which trees do we need upstream
    //sources = mScatterFlow->sinks(mTrees[i]->id());

    // Create an output stream for the upstream scatter
    //upstream = new SerialOutputStream(sources,&mCommunicator);

    fprintf(stderr,"Gather tree %d Merging %d %d\n", mTrees[i]->id(),sources[0],sources[1]);

    // Apply the gather algorithm
    mGatherAlgorithm->apply(*mTrees[i],mInputStreams[i],mOutputStreams[i],/*upstream*/ NULL,mGatherFlow);

#if 1
    char fileName[32] = {0};
    sprintf(fileName, "output_merge_%d.dot", i);
    FILE* fp = fopen(fileName, "w");
    mTrees[i]->writeToFile(fp);
#endif
    // Remove the output stream
    //delete upstream;

    delete mInputStreams[i];
    delete mTrees[i];
  }

/*  // Finally, in reverse order call all scatter
  for (i=mTrees.size()-1;i!=0;i++) {

    // Which trees do we need downstream
    sources = mScatterFlow->sinks(mTrees[i]->id());

    // Create an output stream for the upstream scatter
    upstream = new SerialOutputStream(sources,&mCommunicator);

    // Apply the scatter algorithm
    mScatterAlgorithm->apply(*mTrees[i],mInputStreams[i],upstream);

    // Remove the output stream
    delete upstream;
  }
*/
  return 1;
}

int SerialControl::save()
{

  return 1;
}
