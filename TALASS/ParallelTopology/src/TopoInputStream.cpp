/*
 * TopoInputStream.cpp
 *
 *  Created on: Apr 13, 2012
 *      Author: bremer5
 */

#include "TopoInputStream.h"
#include <iostream>

TopoInputStream::iterator::iterator() : mStream(NULL)
{
}

TopoInputStream::iterator::iterator(TopoInputStream* stream) : mStream(stream)
{
  // If we have no current data
  if (mStream->front() == NULL) {

    // If we are not waiting for more data
    if (mStream->outstanding() == 0) {
      // We simply return since as a result mIt == mEnd which signals
      // that there are no more tokens to come
      return;
    }

    // Otherwise, we attempt to pull more data
    mStream->pull(); // attempt to pull more data
  }

  // At this point we assume we have a valid message
  sterror(mStream->front()==NULL,"We should have at least one message to process");

  mIt = mStream->front()->begin();
  mEnd = mStream->front()->end();
}

TopoInputStream::iterator& TopoInputStream::iterator::operator++(int i)
{
  // If this is an invalid iterator simply return
  if (mStream == NULL)
    return *this;

  // Advance the iterator for this message
  mIt++;

  //if (mIt.mIndex == 91)
  //fprintf(stderr,"break\n");

  // IF this was the end of this message
  if (mIt == mEnd) {

    // Delete the message and pop it from the queue
    delete mStream->front();
    mStream->mMessages.pop_front();

    // If there currently is no other message but we are expecting more data
    if ((mStream->front() == NULL) && (mStream->outstanding() > 0))
      mStream->pull(); // we wait for it

    // If there now exists another message
    if (mStream->front() != NULL) {
      // Reset the internal iterators
      mIt = mStream->front()->begin();
      mEnd = mStream->front()->end();

    }
    else { // We are done with processing
      sterror(mStream->outstanding()!=0,"We should be done");

      // Reset the internal iterators
      mIt = mEnd = TokenInputBuffer::iterator();

      // and invalidate the stream
      mStream = NULL;
    }
  }

  return *this;
}

bool TopoInputStream::iterator::operator==(const iterator& it) const
{
  return ((mStream == it.mStream) && (mIt == it.mIt) && (mEnd == it.mEnd));
}

bool TopoInputStream::iterator::operator!=(const iterator& it) const
{
  return ((mStream != it.mStream) || (mIt != it.mIt) || (mEnd != it.mEnd));
}



TopoInputStream::TopoInputStream(uint32_t outstanding) : mOutstanding(outstanding)
{
}

TopoInputStream::~TopoInputStream()
{
  std::deque<TokenInputBuffer*>::iterator it;
  for (it=mMessages.begin();it!=mMessages.end();it++)
    delete *it;

  mMessages.clear();
}

int TopoInputStream::push(char* buffer)
{
  //fprintf(stderr,"Pushing message to stream %p\n",buffer);
  mMessages.push_back(new TokenInputBuffer(buffer));

  if (lastType() == Token::EMPTY)
    mOutstanding--;

  return 1;
}

Token::TokenType TopoInputStream::lastType() const
{
  sterror(mMessages.empty(),"There exists no message to query");

  return mMessages.back()->lastType();
}


TokenInputBuffer* TopoInputStream::front()
{
  if (mMessages.empty())
    return NULL;
  else
    return mMessages.front();
}

