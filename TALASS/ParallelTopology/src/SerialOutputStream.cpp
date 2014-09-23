/*
 * SerialOutputStream.cpp
 *
 *  Created on: Feb 13, 2012
 *      Author: bremer5
 */

#include "SerialOutputStream.h"

SerialOutputStream::SerialOutputStream(const std::vector<GraphID>& destinations, TopoCommunicator* com) :
TopoOutputStream(destinations), mCommunicator(com)
{
}

int SerialOutputStream::write(const char* buffer, uint32_t size)
{
  //fprintf(stderr,"Sending buffer to tree %d %p %d\n",mDestinations[0],buffer,size);
  mCommunicator->send(mDestinations,buffer,size);

  return 1;
}
