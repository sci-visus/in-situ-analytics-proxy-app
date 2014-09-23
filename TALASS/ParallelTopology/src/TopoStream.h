/*
 * TopoStream.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef TOPOSTREAM_H
#define TOPOSTREAM_H

#include "DistributedDefinitions.h"
#include "TopoCommunicator.h"

//! Baseclass for input and output streams
class TopoStream {

public:

  //! The maximal message size
  static const uint32_t mMaxMessageSize;

  //! The default size of the buffer
  static const uint32_t mDefaultBufferSize;

  //! Default constructor
  TopoStream(TopoCommunicator* com, GraphID gid) : mCommunicator(com), mId(gid) {}

  //! Destructor
  ~TopoStream();

  //! Add size many bytes to the internal buffer
  int add(void* data, uint32_t size);

  //! Remove size many bytes from the internal buffer
  int remove(uint32_t size);

protected:

  //! Pointer to the communicator
  TopoCommunicator* mCommunicator;

  //! The graph id of the graph this stream represents
  const GraphID mId;

private:

  //! The local buffer we will read or write to
  char* mBuffer;

  //! The size of the current buffer
  uint32_t mSize;

  //! Index to the first valid element of the buffer
  uint32_t mHead;

  //! Index of the first invald element of the buffer
  uint32_t mTail;
};


#endif /* TOPOSTREAM_H_ */
