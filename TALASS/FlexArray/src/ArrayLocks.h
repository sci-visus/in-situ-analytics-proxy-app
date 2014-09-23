/***********************************************************************
*
* Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
* Produced at the Lawrence Livermore National Laboratory  
* Written by bremer5@llnl.gov 
* OCEC-08-107
* All rights reserved.  
*   
* This file is part of "Streaming Topological Graphs Version 1.0."
* Please also read BSD_ADDITIONAL.txt.
*   
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*   
* @ Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the disclaimer below.
* @ Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the disclaimer (as noted below) in
*   the documentation and/or other materials provided with the
*   distribution.
* @ Neither the name of the LLNS/LLNL nor the names of its contributors
*   may be used to endorse or promote products derived from this software
*   without specific prior written permission.
*   
*  
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
* LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING
*
***********************************************************************/

#ifndef FA_ARRAYLOCKS_H
#define FA_ARRAYLOCKS_H

#include <vector>
#include "BlockedArray.h"
#include "AtomicLock.h"

namespace FlexArray {


//! ArrayLocks provide high performance thread-locks for FlexArrays
/*! The ArrayLocks class provides high performance element wise of block-wise
 *  thread locking mechanisims for FlexArrays. The class provides a single
 *  per-instance lock designed to be used, for example, when resizing an
 *  array. Furthermore, depending on the given thread-bits size S there exist
 *  another thread-lock for every 2^S elements. The thread-locks are implemented
 *  as byte-sized locks manipulated through compare and swap operations. 
 */
class ArrayLocks : public BlockedArray<AtomicLock,unsigned int>
{
public:

  using Array<AtomicLock,unsigned int>::IndexType;

  //! The number of locks per allocated block
  static const uint8_t sInternalBlockBits = 31;

  //! Default constructor
  /*! Default constructor creating a array locks with one thread lock for each
   *  2^lock_bits elements and a block size of 2^block_bits
   *  @param lock_bits: The number of bits to shift an element index to reach its lock's index
   */
  ArrayLocks(uint8_t lock_bits=0);

  //! Copy constructor
  ArrayLocks(const ArrayLocks& array);

  //! Destructor
  virtual ~ArrayLocks();

  //! Lock an element or corresponding block
  /*! Lock the element with the given index or the block it is contained in.
   *  @param index: The index of the element that should be locked
   *  @return true if the lock has been aquired; false otherwise
   */
  virtual void lock(IndexType index) {this->mArray[index>>mLockBlockBits][(index >> mLockBits) & mLockBlockMask].acquire();}

  //! Unlock the element or corresponding block
  virtual void unlock(IndexType index) {this->mArray[index>>mLockBlockBits][(index >> mLockBits) & mLockBlockMask].release();}

protected:

  //! Internal resize bypassing the thread lock
  int internalResize(IndexType size) {return BlockedArray<AtomicLock,unsigned int>::resize(size >> mLockBits);}

private:

  //! The number of bits used to address the elements sharing lock
  const uint8_t mLockBits;

  //! The number of bits used to address the elements within a block of locks
  const uint8_t mLockBlockBits;

  //! The bitmask used to extract the final lock-index
  const IndexType mLockBlockMask;
};

} // end of namespace  
#endif

