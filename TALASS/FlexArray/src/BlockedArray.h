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


#ifndef FA_BLOCKEDARRAY_H
#define FA_BLOCKEDARRAY_H

#include <vector>
#include <cstdlib>
#include "Array.h"

namespace FlexArray {

//! A dynamic array of arbitrary elements based on blocks
/*! A BlockedArray is a resizeable array of arbitrary elements. The
 *  major difference between a std::vector is that the BlockedArray
 *  allocates its memory in blocks. As a result the memory space is
 *  *not* continuous. However, the advantage is that pointers into the
 *  array remain valid even after resizing the array. This class also
 *  forms an interface for more advanced dynamic arrays for example
 *  OOCArray which use memory mapped files for storage
 */
template <class ElementClass, typename IndexType>
class BlockedArray : public Array<ElementClass,IndexType>
{
public:

  //! Number of bits used for addressing a block
  static const uint8_t sBlockBits = 18;

  //! Number of elements in each block
  static const IndexType sBlockSize = 2 << sBlockBits;

  //! Bitmask to extract the local block index
  static const IndexType sBlockMask = sBlockSize - 1;

  //! Internal iterator iterating over all active elements
  class iterator
  {
  public:

    //! Allow the begin() function to use the special constructor
       friend class BlockedArray;
	   //friend iterator BlockedArray::begin();

    //! Allow the end() function to use the special constructor
   // friend iterator BlockedArray::end();

    //! Default constructor
    iterator() : mIndex(0), mSource(NULL) {}

    ~iterator() {}

    //! Cast the iterator to pointer to the current element
    operator ElementClass*() {return &mSource->get(mIndex);}

    //! Advance the iterator
    iterator& operator++(int i) {mIndex++;return *this;}

    //! Assignement operator between iterators
    iterator& operator=(const iterator& it)
    {mIndex=it.mIndex;mSource=it.mSource;return *this;}

    //! Comparison operator
    bool operator==(const iterator& it) {return (mIndex == it.mIndex);}

    //! Comparison operator
    bool operator!=(const iterator& it) {return (mIndex != it.mIndex);}

    //! Return a pointer to the current element
    ElementClass* operator->() {return &mSource->get(mIndex);}

    //! Return a reference to the current element
    ElementClass& operator*() {return mSource->get(mIndex);}

    //! Return the index of the current element
    operator IndexType() const {return mIndex;}

  protected:

    //! The current index
    IndexType mIndex;

    //! The parent MappedArrayBase this iterator is referring to
    BlockedArray* mSource;

    //! Private constructor to construct begin() and end()
    iterator(IndexType index, BlockedArray* a) : mIndex(index), mSource(a) {}
  };

  //! Internal iterator iterating over all active elements
  class const_iterator
  {
  public:

    //! Allow the begin() function to use the special constructor
    friend class BlockedArray;
	//friend const_iterator BlockedArray::begin() const;

    //! Allow the end() function to use the special constructor
    //friend const_iterator BlockedArray::end() const;

    //! Default constructor
    const_iterator() : mIndex(0), mSource(NULL) {}

    ~const_iterator() {}

    //! Cast the iterator to pointer to the current element
    operator const ElementClass*() {return &mSource->get(mIndex);}

    //! Advance the iterator
    const_iterator& operator++(int i) {mIndex++;return *this;}

    //! Assignement operator between iterators
    const_iterator& operator=(const const_iterator& it)
    {mIndex=it.mIndex;mSource=it.mSource;return *this;}

    //! Comparison operator
    bool operator==(const const_iterator& it) {return (mIndex == it.mIndex);}

    //! Comparison operator
    bool operator!=(const const_iterator& it) {return (mIndex != it.mIndex);}

    //! Return a pointer to the current element
    const ElementClass* operator->() {return &mSource->get(mIndex);}

    //! Return a reference to the current element
    const ElementClass& operator*() {return mSource->get(mIndex);}

    //! Return the index of the current element
    operator IndexType() const {return mIndex;}

  protected:

    //! The current index
    IndexType mIndex;

    //! The parent MappedArrayBase this iterator is referring to
    const BlockedArray* mSource;

    //! Private constructor to construct begin() and end()
    const_iterator(IndexType index, const BlockedArray* a) : mIndex(index), mSource(a) {}
  };


  //! Default constructor
  BlockedArray(uint8_t block_bits=sBlockBits);
  
  //! Copy constructor
  BlockedArray(const BlockedArray& array);

  //! Destructor
  virtual ~BlockedArray();
  
  //! Return an iterator to the first element
  iterator begin() {return iterator(0,this);}

  //! Return a const_iterator to the first element
  const_iterator begin() const {return const_iterator(0,this);}

  //! Return an iterator pointing to after the last element
  iterator end() {return iterator(mNE,this);}

  //! Return a const_iterator pointing to after the last element
  const_iterator end() const {return const_iterator(mNE,this);}

  //! Return a reference to the element of index i 
  virtual ElementClass& at(IndexType i);

  //! Return a const reference to the element of index i 
  virtual const ElementClass& at(IndexType i) const;

  //! Return a reference to the element of index i
  virtual ElementClass& operator[](IndexType i) {return at(i);}

  //! Return a reference to the element of index i
  virtual const ElementClass& operator[](IndexType i) const {return at(i);}

  //! Return a reference to the last element
  virtual ElementClass& back() {return at(mNE-1);}
  
  //! Return a const reference to the last element
  virtual const ElementClass& back() const {return at(mNE-1);}  

  //! Add the given element to the array if necessary grow the array
  virtual void add(IndexType i, const ElementClass& element);

  //! Add the given element to the array if necessary grow the array
  virtual void insert(IndexType i, const ElementClass& element);

  //! Determine whether the given index is part of the array
  virtual bool contains(IndexType i) const {return i < mNE;}

  //! Return the current size
  virtual IndexType size() const {return mNE;}

  //! Return the current capacity
  virtual IndexType capacity() const {return mCE;}

  //! Indicate whether the array is full
  virtual bool full() const {return mNE == mCE;}

  //! Resize the array
  virtual int resize(IndexType size); 

  //! Add an element to the array and return its local index
  virtual IndexType push_back(const ElementClass& element);

  //! Dump the content of the array to disk in binary format
  int dumpBinary(FILE* output) const;

  //! read the contents of the array from disk
  int readBinary(FILE* input);

protected:
  
  //! The number of bits used to address a block
  const uint8_t mBlockBits;

  //! The current block size
  const IndexType mBlockSize;

  //! The bitmask to extract the block index
  const IndexType mBlockMask;

  //! The array of blocks
  std::vector<ElementClass*> mArray;
  
  //! The number of element current in the array
  IndexType mNE;

  //! The number of elements the array can hold
  IndexType mCE;

  //! Return a reference to the element of index i
  virtual ElementClass& get(IndexType i) {return mArray[i >> mBlockBits][i & mBlockMask];}

  //! Return a const reference to the element of index i
  virtual const ElementClass& get(IndexType i) const {return mArray[i >> mBlockBits][i & mBlockMask];}

};

template<class ElementClass, typename IndexType>
BlockedArray<ElementClass,IndexType>::BlockedArray(uint8_t block_bits)
  : Array<ElementClass,IndexType>(), mBlockBits(block_bits), mBlockSize(1 << block_bits),
    mBlockMask((1 << block_bits)-1), mNE(0), mCE(0)
{
}

template<class ElementClass, typename IndexType>
BlockedArray<ElementClass,IndexType>::BlockedArray(const BlockedArray<ElementClass,IndexType>& array) :
  Array<ElementClass,IndexType>(array), mBlockBits(array.mBlockBits), mBlockSize(array.mBlockSize),
  mBlockMask(array.mBlockMask)
{
  uint32_t i;

  mArray.resize(array.mArray.size());
  
  for (i=0;i<mArray.size();i++) {

    mArray[i] = new ElementClass[mBlockSize];
    memcpy(mArray[i],array.mArray[i],mBlockSize*sizeof(ElementClass));
  }

  mNE = array.mNE;
  mCE = array.mCE;
}

template<class ElementClass, typename IndexType>
BlockedArray<ElementClass,IndexType>::~BlockedArray()
{
}


template<class ElementClass, typename IndexType>
ElementClass& BlockedArray<ElementClass,IndexType>::at(IndexType i)
{
  return mArray[i >> mBlockBits][i & mBlockMask];
}


template<class ElementClass, typename IndexType>
const ElementClass& BlockedArray<ElementClass,IndexType>::at(IndexType i) const
{
  return mArray[i >> mBlockBits][i & mBlockMask];
}

template<class ElementClass, typename IndexType>
void BlockedArray<ElementClass,IndexType>::add(IndexType i, const ElementClass& element)
{
  if (mCE <= i)
    resize(i+1);
  else
    mNE = std::max(mNE,i+1);

  at(i) = element;
}

template<class ElementClass, typename IndexType>
void BlockedArray<ElementClass,IndexType>::insert(IndexType i, const ElementClass& element)
{
  if (mCE <= i)
    resize(i+1);
  else
    mNE = std::max(mNE,i+1);

  at(i) = element;
}

template<class ElementClass, typename IndexType>
int BlockedArray<ElementClass,IndexType>::resize(IndexType size)
{
  ElementClass* block;

  // First we check whether the array needs to shrink. While we can
  // remove a block. Note that the first half of the if-condition is
  // necessary to handle unsigned IndexTypes
  while ((mCE >= mBlockSize) && (size < (mCE - mBlockSize))) {

    mArray.pop_back();
    mCE -= mBlockSize;
  }


  // While we need to allocate more blocks
  while (size > mCE) {
  
    block = new (std::nothrow) ElementClass[mBlockSize];
    
    sterror(block==NULL,"Cannot allocate additional block\n");
  
    // Store the new block
    mArray.push_back(block);
    mCE += mBlockSize;
  }
    
  mNE = size;
  
  return 1;
}

template<class ElementClass, typename IndexType>
IndexType BlockedArray<ElementClass,IndexType>::push_back(const ElementClass& element)
{
  if (mNE == mCE) {
    resize(mNE+1);
    at(mNE-1) = element;
  }
  else { 
    at(mNE++) = element;
  }
  
  return mNE-1;
}



template<class ElementClass, typename IndexType>
int BlockedArray<ElementClass,IndexType>::dumpBinary(FILE* output) const
{
  IndexType count = 0;

  while (count < mNE) {

    if (mNE - count >= mBlockSize) 
      fwrite(mArray[count >> mBlockBits],sizeof(ElementClass),mBlockSize,output);
    else 
      fwrite(mArray[count >> mBlockBits],sizeof(ElementClass),mNE-count,output);

    count += mBlockSize;
  }
  
  return 1;
}

template<class ElementClass, typename IndexType>
int BlockedArray<ElementClass,IndexType>::readBinary(FILE* input)
{
  ElementClass buffer[1024];
  uint32_t size;
  uint32_t count;
  uint32_t i;

  count = 0;

  size = fread(buffer,sizeof(ElementClass),1024,input);
  
  while (size > 0) {
    
    resize(count + size);

    for (i=0;i<size;i++) {
      this->at(count++) = buffer[i];
    }

    size = fread(buffer,sizeof(ElementClass),1024,input);
  }
 
  return 1;
}
  
} // namespace FlexArray
  

#endif
