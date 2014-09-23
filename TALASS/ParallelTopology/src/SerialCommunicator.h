/*
 * SerialCommnicator.h
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#ifndef SERIALCOMMNICATOR_H_
#define SERIALCOMMNICATOR_H_

#include <map>
#include "TopoCommunicator.h"


//! A class to implement a single threaded communicator which buffers
//! incoming messages by tree
class SerialCommunicator : public TopoCommunicator
{
public:

  //! Default constructor
  SerialCommunicator() {}

  //! Destructor
  virtual ~SerialCommunicator() {}

  //! Function to pull data from an id to a particular stream
  virtual int connect(GraphID id, TopoInputStream* stream);

  //! Function to send the buffer to a particular id
  virtual int send(const std::vector<GraphID>& destinations, const char* buffer, uint32_t size);
  
  virtual int sendToFile(const std::vector<GraphID>& destinations, const char* buffer, uint32_t size){}

private:

  //! The map of graph id's to their respective input streams
  std::map<GraphID,TopoInputStream*> mStreamMap;
};


#endif /* SERIALCOMMNICATOR_H_ */
