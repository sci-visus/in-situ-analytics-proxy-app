/*
 * Token.cpp
 *
 *  Created on: Jan 31, 2012
 *      Author: bremer5
 */

#include "Token.h"

namespace Token {

VertexToken::VertexToken(GlobalIndexType id, FunctionType f, MultiplicityType mult, MultiplicityType out) :
  mIndex(id), mValue(f), mMultiplicity(mult), mOutstanding(out)
{
}

EdgeToken::EdgeToken(GlobalIndexType v0, GlobalIndexType v1)
{
  mIndex[0] = v0;
  mIndex[1] = v1;
}

SegToken::SegToken(GlobalIndexType id, GlobalIndexType seg, FunctionType f) :
    mId(id), mSeg(seg), mFunction(f)
{
}


}
