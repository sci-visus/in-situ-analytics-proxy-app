/*
 * TopoOutputStream.cpp
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#include "TopoOutputStream.h"

//! The default message size
const uint32_t TopoOutputStream::sDefaultMessageSize = 4096;

TopoOutputStream::TopoOutputStream(const std::vector<GraphID>& destinations,uint32_t message_size) :
        mDestinations(destinations), mBuffer(message_size)
{
}


int TopoOutputStream::flush()
{
  mBuffer.compactify();

  // Pass it on
  write(mBuffer.buffer(),mBuffer.size());

  // and clear it
  mBuffer.reset();

  return 1;
}
