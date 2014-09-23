/*
 * TopoCommunicator.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef TOPOCOMMUNICATOR_H_
#define TOPOCOMMUNICATOR_H_

#include <vector>
#include "DistributedDefinitions.h"

//! Declaration of the input stream
class TopoInputStream;

//! A TopoCommunicator handles messages between elements of unique
//! id's via streams
class TopoCommunicator
{
public:

  //! Default constructor
  TopoCommunicator() {}

  //! Destructor
  virtual ~TopoCommunicator() {}

  //! Associate with the MPI rank
  //virtual int connect(GraphID id, RankID rank) =0;

  //! Connect the stream to receive data for the given graph
  virtual int connect(GraphID id, TopoInputStream* stream) = 0;

  //! Function to receive MPI messages
  //virtual int recv(TopoInputStream* inputStream) = 0;

  //! Function to send the buffer to set of graphs
  virtual int send(const std::vector<GraphID>& id, const char* buffer, uint32_t size) = 0;
  
  virtual int sendToFile(const std::vector<GraphID>& id, const char* buffer, uint32_t size) = 0;

};

#endif /* TOPOCOMMUNICATOR_H_ */
