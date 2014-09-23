/*
 * DataSpaceIputStream.h
 *
 *  Created on: Apr 12, 2012
 *      Author: bremer5
 */

#ifndef DATASPACEINPUTSTREAM_H_
#define DATASPACEINPUTSTREAM_H_

#include "DataSpaceOutputStream.h"
#include "TopoInputStream.h"

class DataSpaceInputStream : public TopoInputStream
{
public:

  //! Default constructor from a data element
  DataSpaceInputStream(const DataElement& data);

  //! Destructor
  ~DataSpaceInputStream() {}

};



#endif /* DATASPACEIPUTSTREAM_H_ */
