/*
 * SeriallInputStream.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef SERIALLINPUTSTREAM_H_
#define SERIALLINPUTSTREAM_H_

#include "TopoInputStream.h"

//! Declaration of the communicator interface
class TopoCommunicator;

//! An input stream that pulls data from a communicator
class SerialInputStream : public TopoInputStream
{
public:

  //! Default constructor
  SerialInputStream(GraphID id, uint32_t outstanding);

  //! Destructor
  virtual ~SerialInputStream() {}

private:

  //! The element id we represent
  const GraphID mId;

};

#endif /* SERIALLINPUTSTREAM_H_ */
