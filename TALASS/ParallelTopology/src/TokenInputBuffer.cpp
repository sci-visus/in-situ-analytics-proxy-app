/*
 * TokenInputBuffer.cpp
 *
 *  Created on: Jun 17, 2012
 *      Author: bremer5
 */

#include "TokenInputBuffer.h"
#include <iostream>
#include <cstdlib>

using namespace Token;

TokenInputBuffer::iterator& TokenInputBuffer::iterator::operator++(int i)
{
  sterror(mIndex>=mCount,"Access error iterator out of range:: %d %d", mIndex, mCount);

  switch (mTypes[mIndex]) {
    case VERTEX:
      mToken += ((const VertexToken*)mToken)->size();
      break;
    case EDGE:
      mToken += ((const EdgeToken*)mToken)->size();
      break;
    case FINAL:
      mToken += ((const FinalToken*)mToken)->size();
      break;
    case SEG:
      mToken += ((const SegToken*)mToken)->size();
      break;
    case EMPTY:
      mToken += ((const EmptyToken*)mToken)->size();
      break;
  }
  mIndex++;

  return *this;
}

bool TokenInputBuffer::iterator::operator==(const TokenInputBuffer::iterator it) const
{
  return ((mTypes == it.mTypes) && (mIndex == it.mIndex) && (mToken == it.mToken));
}

bool TokenInputBuffer::iterator::operator!=(const TokenInputBuffer::iterator it) const
{
  return ((mTypes != it.mTypes) && (mIndex != it.mIndex) && (mToken != it.mToken));
}


TokenInputBuffer::iterator::iterator(const char *buffer, bool fast_forward)
: mSize(*(uint32_t*)buffer),
  mDest(*(GraphID*)(buffer + sizeof(uint32_t))),
  mDirection(*(uint8_t*)(buffer + sizeof(uint32_t) + sizeof(GraphID))),
  mToken(buffer + sizeof(uint32_t) + sizeof(GraphID) + sizeof(uint8_t)),
  mCount(*(uint32_t*)(mToken + mSize)),
  mTypes((uint8_t*)(mToken + mSize + sizeof(uint32_t))),
  mIndex(0)
{
  //std::cout << " Size :: " << mSize
  //          << " Count :: " << mCount
  //          << " Dest :: " << mDest
  //          << std::endl;
  //printf ("Buffer address:: %p\n", buffer);
  if (fast_forward) {
    mIndex = mCount;
    mToken = mToken + mSize;
  }
}

TokenInputBuffer::iterator::iterator(const TokenInputBuffer::iterator& it) :
    mSize(it.mSize), mDest(it.mDest), mDirection(it.mDirection), mToken(it.mToken),
    mCount(it.mCount), mTypes(it.mTypes), mIndex(it.mIndex)
{
}

Token::TokenType TokenInputBuffer::lastType() const
{
  const uint8_t* tokens = (const uint8_t*)(mBuffer + sizeof(uint32_t) + 
                           sizeof(GraphID) + sizeof(uint8_t) + *(const uint32_t*)(mBuffer));
  uint32_t count = *(uint32_t*)(tokens);

  return (Token::TokenType)tokens[count + sizeof(uint32_t) - 1];
}


