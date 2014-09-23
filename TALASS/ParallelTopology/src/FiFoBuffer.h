/*
 * FiFoBuffer.h
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#ifndef FIFOBUFFER_H_
#define FIFOBUFFER_H_
#include <stdio.h>
#include <vector>
#include <memory.h>
#include "TalassConfig.h"
#include "DistributedDefinitions.h"

//! A FIFO buffer
/*! This class implements a FIFO buffer with the usual push, pop,
 *  and top interface. However, unlike the std::dqueue it guarantees
 *  that individual pushes are contiguous in memory.
 */
class FiFoBuffer
{
public:

  //! The default read buffer size;
  static const uint32_t sDefaultBufferSize;

  //! Default constructor
  FiFoBuffer();

  //! Destructor
  ~FiFoBuffer();

  //! Add the given number of bytes at the "end" of the buffer
  /*! Add the given piece of memory to the "end" of the buffer. However,
   *  the function guarantees that this block of memory will be contiguous. I
   * @param tokens: pointer to the start of the memory buffer
   * @param size: size of the buffer in bytes
   * @return: 1 if successful; 0 otherwise.
   */
  int push(char* tokens, uint32_t size);

  //! Remove the given number of bytes from the "beginning" of the buffer
  /*! Remove the given number of bytes from the internal buffer. The
   *  function will assume that this request does not join two pushes and
   *  and if it does may return an error
   * @param size: the number of bytes to be removed
   * @return 1 if successfull; 0 otherwise.
   */
  int pop(uint32_t size);

  //! Return a pointer to the first element or NULL
  char *top();


  //! Determine whether the buffer is empty
  bool empty() const {return mFront->empty();}

private:

  //! The first part of the buffer
  std::vector<char>* mFront;

  //! The second part of the buffer
  std::vector<char>* mBack;

  //! The index of the current head
  uint32_t mHead;
};

#endif /* FIFOBUFFER_H_ */
