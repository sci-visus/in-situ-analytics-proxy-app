/*
 * ParallelOutputStream.cpp
 *
 *  Created on: Feb 13, 2012
 *      Author: bremer5
 */

#include "ParallelOutputStream.h"

ParallelOutputStream::ParallelOutputStream(
                        const std::vector<GraphID> destinationTrees, 
                        ParallelCommunicator* com) 
    : TopoOutputStream(destinationTrees), 
      mCommunicator(com) {
}

int ParallelOutputStream::write(const char* buffer, uint32_t size) {
  //mCommunicator->send(mDestinations, buffer, size);
  mCommunicator->sendToFile(mDestinations, buffer, size);

  return 1;
}
