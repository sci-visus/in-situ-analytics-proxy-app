/*
 * SortedInputStream.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: landge1
 */

#include "SortedInputStream.h"
#include <iostream>

SortedInputStream::iterator::iterator() : mStream(NULL)
{
}

SortedInputStream::iterator::iterator(SortedInputStream* stream) : mStream(stream)
{

  // We first pull all the messages for this stream
  while (mStream->outstanding() != 0) {
    mStream->pull();
  }

  // At this point we assume we have a valid message
  sterror(mStream->front()==NULL,"We should have at least one message to process");

  // we now for the priority queue of the highest vertices from all messages
  mStream->initializePriorityQ();

  mIt = mStream->highVertex();
}

SortedInputStream::iterator& SortedInputStream::iterator::operator++(int i)
{
  // If this is an invalid iterator simply return
  if (mStream == NULL)
    return *this;

  // Check for token in edge queue
  if (mStream->mEdgeQ.size() > 0) {
    //if (mStream->mEdgeQ.size() > 2)
    //  std::cout << "Warning: vertex has more than two parents...\n";

    mIt = (mStream->mEdgeQ.front());
    mStream->mEdgeQ.pop();
    return *this;
  }
  
  // if no element in edge queue, pick highest vertex from vertex priority Q
  if (!mStream->mVertexPriorityQ.empty()) {
    mIt = mStream->mVertexPriorityQ.top();
    mStream->mVertexPriorityQ.pop();

    *(mStream->mHighVertex) = mIt;
    
    // We add the edges of the vertex in the edge queue
    mIt++;
    while (mIt.type() == EDGE) {
      // Since these iterators are custom iterators, they require the following
      // weird way of handling. 
      // TODO: a better job of the following code after the deadline
      *(mStream->mEdgeIter) = mIt;
      mStream->mEdgeQ.push(*(mStream->mEdgeIter));
      mIt++;
    }

    // Add next vertex from this iterator to priority queue
    if (mIt.type() != EMPTY) {
      if (mIt.type() == EDGE) 
        std::cout << "Trying to add edge token to vertex priority queue..\n";
      
      mStream->mVertexPriorityQ.push(mIt);
    }
    
    mIt = *(mStream->mHighVertex);
  }
  else {
    mIt++;
    if (mIt.type() == EMPTY) {
      return *this;
    }
    mEnd = mIt;
  }
  // if no data available clean up

  return *this;
}

bool SortedInputStream::iterator::operator==(const iterator& it) const
{
  return ((mStream == it.mStream) && (mIt == it.mIt) && (mEnd == it.mEnd));
}

bool SortedInputStream::iterator::operator!=(const iterator& it) const
{
  return ((mStream != it.mStream) || (mIt != it.mIt) || (mEnd != it.mEnd));
}



SortedInputStream::SortedInputStream(uint32_t outstanding) : mOutstanding(outstanding)
{
}

SortedInputStream::~SortedInputStream()
{
  std::deque<TokenInputBuffer*>::iterator it;
  for (it=mMessages.begin();it!=mMessages.end();it++)
    delete *it;

  mMessages.clear();
}

int SortedInputStream::push(char* buffer)
{
  //fprintf(stderr,"Pushing message to stream %p\n",buffer);
  mMessages.push_back(new TokenInputBuffer(buffer));

  if (lastType() == Token::EMPTY)
    mOutstanding--;

  return 1;
}

Token::TokenType SortedInputStream::lastType() const
{
  sterror(mMessages.empty(),"There exists no message to query");

  return mMessages.back()->lastType();
}

void SortedInputStream::initializePriorityQ() {

  mEdgeIter = new TokenInputBuffer::iterator();
  mHighVertex = new TokenInputBuffer::iterator();

  TokenInputBuffer* tp;
  TokenInputBuffer::iterator it;
  std::deque<TokenInputBuffer*>::iterator it_dq;
  for (it_dq = mMessages.begin(); it_dq != mMessages.end(); it_dq++) {
    tp = *it_dq;
    it = tp->begin();
    if (it.type() != EMPTY)
      mVertexPriorityQ.push(it);
  } 

  if (mVertexPriorityQ.empty()) {
    if (it.type() == EMPTY) {
      //std::cout << "Pushing empty token!\n";
      mVertexPriorityQ.push(it);
    }
  }
}

TokenInputBuffer* SortedInputStream::front() 
{
  if (mMessages.empty())
    return NULL;
  else
    return mMessages.front();
}

TokenInputBuffer::iterator SortedInputStream::highVertex()
{
    return mVertexPriorityQ.top();
}

