/*
 * TopoStream.cpp
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#include "TopoStream.h"

TopoStream::TopoStream(TopoCommunicator* com, GraphID gid) : mCommunicator(com), mId(gid)
{
  sterror(mCommunicator==NULL,"Did not get a valid communicator");
}
