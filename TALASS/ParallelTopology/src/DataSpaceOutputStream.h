/*
 * DataSpaceOutputStream.h
 *
 *  Created on: Apr 4, 2012
 *      Author: bremer5
 */

#ifndef DATASPACEOUTPUTSTREAM_H_
#define DATASPACEOUTPUTSTREAM_H_

#include "TopoCommunicator.h"
#include "TopoOutputStream.h"
#include <list>

//! The data descriptor containing one chuck of data
struct DataDescriptor {

  int size;
  int rank;
  int timeStep;
  int type;
};

//! The data element containing a descriptor as well as the data
struct DataElement {

  void* data;

  DataDescriptor descriptor;
};

//! The stand in for a data list
typedef std::list<DataElement> DataList;



//! An output stream that will emulate a data space
class DataSpaceOutputStream : public TopoOutputStream
{
public:

  //! The global data list for testing purposes
  static DataList sDataList;

  //! Default constructor using an infinite buffer
  DataSpaceOutputStream(uint32_t rank);

  //! Destructor
  virtual ~DataSpaceOutputStream() {}

  //! Function to write .dot file to view Output Stream
  int write_to_dot(FILE* output);

  //! Flush the buffer to the stream
  int flush() {return TopoOutputStream::flush();}

private:

  //! The virtual rank of this output stream
  uint32_t mRank;


  //! Write the given buffer as a data descriptor
  virtual int write(const char* buffer, uint32_t size);

};


#endif /* DATASPACEOUTPUTSTREAM_H_ */
