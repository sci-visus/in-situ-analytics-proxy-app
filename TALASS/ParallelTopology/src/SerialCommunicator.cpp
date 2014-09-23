/*
 * SerialCommunicator.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "TopoInputStream.h"
#include "SerialCommunicator.h"

int SerialCommunicator::connect(GraphID id, TopoInputStream* stream) {
  sterror(mStreamMap.find(id) != mStreamMap.end(),"Connecting multiple input streams is not supported");
  mStreamMap[id] = stream;

  return 1;
}

int SerialCommunicator::send(const std::vector<GraphID>& destinations, const char* buffer, uint32_t size)
{
  std::map<GraphID,TopoInputStream*>::iterator it;
  std::vector<GraphID>::const_iterator vIt;

  for (vIt=destinations.begin();vIt!=destinations.end();vIt++) {
    it = mStreamMap.find(*vIt);

    sterror(it==mStreamMap.end(),"GraphID not found this communicator knows nothing about graph %d",*vIt);


    char* message = new char[size];
    memcpy(message,buffer,size);

    it->second->push(message);
  }

  return 1;
}
