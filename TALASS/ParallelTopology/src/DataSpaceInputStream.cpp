/*
 * DataSpaceInputStream.cpp
 *
 *  Created on: Apr 12, 2012
 *      Author: bremer5
 */

#include "DataSpaceInputStream.h"

DataSpaceInputStream::DataSpaceInputStream(const DataElement& data)
: TopoInputStream(1)
{
  char* buffer = new char(data.descriptor.size);
  memcpy(buffer,data.data,data.descriptor.size);
  push(buffer);
}

