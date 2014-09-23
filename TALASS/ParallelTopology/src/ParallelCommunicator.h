/*
 * ParallelCommnicator.h
 *
 *  Created on: July 16, 2012
 *      Author: landge1
 */

#ifndef PARALLELCOMMNICATOR_H_
#define PARALLELCOMMNICATOR_H_

#include <map>
#include <queue>
#include "TopoCommunicator.h"
#include "ControlFlow.h"
#include "ParallelInputStream.h"
#include "mpi.h"

//! A class to implement a single threaded communicator which buffers
//! incoming messages by tree
class ParallelCommunicator: public TopoCommunicator {
public:

  //! Default constructor
  ParallelCommunicator() {
  }

  //! Destructor
  virtual ~ParallelCommunicator() {
  }

  //! Function to pull data from an id to a particular stream
  //virtual int connect(RankID rank, ControlFlow* gFlow);

  virtual int connect(GraphID id, TopoInputStream* stream) {
    return 1;
  }
  virtual int connect(GraphID id, uint8_t direction, InputStream* stream);
  
//  virtual int connect(GraphID id, uint8_t direction, SortedInputStream* stream);

  virtual int connect(RankID rank, ControlFlow* gFlow,
                      std::vector<uint32_t> treeIDs, uint32_t mode);

  void setFlow(ControlFlow *gFlow) { mGatherFlow = gFlow; } 

  //uint32_t parent(uint32_t id, uint32_t Factor, uint32_t NumLeafElements);

  //std::vector<int32_t> children(uint32_t Rank, uint32_t Factor, 
  //                              uint32_t NumLeafElements);

  virtual void recv(GraphID treeID);
  
  void recvFromFile(GraphID treeID);

  //! Function to send the buffer to a particular id
  virtual int send(const std::vector<GraphID>& destinations,const char* buffer,
                   uint32_t size);

  //! Function to send the buffer to a file
  int sendToFile(const std::vector<GraphID>& destinations,
                         const char* buffer,
                         uint32_t size);
  
  std::vector<uint32_t> destinations(RankID rank) ;

  uint32_t getTreeMPIrank(uint32_t id, ControlFlow* gFlow, uint32_t mode);

  uint32_t sendInternal(const char* buffer, uint32_t size);

  uint32_t passUpstream(RankID destRank, GraphID destTree, 
                        const char* buffer, uint32_t size);

  char* recvInternal();

  void printInternalBuffer();

  void printMap();

  void clearRequests();
private:

  bool mPostReceives;
  std::vector<char*> mRecvBuffers;
  std::vector<MPI_Request> mMPIreq;
  std::vector<GraphID> mFileSources;

  //! The map of graph id's to their respective input streams
  std::map<GraphID, RankID> mTreeMapRecv;
  std::map<GraphID, RankID> mTreeMapSend;

  //! The map of graph is's to their respective input streams
  std::map<GraphID, InputStream*> mInputStreamMapDownstream;
  std::map<GraphID, InputStream*> mInputStreamMapUpstream;

  ControlFlow* mGatherFlow;

  //std::vector<const char*> mInternalBuffer;
  std::queue<const char*> mInternalBuffer;
  uint32_t mDownStreamRank;
  uint32_t mRank;
  
  void createFile(int dest_rank, const char* buffer, int size, int direction);
  int readMsgFile(int source_rank, char** buffer, int size, int tree_id);
};

#endif /* PARALLELCOMMNICATOR_H_ */
