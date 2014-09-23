/*
 * ParallelOutputStream.h
 *
 *  Created on: Feb 13, 2012
 *      Author: bremer5
 */

#ifndef PARALLELOUTPUTSTREAM_H_
#define PARALLELOUTPUTSTREAM_H_

#include "TopoCommunicator.h"
#include "ParallelCommunicator.h"
#include "TopoOutputStream.h"

//! An output stream that pushes data to a communicator
class ParallelOutputStream: public TopoOutputStream {
public:

  //! Default constructor
  ParallelOutputStream(const std::vector<GraphID> destinationTrees,
                       ParallelCommunicator* com);

  //! Destructor
  virtual ~ParallelOutputStream() {}

private:

  //! The communicator from which to pull data
  TopoCommunicator* const mCommunicator;

  //! Write the given buffer to the communicator
  virtual int write(const char* buffer, uint32_t size);
};

#endif /* PARALLELOUTPUTSTREAM_H_ */
