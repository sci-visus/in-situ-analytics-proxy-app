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


#ifndef FA_MAPPEDARRAY_H
#define FA_MAPPEDARRAY_H

#include <map>

#include "BlockedArray.h"


#ifdef WIN32
#define assert(a) ;
#endif


namespace FlexArray {

//! A dynamic array that keeps track of empty spaces
/*! As the name suggests the MappedArrayBase implements an array of
 *  arbitrary elements that can grow but not shrink. The key feature
 *  is that memmory is allocated in blocks rather than using a dynamic
 *  vector or malloc/realloc calls. This is crucial to maintain
 *  pointers to elements consistently during the resizing process. A
 *  MappedArrayBase assumes that it's elements conform to the
 *  MappedElement interface to store the local index as well as the
 *  skip list maintaining the empty spaces.
 */
template <class ElementClass, typename GlobalIndexType, typename LocalIndexType>
class MappedArrayBase : public BlockedArray<ElementClass,LocalIndexType>
{
public:
  
  //! Typedef to define the baseclass
  typedef BlockedArray<ElementClass,LocalIndexType> BaseClass;

  //! Typedef to define the map from global to local index space
  typedef std::map<GlobalIndexType,LocalIndexType> IndexMapType;

  static const LocalIndexType LNULL = (LocalIndexType)(-1);

  //! Internal iterator iterating over all active elements 
  class iterator
  {
  public:
    
    //! Allow the begin() function to use the special constructor
    friend class MappedArrayBase;
  
    //! Default constructor
    iterator() : mSource(NULL) {}

    //! Cast the iterator to pointer to the current element
    operator ElementClass*() {return &mSource->get(mIt->second);}

    //! Advance the iterator
    iterator& operator++(int i) {mIt++;return *this;}

    //! Assignement operator between iterators
    iterator& operator=(const iterator& it)
    {mIt=it.mIt; mSource=it.mSource;return *this;}
    
    //! Comparison operator
    bool operator==(const iterator& it) {return (mIt == it.mIt);}

    //! Comparison operator
    bool operator!=(const iterator& it) {return (mIt != it.mIt);}

    //! Return a pointer to the current element 
    ElementClass* operator->() {return &mSource->get(mIt->second);}
    
    //! Return a reference to the current element
    ElementClass& operator*() {return mSource->get(mIt->second);}
    
    //! Return the index of the current element
    operator GlobalIndexType() const {return mIt->first;}

  private:
    
    //! The internal iterator into the index map
    typename IndexMapType::iterator mIt;

    //! The parent MappedArrayBase this iterator is referring to
    MappedArrayBase* mSource;
    
    //! Private constructor to construct begin() and end()
    iterator(const typename IndexMapType::iterator& it,
             MappedArrayBase* a) : mIt(it), mSource(a) {}
  };
  
  //! Internal iterator iterating over all active elements
  class const_iterator
  {
  public:

    //! Friend declaration to allow acccess to private constructor
    friend class MappedArrayBase;

    //! Default constructor
    const_iterator() : mSource(NULL) {}

    //! Cast the iterator to pointer to the current element
    operator const ElementClass*() const {return &mSource->get(mIt->second);}

    //! Advance the iterator
    const_iterator& operator++(int i) {mIt++;return *this;}

    //! Assignment operator between iterators
    const_iterator& operator=(const const_iterator& it)
    {mIt=it.mIt;mSource=it.mSource;return *this;}

    //! Comparison operator
    bool operator==(const const_iterator& it) const {return (mIt == it.mIt);}

    //! Comparison operator
    bool operator!=(const const_iterator& it) const {return (mIt != it.mIt);}

    //! Return a pointer to the current element
    const ElementClass* operator->() const {return &mSource->get(mIt->second);}

    //! Return a reference to the current element
    const ElementClass& operator*() const {return mSource->get(mIt->second);}

    //! Return the index of the current element
    operator GlobalIndexType() const {return mIt->first;}

  private:

    //! The internal iterator into the index map
    typename IndexMapType::const_iterator mIt;

    //! The parent MappedArrayBase this iterator is referring to
    const MappedArrayBase* mSource;

    //! Private constructor to construct begin() and end()
    const_iterator(const typename IndexMapType::const_iterator& it,
                   const MappedArrayBase* a) : mIt(it), mSource(a) {}
  };


  //! Default constructor
  MappedArrayBase(uint8_t block_bits=BaseClass::sBlockBits);

  //! Destructor
  virtual ~MappedArrayBase();

  //! Return an iterator to the first element
  iterator begin() {return iterator(mIndexMap.begin(),this);}

  //! Return a const_iterator to the first element
  const_iterator begin() const {return const_iterator(mIndexMap.begin(),this);}

  //! Return an iterator pointing to after the last element
  iterator end() {return iterator(mIndexMap.end(),this);}

  //! Return a const_iterator pointing to after the last element
  const_iterator end() const {return const_iterator(mIndexMap.end(),this);}

  //! Return the number of active elements
  int elementCount() {return mIndexMap.size();}

  //! Return a reference to the element of index i
  virtual ElementClass& at(GlobalIndexType i);

  //! Return a const reference to the element of index i
  virtual const ElementClass& at(GlobalIndexType i) const;

  //! Resize the array
  virtual int resize(GlobalIndexType size);

  //! Determine whether the given index is part of the array
  virtual bool contains(GlobalIndexType i) const {return (findElement(i) != NULL);}

  //! Find the element with the given id
  /*! Search the index map for an element with the given id. If such
   *  an element exists return a pointer to it location,
   *  otherwise return NULL.
   *  @param id: The global id of the element we are looking for
   *  @return pointer to the element corresponding to id or NULL
   */
  ElementClass* findElement(GlobalIndexType id);

  //! Find the element with the given id
  /*! Search the index map for an element with the given id. If such
   *  an element exists return a pointer to it location,
   *  otherwise return NULL.
   *  @param id: The global id of the element we are looking for
   *  @return pointer to the element corresponding to id or NULL
   */
  const ElementClass* findElement(GlobalIndexType id) const;

  //! Find the element with the given id
  /*! Search the index map for an element with the given id. If such
   *  an element exists return its current local index in the array,
   *  otherwise return LNULL.
   *  @param id: The global id of the element we are looking for
   *  @return the local index corresponding to id or LNULL
   */
  LocalIndexType findElementIndex(GlobalIndexType id) const;

  //! Delete the element with the given global id from the array
  int deleteElement(GlobalIndexType id);
  
  /*************************************************************************************
   *******************     File Interface **********************************************
   ************************************************************************************/
  
  //! Write the node in binary format to the given stream
  virtual void toFile(FILE* output) const;

  //! Read the node in binary format from the given stream
  virtual void fromFile(FILE* input);

protected:
 
  //! Index of the first free space in the array
  LocalIndexType mHeadHole;
  
  //! Mapping from the global index to the local one
  IndexMapType mIndexMap;

  //! Increase the size of the array
  void expandArray();
};

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::MappedArrayBase(uint8_t block_bits)
  : BlockedArray<ElementClass,LocalIndexType>(block_bits), mHeadHole(LNULL)
{
  expandArray();
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::~MappedArrayBase()
{
  for (unsigned int i=0;i<this->mArray.size();i++) 
    //free(mArray[i]);
    delete[](this->mArray[i]);

  mIndexMap.clear();
}

//! Return a reference to the element of index i
template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
ElementClass& MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::at(GlobalIndexType i)
{
  typename IndexMapType::iterator mIt;

  mIt = mIndexMap.find(i);

  // This is equivalent to a memory access violation
  if (mIt == mIndexMap.end())
    assert(false);

  return this->get(mIt->second);
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
const ElementClass& MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::at(GlobalIndexType i) const
{
  typename IndexMapType::const_iterator mIt;

  mIt = mIndexMap.find(i);

  // This is equivalent to a memory access violation
  if (mIt == mIndexMap.end())
    assert(false);

  return this->get(mIt->second);
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
int MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::resize(GlobalIndexType size)
{
  // A mapped array re-sizes automatically and only if necessary not based on the
  // the index
  return 1;
}


template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
ElementClass* MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::findElement(GlobalIndexType id)
{
  typename IndexMapType::iterator mIt;

  mIt = mIndexMap.find(id);

  if (mIt == mIndexMap.end())
    return NULL;
  else
    return &this->get(mIt->second);
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
const ElementClass* MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::findElement(GlobalIndexType id) const
{
  typename IndexMapType::const_iterator mIt;

  mIt = mIndexMap.find(id);

  if (mIt == mIndexMap.end())
    return NULL;
  else
    return &this->get(mIt->second);
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
LocalIndexType MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::findElementIndex(GlobalIndexType id) const
{
  typename IndexMapType::const_iterator mIt;

  mIt = mIndexMap.find(id);

  if (mIt == mIndexMap.end())
    return MappedArrayBase::LNULL;
  else
    return mIt->second;
}


template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
void MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::expandArray()
{
  ElementClass* block;


  //block = (ElementClass*)malloc(sizeof(ElementClass)*sBlockSize);
  block = new ElementClass[this->mBlockSize];
  if (block == NULL) {
    fprintf(stderr,"Could not allocate enough memory for extendable array.");
    assert(false);
  }
  else {
    this->mArray.push_back(block);
    this->mCE += this->mBlockSize;
  }
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
void MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::toFile(FILE* output) const
{
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
void MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::fromFile(FILE* output)
{
}


template <class ElementClass, typename GlobalIndexType, typename LocalIndexType>
class MappedArray : public MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>
{
public:

  //! Typedef to satidfy the compiler
  typedef BlockedArray<ElementClass,LocalIndexType> BlockedType;

  //! Default constructor
  MappedArray(uint8_t block_bits=BlockedType::sBlockBits) :
    MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>(block_bits) {}

  //! Destructor
  virtual ~MappedArray() {}

  //! Add the given element to the array and return its local index
  ElementClass* insertElement(const ElementClass& element);

  //! Add the given element to the array if necessary grow the array
  virtual void insert(GlobalIndexType i, const ElementClass& element) {assert(i == element.id());insertElement(element);}

  //! Delete the given element from the array
  virtual int deleteElement(ElementClass* element) {return this->deleteElement(element->id());}

  //! Delete the element with the given global id from the array
  int deleteElement(GlobalIndexType id);
};


template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
ElementClass* MappedArray<ElementClass,GlobalIndexType,LocalIndexType>::insertElement(const ElementClass& element)
{
//  stmessage(this->mIndexMap.find(element.id())!=this->mIndexMap.end(),
  //        "Adding already existing element %d to the array.",element.id());

  if (this->mIndexMap.find(element.id())!=this->mIndexMap.end())
    return NULL;

  if (this->mHeadHole != LNULL) {
    LocalIndexType tmp = this->mHeadHole;

    this->mHeadHole = this->get(this->mHeadHole).hole();
    this->get(tmp) = element;
    this->mIndexMap[element.id()] = tmp;

    return &this->get(tmp);
  }
  else if (this->mNE < this->mCE) {
    this->get(this->mNE++) = element;
    this->mIndexMap[element.id()] = this->mNE - 1;
    return &this->get(this->mNE-1);
  }
  else {
    sterror(this->mNE != this->mCE,"Extendable array inconsistent.");

    this->expandArray();

    this->get(this->mNE++) = element;
    this->mIndexMap[element.id()] = this->mNE - 1;
    return &this->get(this->mNE-1);
  }
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
int MappedArray<ElementClass,GlobalIndexType,LocalIndexType>::deleteElement(GlobalIndexType id)
{
  typename MappedArrayBase<ElementClass,GlobalIndexType,LocalIndexType>::IndexMapType::iterator mIt;

  mIt = this->mIndexMap.find(id);

  if (mIt != this->mIndexMap.end()) {
    this->get(mIt->second) = ElementClass();
    this->get(mIt->second).hole(this->mHeadHole);
    this->mHeadHole = mIt->second;
    this->mIndexMap.erase(mIt);

    return 1;
  }

  return 0;
}


template<>
class MappedArray<GlobalIndexType,GlobalIndexType,LocalIndexType> : public MappedArrayBase<GlobalIndexType,GlobalIndexType,LocalIndexType>
{
public:

  //! Typedef to satidfy the compiler
  typedef BlockedArray<GlobalIndexType,LocalIndexType> BlockedType;

  //! Default constructor
  MappedArray(uint8_t block_bits=BlockedType::sBlockBits) :
    MappedArrayBase<GlobalIndexType,GlobalIndexType,LocalIndexType>(block_bits) {}

  //! Destructor
  virtual ~MappedArray() {}

  //! Add the given element to the array and return its local index
  GlobalIndexType* insertElement(const GlobalIndexType& element) {
    stmessage(this->mIndexMap.find(element)!=this->mIndexMap.end(),
            "Adding already existing element %d to the array.",element);

    if (this->mHeadHole != LNULL) {
      LocalIndexType tmp = this->mHeadHole;

      this->mHeadHole = this->get(mHeadHole);
      this->get(tmp) = element;
      this->mIndexMap[element] = tmp;

      return &get(tmp);
    }
    else if (this->mNE < this->mCE) {
      this->get(this->mNE++) = element;
      this->mIndexMap[element] = this->mNE - 1;
      return &get(this->mNE-1);
    }
    else {
      sterror(this->mNE != this->mCE,"Extendable array inconsistent.");

      this->expandArray();

      this->get(this->mNE++) = element;
      this->mIndexMap[element] = this->mNE - 1;
      return &get(this->mNE-1);
    }
  }

  virtual void insert(GlobalIndexType i, const GlobalIndexType& element) {*insertElement(i) = element;}



  //! Delete the given element from the array
  virtual int deleteElement(GlobalIndexType* element) {return deleteElement(*element);}

  //! Delete the element with the given global id from the array
  int deleteElement(GlobalIndexType id) {
    MappedArrayBase<GlobalIndexType,GlobalIndexType,LocalIndexType>::IndexMapType::iterator mIt;

    mIt = this->mIndexMap.find(id);

    if (mIt != this->mIndexMap.end()) {
      this->get(mIt->second) = this->mHeadHole;
      this->mHeadHole = mIt->second;
      this->mIndexMap.erase(mIt);

      return 1;
    }

    return 0;
  }
};


}


#endif
