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


#ifndef EXTENDABLEARRAY_H
#define EXTENDABLEARRAY_H

#include <map>

#include "Definitions.h"
#include "BlockedArray.h"

namespace FlexArray {

//! A dynamic array that keeps track of empty spaces
/*! As the name suggests the ExtendableArray implements an array of
 *  arbitrary elements that can grow but not shrink. The key feature
 *  is that memmory is allocated in blocks rather than using a dynamic
 *  vector or malloc/realloc calls. This is crucial to maintain
 *  pointers to elements consistently during the resizing process.  
 */
template <class ElementClass, typename GlobalIndexType, typename LocalIndexType>
class ExtendableArray : public BlockedArray<ElementClass,LocalIndexType>
{
public:
  
  //! Typedef to define the map from global to local index space
  typedef std::map<GlobalIndexType,LocalIndexType> IndexMapType;

  class iterator 
  {
  public:
    friend iterator ExtendableArray::begin();
    friend iterator ExtendableArray::end();

    iterator() : mSource(NULL) {}

    operator ElementClass*() {return &mSource->at(mIt->second);}

    iterator& operator++(int i) {mIt++;return *this;}

    iterator& operator=(const iterator& it) 
    {mIt=it.mIt;mSource=it.mSource;return *this;}
    
    bool operator==(const iterator& it) {return (mIt == it.mIt);}

    bool operator!=(const iterator& it) {return (mIt != it.mIt);}

    ElementClass* operator->() {return &mSource->at(mIt->second);}
    
    ElementClass& operator*() {return mSource->at(mIt->second);}
    
  private:
    
    typename IndexMapType::iterator mIt;

    ExtendableArray* mSource;
    
    iterator(const typename IndexMapType::iterator& it,
             ExtendableArray* a) : mIt(it), mSource(a) {}
  };
  

  //! Default constructor
  ExtendableArray(uint8_t block_bits=BlockedArray<ElementClass>::sBlockBits);

  //! Destructor
  ~ExtendableArray();

  //! Return an iterator to the first element
  iterator begin() {return iterator(mIndexMap.begin(),this);}

  //! Return an iterator pointing to after the last element
  iterator end() {return iterator(mIndexMap.end(),this);}  

  //! Return the number of active elements
  int elementCount() {return mIndexMap.size();}

  //! Find the element with the given id
  /*! Search the index map for an element with the given id. If such
   *  an element exists return its current local index in the array,
   *  otherwise return NULL.
   *  @param id: The global id of the element we are looking for
   *  @return pointer to the element corresponding to id or NULL
   */
  ElementClass* findElement(GlobalIndexType id);

  //! Find the element with the given id
  /*! Search the index map for an element with the given id. If such
   *  an element exists return its current local index in the array,
   *  otherwise return LNULL.
   *  @param id: The global id of the element we are looking for
   *  @return the local index corresponding to id or LNULL
   */
  LocalIndexType findElementIndex(GlobalIndexType id) const;

  //! Add the given element to the array and return its local index
  LocalIndexType insertElement(const ElementClass& element);
  
  //! Delete the given element from the array
  virtual int deleteElement(ElementClass* element) {return deleteElement(element->id());}

  //! Delete the element with the given global id from the array
  int deleteElement(GlobalIndexType id);
  
  /*************************************************************************************
   *******************     File Interface **********************************************
   ************************************************************************************/
  
  //! Write the node in binary format to the given stream
  virtual void toFile(FILE* output) const;

  //! Read the node in binary format from the given stream
  virtual void fromFile(FILE* input);

private:
 
  //! Index of the first free space in the array
  LocalIndexType mHeadHole;
  
  //! Mapping from the global index to the local one
  IndexMapType mIndexMap;

  //! Increase the size of the array
  void expandArray();
};


template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::ExtendableArray(uint8_t block_bits) 
  : BlockedArray<ElementClass,LocalIndexType>(block_bits), mHeadHole(LNULL)
{
  expandArray();
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::~ExtendableArray()
{
  for (unsigned int i=0;i<this->mArray.size();i++) 
    //free(mArray[i]);
    delete[](this->mArray[i]);
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
ElementClass* ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::findElement(GlobalIndexType id)
{
  typename IndexMapType::iterator mIt;

  mIt = mIndexMap.find(id);

  if (mIt == mIndexMap.end())
    return NULL;
  else
    return &this->at(mIt->second);
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
LocalIndexType ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::findElementIndex(GlobalIndexType id) const
{
  typename IndexMapType::const_iterator mIt;

  mIt = mIndexMap.find(id);

  if (mIt == mIndexMap.end())
    return LNULL;
  else
    return mIt->second;
}


template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
LocalIndexType ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::insertElement(const ElementClass& element)
{
  message(mIndexMap.find(element.id())!=mIndexMap.end(),
          "Adding already existing element %d to the array.",element.id());
 
  if (mHeadHole != LNULL) {
    LocalIndexType tmp = mHeadHole;

    mHeadHole = this->at(mHeadHole).hole();
    this->at(tmp) = element;
    mIndexMap[element.id()] = tmp;

    return tmp;
  }
  else if (this->mNE < this->mCE) {
    this->at(this->mNE++) = element;
    mIndexMap[element.id()] = this->mNE - 1;
    return this->mNE-1;
  }
  else if (mHeadHole != LNULL) {
    LocalIndexType tmp = mHeadHole;

    mHeadHole = this->at(mHeadHole).hole();
    this->at(tmp) = element;
    mIndexMap[element.id()] = tmp;

    return tmp;
  }
  else {
    sterror(this->mNE != this->mCE,"Extendable array inconsistent.");
    
    expandArray();
  
    this->at(this->mNE++) = element;
    mIndexMap[element.id()] = this->mNE - 1;
    return this->mNE-1;
  }
}    
  
template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
int ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::deleteElement(GlobalIndexType id)
{
  typename IndexMapType::iterator mIt;

  mIt = mIndexMap.find(id);

  if (mIt != mIndexMap.end()) {
    this->at(mIt->second) = ElementClass();
    this->at(mIt->second).hole(mHeadHole);
    mHeadHole = mIt->second;
    mIndexMap.erase(mIt);
    
    return 1;
  }

  return 0;
}


template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
void ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::expandArray()
{
  ElementClass* block;


  //block = (ElementClass*)malloc(sizeof(ElementClass)*sBlockSize);
  block = new ElementClass[this->mBlockSize];
  if (block == NULL) {
    stwarning("Could not allocate enough memory for extendable array.");
  }
  else {
    this->mArray.push_back(block);
    this->mCE += this->mBlockSize;
  }
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
void ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::toFile(FILE* output) const
{
}

template<class ElementClass,typename GlobalIndexType,typename LocalIndexType>
void ExtendableArray<ElementClass,GlobalIndexType,LocalIndexType>::fromFile(FILE* output)
{
}

}


#endif
