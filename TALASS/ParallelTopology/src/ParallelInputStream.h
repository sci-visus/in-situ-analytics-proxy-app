/*
 * ParallellInputStream.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef PARALLELINPUTSTREAM_H_
#define PARALLELINPUTSTREAM_H_

#include "TopoInputStream.h"
#include "ParallelCommunicator.h"
#include "FiFoBuffer.h"

//! Declaration of the communicator interface
class ParallelCommunicator;

//! An input stream that pulls data from a communicator
class ParallelInputStream: public TopoInputStream {
public:

  //! Default constructor
  ParallelInputStream(GraphID treeID, uint8_t direction, ParallelCommunicator* comm, uint32_t outstanding);

  //! Destructor
  virtual ~ParallelInputStream() {
  }

  //! Following set of methods are used for the PARALLEL_GATHER_SORT algo

  void recvAllMessagesFromCommunicator();

  void initializeSortedTokenStream();

  TopoInputStream::iterator getNextToken();

private:

  //! The element id we represent
  const GraphID mTreeID;

  //! The communicator from which to pull data
  ParallelCommunicator* const mCommunicator;

  //! Pull more data
  virtual int pull();

};

#endif /* PARALLELINPUTSTREAM_H_ */
