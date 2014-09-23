/*
 * SerialOutputStream.h
 *
 *  Created on: Feb 13, 2012
 *      Author: bremer5
 */

#ifndef SERIALOUTPUTSTREAM_H_
#define SERIALOUTPUTSTREAM_H_

#include "TopoCommunicator.h"
#include "TopoOutputStream.h"

//! An output stream that pushes data to a communicator
class SerialOutputStream : public TopoOutputStream
{
public:

  //! Default constructor
  SerialOutputStream(const std::vector<GraphID>& destinations, TopoCommunicator* com);

  //! Destructor
  virtual ~SerialOutputStream() {}

private:

  //! The communicator from which to pull data
  TopoCommunicator* const mCommunicator;

  //! Write the given buffer to the communicator
  virtual int write(const char* buffer, uint32_t size);
};


#endif /* SERIALOUTPUTSTREAM_H_ */
