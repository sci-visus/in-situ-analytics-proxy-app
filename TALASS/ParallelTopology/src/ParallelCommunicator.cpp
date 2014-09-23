/*
 * ParallelCommunicator.cpp
 *
 *  Created on: July 16, 2012
 *      Author: landge1
 */

#include "mpi.h"
#include "ControlFlow.h"
#include "ModuloFlow.h"
#include "ParallelCommunicator.h"
#include "Token.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>

#define MODULO 0
#define SPATIAL 1

uint32_t ParallelCommunicator::getTreeMPIrank(uint32_t id, ControlFlow* gFlow,
                                              uint32_t mode) {

  uint32_t k;
  std::vector<uint32_t> sources;
 
  switch(mode) {

    case MODULO: 
    {
      if (id < gFlow->baseCount())
        return id;
      else {
        sources =  gFlow->sources(id);
        k = *std::min_element(sources.begin(), sources.end());
        while(k >= gFlow->baseCount()){
          sources =  gFlow->sources(k);
          k = *std::min_element(sources.begin(), sources.end());
        }
        return k;
      }
      break;
    }

    case SPATIAL:
    {
      return gFlow->getBlockId(id);    
    }

    default:
    {
      std::cout << "Incorrect flow mode in communicator.. \n"; 
      return -1;
    }
  }
  return -1;
}


int ParallelCommunicator::connect(RankID rank, ControlFlow* gFlow,
std::vector<uint32_t> treeIDs, uint32_t mode) {
 
  mPostReceives = false;
  std::vector<uint32_t> sink;

  // Set the MPI rank for this communicator  
  mRank = rank;

  // Initializing downstream rank to self
  mDownStreamRank = mRank;

  // We store the MPI rank of the sink for every tree to be computed by this
  // rank. This allows a simple look up while sending data downstream.
  for (uint32_t i=0; i< treeIDs.size(); i++) {
    sink = gFlow->sinks(treeIDs[i]);
    if (!sink.empty()) {
      mTreeMapSend[sink[0]] = getTreeMPIrank(sink[0], gFlow, mode);
      //std::cout << "rank :: " << mRank << " ID asked:: " << treeIDs[i] 
      //          << " Dest:: " << mTreeMapSend[sink[0]] << "\n";
    }
    std::vector<uint32_t> sources;
    sources = gFlow->sources(treeIDs[i]);

    for (uint32_t j=0; j<sources.size(); j++) {
      mTreeMapRecv[sources[j]]=getTreeMPIrank(sources[j],gFlow, mode);
    }
  }
 /* 
  // Prints for debugging
  int target= -1; 
  //std::cout << "ParallelComm Rank:: " << mRank << std::endl;
  for (int i=0; i< treeIDs.size(); i++) {
    sink = gFlow->sinks(treeIDs[i]);
    if (!sink.empty())
      target = mTreeMapSend[sink[0]] ;
    std::cout << "TreeID " << treeIDs[i] << " MPI Rank: " <<
              target << "\n";
  }
  */

  return 1;
}

void ParallelCommunicator::printMap() {

  printf( "\n## RANK :: %d  MAP 0 :: %p\n", mRank, mInputStreamMapUpstream[0]);
  printf( "\n## RANK :: %d  MAP 1 :: %p\n", mRank, mInputStreamMapUpstream[1]);
  printf( "\n## RANK :: %d  MAP 2 :: %p\n", mRank, mInputStreamMapUpstream[2]);
  printf( "\n## RANK :: %d  MAP 3 :: %p\n", mRank, mInputStreamMapUpstream[3]);
  printf( "\n## RANK :: %d  MAP 4 :: %p\n", mRank, mInputStreamMapUpstream[4]);
  printf( "\n## RANK :: %d  MAP 5 :: %p\n", mRank, mInputStreamMapUpstream[5]);
  printf( "\n## RANK :: %d  MAP 6 :: %p\n", mRank, mInputStreamMapUpstream[6]);
}

Token::TokenType checkLastToken(char *Buffer) {
  const uint8_t* tokens = (const uint8_t*)(Buffer + sizeof(uint32_t) +
                        sizeof(GraphID) + sizeof(uint8_t) + 
                        *(const uint32_t*)(Buffer));
  uint32_t count = *(uint32_t*)(tokens);
  return (Token::TokenType)tokens[count + sizeof(uint32_t) - 1];
}

void ParallelCommunicator::recv(GraphID treeID) {//TopoInputStream* inputStream) {

  std::vector<RankID>::const_iterator it;
  char* buff;
  GraphID destTree = -1;
  uint8_t direction = -1;
  enum directionType {DOWNSTREAM, UPSTREAM};
  uint32_t bufferSize = 1024*1024*25; 
  std::vector<uint32_t> sources;

  sources = mGatherFlow->sources(treeID);

  if(!mPostReceives) {
    uint32_t sourceMPIrank;

    // post receives for downstream comm
    for (uint32_t i=0; i<sources.size() ; i++) {
      sourceMPIrank = mTreeMapRecv[sources[i]];

      if (sourceMPIrank != mRank) {
        MPI_Request req;
        buff = new char[bufferSize]; 
        
        MPI_Irecv((void*)buff, bufferSize,  MPI_BYTE, sourceMPIrank, 0,
                  MPI_COMM_WORLD, &req);

        mRecvBuffers.push_back(buff);
        mMPIreq.push_back(req);
        //std::cout << "POsted downstream recv:: " << mRank
        //          << " Source MPI Rank: " << sourceMPIrank
        //          << std::endl;
      }
    }

    // post receive for upstream communication
    // since we recv from sink during upstream comm
    sourceMPIrank = mDownStreamRank;
    if (sourceMPIrank != mRank) {
      MPI_Request req;
      buff = new char[bufferSize];

      MPI_Irecv((void*)buff, bufferSize,  MPI_BYTE, sourceMPIrank, 0,
                MPI_COMM_WORLD, &req);

      mRecvBuffers.push_back(buff);
      mMPIreq.push_back(req);
      //std::cout << "POsted upstream recv:: " << mRank
      //          << " Source MPI Rank: " << sourceMPIrank
      //          << std::endl;
    }

    mPostReceives = true;
    //std::cout << "Receives Posted... rank :: " << mRank << std::endl;
  }
 
  while (destTree != treeID) {
     
    //buff = new char[bufferSize]; 
    // We receive from any source that is sending to this rank
    //MPI_Irecv(buff, 8192*2, MPI_BYTE, MPI_ANY_SOURCE, 0, 
    //          MPI_COMM_WORLD, &request);
    //request, MPI_STATUS_IGNORE);
    //MPI_Recv(buff, 8192*2, MPI_BYTE, MPI_ANY_SOURCE, 0, 
    //         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //std::cout << "Waiting... Rank :: " << mRank 
    //          << " TREE ID:: " << treeID << std::endl;
    //MPI_Recv(buff, bufferSize, MPI_BYTE, MPI_ANY_SOURCE, 0, 
    //         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
    int index=0;
    int status;
    MPI_Status mpiStatus;

    //std::cout << "number of buffers:: "<< mRecvBuffers.size() 
    //          << " number of requests:: " << mMPIreq.size() << std::endl; 
    status = MPI_Waitany(mRecvBuffers.size(), &mMPIreq[0], &index, &mpiStatus);

    //std::cout << "Waiting...done.. rank :: " << mRank 
    //          << " index:: " << index 
    //          << " :: Source: " << mpiStatus.MPI_SOURCE 
    //          << std::endl;
    if (status != MPI_SUCCESS || index < 0) {  
      printf( "Error in receiving messages. Tree: %d rank: %d\n", 
              treeID, mRank);
    }

    buff = mRecvBuffers[index];

    //uint32_t size = *(uint32_t*) buff;
    destTree = *(GraphID*)(buff + sizeof(uint32_t));
    direction = *(uint8_t*)(buff + sizeof(uint32_t) + sizeof(GraphID));

    //std::cout << "MPI RECV :: Rank: " << mRank 
    //  << " Size:: " << (int) size 
    //  << " Dest Tree :: " << destTree
    //  << " Direction :: " << (int)direction
    //  << " Token Count :: " <<  *(uint32_t*)(buff + sizeof(uint32_t) 
    //                                         + sizeof(GraphID) + sizeof(uint8_t)
    //                                         + size) 
    //  << std::endl;

    if (checkLastToken(buff) != Token::EMPTY) {
      char *buff1 = new char[bufferSize];
      MPI_Request req;
      MPI_Irecv((void*)buff1, bufferSize,  MPI_BYTE, mpiStatus.MPI_SOURCE, 0,
                MPI_COMM_WORLD, &req);
      mRecvBuffers.push_back(buff1);
      mMPIreq.push_back(req);
    }

    if (direction == DOWNSTREAM) {
      mInputStreamMapDownstream[destTree]->push(buff);
    }
    else if (direction == UPSTREAM) {
      if (destTree != mRank) {
        std::vector<GraphID> destinations = mGatherFlow->sources(destTree);
        std::map<GraphID, RankID>::iterator destRankIt;
        uint32_t destRank;
        std::vector<uint32_t> internalTrees; 

        for (uint32_t i=0 ; i < destinations.size(); i++) {
          destRankIt = mTreeMapSend.find(destinations[i]);
          if (destRankIt == mTreeMapSend.end()) {
            //destRank = getTreeMPIrank(destinations[i], mGatherFlow, MODULO);
            destRank = getTreeMPIrank(destinations[i], mGatherFlow, SPATIAL);
            mTreeMapSend[destinations[i]] = destRank;
          }
          else 
            destRank = destRankIt->second;

          if (destRank == mRank) {
            internalTrees.push_back(destinations[i]);
          }
          else {  
            //std::cout << "Passing Upstream from TreeID :: " << destTree 
            //          << std::endl;
            passUpstream(destRank, destinations[i], buff, bufferSize);
          }
        }
    
        for (uint32_t i=0; i < internalTrees.size() ; i++) {
          passUpstream(mRank, internalTrees[i], buff, bufferSize);
        }
        break;
      }
      else {
        mInputStreamMapUpstream[destTree]->push(buff);
      }
    }
    else
      std::cout << "Incorrect Direction!!\n";

    //std::cout << "Done Pushing stuff for Tree :: " << destTree << std::endl; 
  }

  //return buff;
}

void ParallelCommunicator::recvFromFile(GraphID treeID) {//TopoInputStream* inputStream) {

  std::vector<RankID>::const_iterator it;
  char* buff;
  GraphID destTree = -1;
  uint8_t direction = -1;
  enum directionType {DOWNSTREAM, UPSTREAM};
  uint32_t bufferSize = 1024*1024*25; 
  std::vector<uint32_t> sources;
  int sourceMPIrank;

  sources = mGatherFlow->sources(treeID);

  if (!mPostReceives) {
    mFileSources = sources;
    mPostReceives = true;
  }
 
  if (!mFileSources.empty()) {
    sourceMPIrank = mTreeMapRecv[mFileSources.back()];
    mFileSources.pop_back();
  }

  int status = 0;
  status = readMsgFile(sourceMPIrank, &buff, bufferSize, treeID);
  mRecvBuffers.push_back(buff);

  uint32_t size = *(uint32_t*) buff;
  destTree = *(GraphID*)(buff + sizeof(uint32_t));
  direction = *(uint8_t*)(buff + sizeof(uint32_t) + sizeof(GraphID));

  //std::cout << "MPI RECV :: Rank: " << mRank 
  //  << " Size:: " << (int) size 
  //  << " Dest Tree :: " << destTree
  //  << " Direction :: " << (int)direction
  //  << " Token Count :: " <<  *(uint32_t*)(buff + sizeof(uint32_t) 
  //      + sizeof(GraphID) + sizeof(uint8_t)
  //      + size) 
  //  << std::endl;

  if (direction == DOWNSTREAM) {
    mInputStreamMapDownstream[treeID]->push(buff);
  }
  else if (direction == UPSTREAM) {
    if (destTree != mRank) {
      std::vector<GraphID> destinations = mGatherFlow->sources(destTree);
      std::map<GraphID, RankID>::iterator destRankIt;
      uint32_t destRank;
      std::vector<uint32_t> internalTrees; 

      for (uint32_t i=0 ; i < destinations.size(); i++) {
        destRankIt = mTreeMapSend.find(destinations[i]);
        if (destRankIt == mTreeMapSend.end()) {
          //destRank = getTreeMPIrank(destinations[i], mGatherFlow, MODULO);
          destRank = getTreeMPIrank(destinations[i], mGatherFlow, SPATIAL);
          mTreeMapSend[destinations[i]] = destRank;
        }
        else 
          destRank = destRankIt->second;

        if (destRank == mRank) {
          internalTrees.push_back(destinations[i]);
        }
        else {  
          //std::cout << "Passing Upstream from TreeID :: " << destTree 
          //          << std::endl;
          passUpstream(destRank, destinations[i], buff, bufferSize);
        }
      }

      for (uint32_t i=0; i < internalTrees.size() ; i++) {
        passUpstream(mRank, internalTrees[i], buff, bufferSize);
      }
      //      break;
    }
    else {
      mInputStreamMapUpstream[destTree]->push(buff);
    }
  }
  else
    std::cout << "Incorrect Direction!!\n";
}

void ParallelCommunicator::clearRequests() {
  mRecvBuffers.clear();
  mMPIreq.clear();
  mPostReceives = false;
}

void ParallelCommunicator::printInternalBuffer() {
  
  std::cout << "\nPrinting Buffer... \n";

  if (!mInternalBuffer.empty()) {
    char* buff = (char*)mInternalBuffer.front();
    std::cout << "Q enteries:: " << mInternalBuffer.size() << std::endl <<
    " Rank:: " << mRank << 
    " Q FRONT :: Token Count:: " <<
    *(uint32_t*)(buff + sizeof(uint32_t) + *(uint32_t*)buff)<< std::endl;

    buff = (char*)mInternalBuffer.back();
    std::cout << " Rank:: "<<mRank<< " Q BACK:: "<<
    " Token Count:: " <<
    *(uint32_t*)(buff + sizeof(uint32_t) + *(uint32_t*)buff)<< std::endl;
  }
  else {
    std::cout << "Buffer Empty\n!!" ;
  }
}

char* ParallelCommunicator::recvInternal() {

  char* buff;
  if (mInternalBuffer.empty()) {
    buff = NULL;
  }
  else {
    //std::cout << "RANK :: " << mRank << "  Receiving Internal!!\n";
    //printInternalBuffer();
    buff = (char*)mInternalBuffer.front();
    mInternalBuffer.pop();
    //printInternalBuffer();
    //std::cout << "RECV INTERNAL :: SIZE :: " << " Rank:: "<<mRank<<
    //" Token Count:: " <<
    //  *(uint32_t*)(buff + sizeof(uint32_t) + *(uint32_t*)buff)<<
    //  " Queue Count:: " << mInternalBuffer.size() << std::endl;
    
    return buff;
  }
  //std::cout << "RECV INTERNAL DONE " << std::endl;

  return buff;  
}

uint32_t ParallelCommunicator::sendInternal(const char* tokenBuffer,
                                            uint32_t size) {

  //printInternalBuffer();
  enum directionType {DOWNSTREAM, UPSTREAM};

  char* buff = new char[size];   
  memcpy(buff, tokenBuffer, size);
  GraphID destTree = *(GraphID*)(buff + sizeof(uint32_t));
  uint8_t direction = *(uint8_t*)(buff + sizeof(uint32_t) + sizeof(GraphID));

  if (direction == DOWNSTREAM)
      mInputStreamMapDownstream[destTree]->push(buff);
  else if (direction == UPSTREAM) {
      //std::cout << "Recieving UPSTREAM Internal... RANK :: " << mRank << "\n";
      //*(GraphID*)(buff + sizeof(uint32_t)) = destTree;
      //std::cout << "\nMPI SEND UPSTREAM INTERNAL:: Destination: " << mRank
      //  << " Rank: " << mRank 
      //  << " Size:: " << *(uint32_t*)buff 
      //  << " Destination Tree :: " << *(GraphID*)(buff + sizeof(uint32_t))
      //  << " Direction :: " << (int)*(uint8_t*)(buff + sizeof(uint32_t) 
      //                                          + sizeof(GraphID))
      //  << " Token Count:: " << *(uint32_t*)(buff + sizeof(uint32_t) + 
      //                                       sizeof(GraphID) + sizeof(uint8_t) 
      //                                       + *(uint32_t*)buff)
      //  << std::endl;
      //printf ("Stream MAP:: %p sending buffer %p\n", 
      //        mInputStreamMapUpstream[destTree], buff);
      if (destTree == mRank)
        mInputStreamMapUpstream[destTree]->push(buff);
      else {
        std::vector<GraphID> destinations = mGatherFlow->sources(destTree);
        std::map<GraphID, RankID>::iterator destRankIt;
        uint32_t destRank;
        std::vector<uint32_t> internalTrees; 
        for (uint32_t i=0 ; i < destinations.size(); i++) {
          destRankIt = mTreeMapSend.find(destinations[i]);
          if (destRankIt == mTreeMapSend.end()) {
            destRank = getTreeMPIrank(destinations[i], mGatherFlow, SPATIAL);
            //destRank = getTreeMPIrank(destinations[i], mGatherFlow, MODULO);
            mTreeMapSend[destinations[i]] = destRank;
          }
          else 
            destRank = destRankIt->second;

          if (destRank == mRank) {
            internalTrees.push_back(destinations[i]);
          }
          else {  
            //std::cout << "Passing Upstream from TreeID :: " << destTree 
            //          << std::endl;
            //uint32_t destRank = 0; 
            passUpstream(destRank, destinations[i], buff, size);
          }
        }
    
        for (uint32_t i=0; i < internalTrees.size() ; i++) {
          passUpstream(mRank, internalTrees[i], buff, size);
        }
     }
  }
  else
      std::cout << "Incorrect Direction!!\n";
  return 1;
}

int ParallelCommunicator::connect(GraphID id, uint8_t direction, 
                                   InputStream* stream)
{
 enum directionType {DOWNSTREAM, UPSTREAM};
 if (direction == DOWNSTREAM)
    mInputStreamMapDownstream[id] = stream;
 else if (direction == UPSTREAM)
    mInputStreamMapUpstream[id] = stream;
 else
    std::cout << "Inconsistent Stream Direction\n";

  return 1;
}

uint32_t ParallelCommunicator::passUpstream(RankID destRank, GraphID destTree, 
                                            const char* buffer, uint32_t size) {

  //destRank = getTreeMPIrank(destTree, mGatherFlow);

  //Setting destination field in the buffer
  enum directionType {DOWNSTREAM, UPSTREAM};
  *(GraphID*)(buffer + sizeof(uint32_t)) = destTree;

  //std::cout << "\nMPI SEND UPSTREAM :: Destination: " << destRank
  //  << " Rank: " << mRank 
  //  << " Size:: " << *(uint32_t*)buffer 
  //  << " Destination Tree :: " << *(GraphID*)(buffer + sizeof(uint32_t))
  //  << " Direction :: " << (int)*(uint8_t*)(buffer + sizeof(uint32_t) 
  //                                          + sizeof(GraphID))
  //  << " Token Count:: " << *(uint32_t*)(buffer + sizeof(uint32_t) + 
  //                                       sizeof(GraphID) + sizeof(uint8_t) 
  //                                       + *(uint32_t*)buffer)
  //  << std::endl;

  // WRITE TO FILE HERE
  createFile(destRank, buffer, size, UPSTREAM);
  return 1;
}

int ParallelCommunicator::send(const std::vector<GraphID>& destinations,
                               const char* buffer, uint32_t size) {

  uint8_t direction = -1;
  enum directionType {DOWNSTREAM, UPSTREAM};

  if (destinations.size() == 1) {
    RankID destRank = mTreeMapSend[destinations[0]];
    direction = DOWNSTREAM;

    //Setting destination field in the buffer
    *(GraphID*)(buffer + sizeof(uint32_t)) = destinations[0];
    *(uint8_t*)(buffer + sizeof(uint32_t) + sizeof(GraphID)) = direction;

    //std::cout << "\nMPI SEND DOWNSTREAM :: Destination: " << destRank 
    //  << " Rank: " << mRank 
    //  << " Size:: " << *(uint32_t*)buffer 
    //  << " Destination Tree :: " << *(GraphID*)(buffer + sizeof(uint32_t))
    //  << " Direction :: " << (int)*(uint8_t*)(buffer + sizeof(uint32_t) + 
    //                                          sizeof(GraphID))
    //  << " Token Count:: " << *(uint32_t*)(buffer + sizeof(uint32_t) + 
    //                                       sizeof(GraphID) + sizeof(uint8_t) 
    //                                       + *(uint32_t*)buffer)
    //  << std::endl;
  
    if (destRank != mRank) {
       MPI_Send((void*)buffer, size, MPI_BYTE, destRank, 0, MPI_COMM_WORLD);
       mDownStreamRank = destRank;
       return 1;
    }

    sendInternal(buffer, size);
    //if (destinations.size() > 0) {
    //MPI_Send((void*)buffer, size, MPI_BYTE, destRank, 0, MPI_COMM_WORLD);
    //MPI_Isend((void*)buffer, size, MPI_BYTE, destRank, 0, 
    //          MPI_COMM_WORLD, &request);
    //}
  }
  else {

    direction = UPSTREAM;
    std::map<GraphID, RankID>::iterator destRankIt;
    uint32_t destRank;
    std::vector<uint32_t> internalTrees; 
    for (uint32_t i=0 ; i < destinations.size(); i++) {
      
       destRankIt = mTreeMapSend.find(destinations[i]);
       if (destRankIt == mTreeMapSend.end()) {
         destRank = getTreeMPIrank(destinations[i], mGatherFlow, SPATIAL);
         //destRank = getTreeMPIrank(destinations[i], mGatherFlow, MODULO);
         mTreeMapSend[destinations[i]] = destRank;
       }
       else 
         destRank = destRankIt->second;

       // First we send external trees then internal trees
       // Following if to identify internal trees
       if (destRank == mRank) {
         internalTrees.push_back(destinations[i]);
         //std::cout << " Internal Tree " << destinations[i] << "   updates... Rank:: " << mRank << std::endl; 
       }
       else {  
         //Setting destination field in the buffer
         *(GraphID*)(buffer + sizeof(uint32_t)) = destinations[i];
         *(uint8_t*)(buffer + sizeof(uint32_t) + sizeof(GraphID)) = direction;

         //std::cout << "\nMPI SEND :: Destination: " << destinations[i]
         //  << " Rank: " << mRank 
         //  << " Size:: " << *(uint32_t*)buffer 
         //  << " Destination Tree :: " << *(GraphID*)(buffer + sizeof(uint32_t))
         //  << " Direction :: " << (int)*(uint8_t*)(buffer + sizeof(uint32_t) + 
         //                                          sizeof(GraphID))
         //  << " Token Count:: " << *(uint32_t*)(buffer + sizeof(uint32_t) + 
         //                           sizeof(GraphID) + sizeof(uint8_t) + 
         //                           *(uint32_t*)buffer)
         //  << std::endl;

         passUpstream(destRank, destinations[i], buffer, size);
         //std::cout << "Rassing external...tree :: " << destinations[i]<< "\n";
       }
/*
       if (destinations[i] == mRank) {

         sendInternal(buffer, size);
       }
       else {   
         //MPI_Isend((void*)buffer, size, MPI_BYTE, destinations[i], 0, 
         //           MPI_COMM_WORLD, &request);
         MPI_Send((void*)buffer, size, MPI_BYTE, destinations[i], 0, 
                    MPI_COMM_WORLD);

       //std::cout << "SCATTER RANK:: " << destinations[i] << std::endl;
       } */
    }

    // now send internal trees
    for (uint32_t i=0; i < internalTrees.size() ; i++) {
      *(GraphID*)(buffer + sizeof(uint32_t)) = internalTrees[i];
      *(uint8_t*)(buffer + sizeof(uint32_t) + sizeof(GraphID)) = direction;
      passUpstream(mRank, internalTrees[i], buffer, size);
    //  std::cout << "Passing Internal...tree " << internalTrees[i] << " \n";
    }
    
  }
  
  return 1;
}

int ParallelCommunicator::sendToFile(const std::vector<GraphID>& destinations,
                                     const char* buffer, uint32_t size) {

  uint8_t direction = -1;
  enum directionType {DOWNSTREAM, UPSTREAM};

  if (destinations.size() == 1) {
    RankID destRank = mTreeMapSend[destinations[0]];
    direction = DOWNSTREAM;

    //Setting destination field in the buffer
    *(GraphID*)(buffer + sizeof(uint32_t)) = destinations[0];
    *(uint8_t*)(buffer + sizeof(uint32_t) + sizeof(GraphID)) = direction;

   // std::cout << "\nMPI SEND DOWNSTREAM :: Destination: " << destRank 
   //   << " Rank: " << mRank 
   //   << " Size:: " << *(uint32_t*)buffer 
   //   << " Destination Tree :: " << *(GraphID*)(buffer + sizeof(uint32_t))
   //   << " Direction :: " << (int)*(uint8_t*)(buffer + sizeof(uint32_t) + 
   //                                           sizeof(GraphID))
   //   << " Token Count:: " << *(uint32_t*)(buffer + sizeof(uint32_t) + 
   //                                        sizeof(GraphID) + sizeof(uint8_t) 
   //                                        + *(uint32_t*)buffer)
   //   << std::endl;
  
    // WRITE FILE TO DISK
    createFile(destRank, buffer, size, direction);
  }
  else {

    direction = UPSTREAM;
    std::map<GraphID, RankID>::iterator destRankIt;
    uint32_t destRank;
    std::vector<uint32_t> internalTrees; 
    for (uint32_t i=0 ; i < destinations.size(); i++) {
      
       destRankIt = mTreeMapSend.find(destinations[i]);
       if (destRankIt == mTreeMapSend.end()) {
         destRank = getTreeMPIrank(destinations[i], mGatherFlow, SPATIAL);
         //destRank = getTreeMPIrank(destinations[i], mGatherFlow, MODULO);
         mTreeMapSend[destinations[i]] = destRank;
       }
       else 
         destRank = destRankIt->second;

       // First we send external trees then internal trees
       // Following if to identify internal trees
       if (destRank == mRank) {
         internalTrees.push_back(destinations[i]);
         //std::cout << " Internal Tree " << destinations[i] << "   updates... Rank:: " << mRank << std::endl; 
       }
       else {  
         //Setting destination field in the buffer
         *(GraphID*)(buffer + sizeof(uint32_t)) = destinations[i];
         *(uint8_t*)(buffer + sizeof(uint32_t) + sizeof(GraphID)) = direction;

         //std::cout << "\nMPI SEND :: Destination: " << destinations[i]
         //  << " Rank: " << mRank 
         //  << " Size:: " << *(uint32_t*)buffer 
         //  << " Destination Tree :: " << *(GraphID*)(buffer + sizeof(uint32_t))
         //  << " Direction :: " << (int)*(uint8_t*)(buffer + sizeof(uint32_t) + 
         //                                          sizeof(GraphID))
         //  << " Token Count:: " << *(uint32_t*)(buffer + sizeof(uint32_t) + 
         //                           sizeof(GraphID) + sizeof(uint8_t) + 
         //                           *(uint32_t*)buffer)
         //  << std::endl;

         passUpstream(destRank, destinations[i], buffer, size);
         //std::cout << "Rassing external...tree :: " << destinations[i]<< "\n";
       }
/*
       if (destinations[i] == mRank) {

         sendInternal(buffer, size);
       }
       else {   
         //MPI_Isend((void*)buffer, size, MPI_BYTE, destinations[i], 0, 
         //           MPI_COMM_WORLD, &request);
         MPI_Send((void*)buffer, size, MPI_BYTE, destinations[i], 0, 
                    MPI_COMM_WORLD);

       //std::cout << "SCATTER RANK:: " << destinations[i] << std::endl;
       } */
    }

    // now send internal trees
    for (uint32_t i=0; i < internalTrees.size() ; i++) {
      *(GraphID*)(buffer + sizeof(uint32_t)) = internalTrees[i];
      *(uint8_t*)(buffer + sizeof(uint32_t) + sizeof(GraphID)) = direction;
      passUpstream(mRank, internalTrees[i], buffer, size);
    //  std::cout << "Passing Internal...tree " << internalTrees[i] << " \n";
    }
    
  }
  
  return 1;
}

void ParallelCommunicator::createFile(int dest_rank, const char* buffer, 
                                      int size, int direction) {

  char filename[64];
  enum directionType {DOWNSTREAM, UPSTREAM};
  int dest_tree = *(GraphID*)(buffer + sizeof(uint32_t));
  if (direction == UPSTREAM) {
    sprintf(filename, "../../data/AMT_s_rank_%d_d_rank_%d_d_tree%d_up.dat",
          mRank, dest_rank, dest_tree);
  }
  else { 
    sprintf(filename, "../../data/AMT_s_rank_%d_d_rank_%d_d_tree%d_down.dat",
          mRank, dest_rank, dest_tree);
  }

  char* file_buffer = (char*)malloc(sizeof(int)+size);
  memcpy(file_buffer+sizeof(int), buffer, size);
  *(int*)file_buffer = size;

  FILE *fp = fopen(filename, "wb");
  fwrite(file_buffer, sizeof(char), sizeof(int) + size, fp);
  fclose(fp);

}
int ParallelCommunicator::readMsgFile(int source_rank, char** buffer, 
                                       int size, int tree_id) {

  char filename[64];
  enum directionType {DOWNSTREAM, UPSTREAM};
  int direction = DOWNSTREAM;
  if (direction == UPSTREAM) {
    sprintf(filename, "../../data/AMT_s_rank_%d_d_rank_%d_d_tree%d_up.dat",
          source_rank, mRank, tree_id);
  }
  else { 
    sprintf(filename, "../../data/AMT_s_rank_%d_d_rank_%d_d_tree%d_down.dat",
          source_rank, mRank, tree_id);
  }
  
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    std::cout << "Unable to open file\n";
    return -1;
  }

  char* buff = (char*)malloc(sizeof(char)*size);
  fread(buff, sizeof(char), size, fp);
  fclose(fp);

  int msg_size = *(int*)buff;
  char* msg_buffer = (char*)malloc(sizeof(char)*msg_size);
  memcpy(msg_buffer, buff+sizeof(int), msg_size);

  free(buff);

  *buffer = msg_buffer;
  return 1;
}
