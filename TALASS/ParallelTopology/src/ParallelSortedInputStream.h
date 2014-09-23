/*
 * ParallelSortedlInputStream.h
 *
 *  Created on: Mar 13, 2014
 *      Author: landge1
 */

#ifndef PARALLELSORTEDINPUTSTREAM_H_
#define PARALLELSORTEDINPUTSTREAM_H_

#include "SortedInputStream.h"
#include "ParallelCommunicator.h"
#include "FiFoBuffer.h"

//! Declaration of the communicator interface
class ParallelCommunicator;

//! An input stream that pulls data from a communicator
class ParallelSortedInputStream: public SortedInputStream {
public:

  //! Default constructor
  ParallelSortedInputStream(GraphID treeID, uint8_t direction, 
                            ParallelCommunicator* comm, uint32_t outstanding);

  //! Destructor
  virtual ~ParallelSortedInputStream() {
  }

  //! Following set of methods are used for the PARALLEL_GATHER_SORT algo

private:

  //! The element id we represent
  const GraphID mTreeID;

  //! The communicator from which to pull data
  ParallelCommunicator* const mCommunicator;

  //! Pull more data
  virtual int pull();
};

#endif /* PARALLELSORTEDINPUTSTREAM_H_ */
