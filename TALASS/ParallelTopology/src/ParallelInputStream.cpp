/*
 * ParallelInputStream.cpp
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#include "ParallelCommunicator.h"
#include "ParallelInputStream.h"
#include <iostream>

ParallelInputStream::ParallelInputStream(GraphID treeID, uint8_t direction, 
                                         ParallelCommunicator* comm, 
                                         uint32_t outstanding) 
    : TopoInputStream(outstanding), 
      mTreeID(treeID), 
      mCommunicator(comm)
{
  // We do connect separately for the communicator
  comm->connect(mTreeID, direction, this);
}

int ParallelInputStream::pull()
{
  char *buff = NULL;
  //std::cout << "Pull() called TreeID :: "<< mTreeID << std::endl;
 
  //buff = mCommunicator->recvInternal();

  if (buff == NULL) {

    //std::cout << "TREE ID :: " << mTreeID << "  Calling MPI_RECV \n ";
    mCommunicator->recvFromFile(mTreeID);

  }

  // Assuming push() deletes buff after use
  //this->push(buff);
  //std::cout << "Finished Pull!! TreeID:: "<< mTreeID << std::endl;

  return 1;
}

