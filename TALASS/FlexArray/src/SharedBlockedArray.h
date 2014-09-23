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

#include "BlockedArray.h"
#include "ArrayLocks.h"

namespace FlexArray {

template <class ElementClass, typename IndexType>
class SharedBlockedArray : public BlockedArray<ElementClass,IndexType>
{
public:

  //! Typedef to satisfy the compiler
  typedef BlockedArray<ElementClass,IndexType> BaseClass;

  //! Default constructor
  SharedBlockedArray(uint8_t block_bits=BaseClass::sBlockBits) : BlockedArray<ElementClass,IndexType>(block_bits), mLocks() {}

  //! Default destructor
  virtual ~SharedBlockedArray() {}
  
  //! Adapt the size of the array
  virtual int resize(IndexType size);

  //! Lock a certain element
  virtual void lock(IndexType index) {mLocks.lock(index);}

  //! Unlock a certain element
  virtual void unlock(IndexType index) {mLocks.unlock(index);}

private:

  //! The array of all necessary locks
  ArrayLocks mLocks;

  //! The global lock to protect against resizing
  AtomicLock mInstanceLock;

  //! Copy constructor 
  SharedBlockedArray(const SharedBlockedArray& array) {}
};


template <class ElementClass, typename IndexType>
int SharedBlockedArray<ElementClass, IndexType>::resize(IndexType size)
{
  mInstanceLock.acquire();

  BlockedArray<ElementClass,IndexType>::resize(size);
  
  mLocks.resize(size);

  mInstanceLock.release();

  return this->size();
}

}

