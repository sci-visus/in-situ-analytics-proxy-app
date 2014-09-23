/*
 * FiFoBuffer.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "DistributedDefinitions.h"
#include "FiFoBuffer.h"

using namespace std;

const uint32_t FiFoBuffer::sDefaultBufferSize = 8192;

FiFoBuffer::FiFoBuffer() : mHead(0)
{
  mFront = new std::vector<char>();
  mFront->reserve(sDefaultBufferSize);

  mBack = new std::vector<char>();
  mBack->reserve(sDefaultBufferSize);
}

FiFoBuffer::~FiFoBuffer()
{
  sterror(mFront->size()>0,"~FiFoBuffer: deleting non-empty buffer.");
  sterror(mBack->size()>0,"~FiFoBuffer: deleting non-empty buffer.");

  delete mFront;
  delete mBack;
}


int FiFoBuffer::push(char* tokens, uint32_t size)
{
  uint32_t pos;

  // If we are already filling the second buffer or if the new
  // message would not fit into the first buffer anymore
  if ((mBack->size() > 0)
      || (mFront->capacity() - mFront->size() < size)) {
    pos = mBack->size();

    // We have no choice but to make sure that the message fits
    // into the second buffer. Note that this call should not
    // allocate any memory if there is still capacity
    mBack->resize(pos + size);

    // and copy the data
    memcpy(&((*mBack)[pos]),tokens,size);
  }
  else { // Otherwise we fit the data into the first buffer

    pos = mFront->size();

    // This call is guaranteed to not allocate any memory
    mFront->resize(pos+size);

    // and copy the data
    memcpy(&((*mFront)[pos]),tokens,size);
  }

  return 1;
}

int FiFoBuffer::pop(uint32_t size)
{
  // There should be at least size many bytes left
  sterror((mFront->size() - mHead) < size,"There are not enough bytes left to pop");

  mHead += size;

  //! If this was the last token out of the first buffer
  if (mHead == mFront->size()) {

    //! Swap the two buffers
    std::swap(mFront,mBack);

    //! Reset the head
    mHead = 0;

    //! Invalidate the second buffer
    mBack->resize(0);
  }

  return 1;
}

char *FiFoBuffer::top()
{
  sterror(mHead > mFront->size(),"The buffer you are trying to read is empty");
  return &((*mFront)[mHead]);
}


