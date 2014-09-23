/*
 * ParallelSortedInputStream.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: landge1
 */

#include "ParallelCommunicator.h"
#include "ParallelSortedInputStream.h"
#include <iostream>

ParallelSortedInputStream::ParallelSortedInputStream(GraphID treeID, 
                                                     uint8_t direction, 
                                                     ParallelCommunicator* comm,
                                                     uint32_t outstanding) 
    : SortedInputStream(outstanding), 
      mTreeID(treeID), 
      mCommunicator(comm)
{
  // We do connect separately for the communicator
  comm->connect(mTreeID, direction, this);
}

int ParallelSortedInputStream::pull()
{
  char *buff = NULL;
  //std::cout << "Pull() called TreeID :: "<< mTreeID << std::endl;
 
  //buff = mCommunicator->recvInternal();

  if (buff == NULL) {

    //std::cout << "TREE ID :: " << mTreeID << "  Calling MPI_RECV \n ";
    //mCommunicator->recv(mTreeID);
    mCommunicator->recvFromFile(mTreeID);

  }

  // Assuming push() deletes buff after use
  //this->push(buff);
  //std::cout << "Finished Pull!! TreeID:: "<< mTreeID << std::endl;

  return 1;
}

