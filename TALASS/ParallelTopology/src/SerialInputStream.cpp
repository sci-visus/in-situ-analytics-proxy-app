/*
 * SerialInputStream.cpp
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#include "TopoCommunicator.h"
#include "SerialInputStream.h"
#include <iostream>

SerialInputStream::SerialInputStream(GraphID id, uint32_t outstanding)
: TopoInputStream(outstanding), mId(id)
{

}
