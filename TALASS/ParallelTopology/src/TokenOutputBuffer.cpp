/*
 * TokenBuffer.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: bremer5
 */

#include "TokenOutputBuffer.h"
#include <iostream>


TokenOutputBuffer::TokenOutputBuffer(uint32_t max_size) :
/*mCapacity(max_size),*/ mCompact(false)
{
  max_size = 1024*1023*24;
  mCapacity = max_size;
  mBuffer = new char[max_size];
  mToken = mBuffer + sizeof(uint32_t) + sizeof(GraphID) + sizeof(uint8_t);
}

TokenOutputBuffer::~TokenOutputBuffer()
{
  delete[] mBuffer;
 }

void TokenOutputBuffer::reset()
{
  mToken = mBuffer + sizeof(uint32_t) + sizeof(GraphID) + sizeof(uint8_t);
  mTypeBuffer.clear();
  mCompact = false;
}


uint32_t TokenOutputBuffer::size() const
{
  // The size is computed as the size of the token portion
  // plus the type of the type-portion plus two 32-bit int's
  // that store the respective sizes plus the graphID that stores the
  // the destination of the message
  return mToken - mBuffer + sizeof(uint32_t) + mTypeBuffer.size();
}

const char* TokenOutputBuffer::buffer() const
{
  sterror(!mCompact,"This buffer is not valid");
  return mBuffer;
}


void TokenOutputBuffer::compactify()
{
  //fprintf(stderr,"Compactifying buffer with\n\t%d bytes (total %d)\n\twith %d\
  tokens\n\tfirst %d last %d\n",mToken - mBuffer - sizeof(uint32_t) - sizeof(GraphID)
  //     -  sizeof(uint8_t),size(),mTypeBuffer.size(),mTypeBuffer[0],mTypeBuffer.back());

  // Store the size of the data section in the first bytes
  *((uint32_t*)mBuffer) = mToken - mBuffer - sizeof(uint32_t) - sizeof(GraphID)
                           -  sizeof(uint8_t);

  // Set the destination to -1
  // This will be reset by the Communicator
  *((GraphID*)(mBuffer + sizeof(uint32_t))) = -1;

  // Set the direction to -1
  // This will be set by the communicator
  *((uint8_t*)(mBuffer + sizeof(uint32_t) + sizeof(GraphID))) = -1;

  // Store the count in the next bytes
  *((uint32_t*)mToken) = (uint32_t)mTypeBuffer.size();

  // Copy the type information to the end
  memcpy(mToken + sizeof(uint32_t),&mTypeBuffer[0],mTypeBuffer.size());

  mCompact = true;
}

