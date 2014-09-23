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


#ifndef IDXARRAY_H
#define IDXARRAY_H

#include "visus.h"
#include "db/dataset/visus_dataset.h"
#include "db/access/visus_diskaccess.h"
#include "db/query/visus_boxquery.h"
#include "Array.h"

namespace FlexArray {

template <class ElementClass>
class IDXArray : public Array<ElementClass>
{
public:

  //! The default dimension of the buffer
  static const uint16_t sDefaultBoxSize[3];

  //! Default constructor
  IDXArray();

  //! Constructor to connect the array to an idx-file
  IDXArray(const char* idx_file, uint8_t field=0);

  //! Destructor
  ~IDXArray() {}

  //! Return a reference to the element of index i
 virtual ElementClass& at(IndexType i) {return mData[localIndex(i)];}

 //! Return a const reference to the element of index i
 virtual const ElementClass& at(IndexType i) const {return mData[localIndex(i)];}

 //! Return a reference to the last element
 virtual ElementClass& back() {error(true,"An IDXArray is static and does not implement \"back()\"");}

 //! Return a const reference to the last element
 virtual const ElementClass& back() const {error(true,"An IDXArray is static and does not implement \"back()\"");}

 //! Return the current size
 virtual IndexType size() const;

 //! Resize the array
 virtual int resize(IndexType size) {error(true,"An IDXArray is static and cannot be resized");return 0;}

 //! Add an element to the array and return its index
 virtual IndexType push_back(const ElementClass& element) {error(true,"An IDXArray is static and cannot be extended");return 0;}

private:

 //! A pointer to the IDX file in question
 Visus::DatasetPtr mIdx;

 //! A pointer to a data access
 Visus::AccessPtr mAccess;

 //! The pointer to the last query
 mutable Visus::BoxQueryPtr mQuery;

 //! The world box stored separate for convenience
 Visus::NdBox mWorldBox;

 //! The dimension and location of the samples
 Visus::NdBox mQueryBox;

 //! The index of the field
 uint8_t mField;

 //! The internal block of memory we are using
 ElementClass* mData;

 //! The size of the current box
 uint16_t mBoxSize[3];

 //! The position of the low corner of the current box
 uint16_t mLow[3];

 //! Helper function for the constructor;
 void init();

 //! Compute the local index of the global index i and load the corresponding data if necessary
 IndexType localIndex(IndexType i) const;
};

template <class ElementClass>
const uint16_t IDXArray<ElementClass>::sDefaultBoxSize[3] = {101,101,101};



template <class ElementClass>
IDXArray<ElementClass>::IDXArray() : mIdx(new Visus::Dataset()), mField(0), mData(NULL)
{
  init();
}

template <class ElementClass>
IDXArray<ElementClass>::IDXArray(const char* idx_file, uint8_t field) : mIdx(new Visus::Dataset()), mField(field), mData(NULL)
{
  // Now try to open the idx file
  if (!mIdx->setUrl(idx_file))
    error(true,"Could not open idx file \"%s\"",idx_file);

  // Make sure that the file has enough fields
  error(mField < mIdx->getNumberOfFields(),"This idx file does not contains %d fields",mField+1);

  // Make sure that the data as the correct size
  error(Visus::DType::getByteSize(mIdx->getField(mField)->dtype, mIdx->getField(mField)->dtype,1) != sizeof(ElementClass),
        "Data sizes do not match");

  // Since the file looks alright we'll create the access to get data
  mAccess = Visus::DiskAccess::createDiskAccess(mIdx);

  // Store the world box
  mWorldBox = mIdx->getUserBox(mIdx->getMaxH());

  // Now query the left lower corner of the data set to start things off
  Visus::NdBox query_box;

  // Lower corner
  query_box.p1[0] = query_box.p1[1] = query_box.p1[2] = 0;

  // Upper corner
  query_box.p2[0] = mBoxSize[0];
  query_box.p2[1] = mBoxSize[1];
  query_box.p2[2] = mBoxSize[2];

  // Create the actual query
  mQuery = Visus::BoxQueryPtr(new Visus::BoxQuery(mIdx,query_box));
  mQuery->setField(mField);
  mQuery->allocateBuffers();
  mQuery->setAccess(mAccess);

  // Execute the query
  mQuery->read();

  mData = (ElementClass*)mQuery->buffer->c_ptr();
}

template <class ElementClass>
void IDXArray<ElementClass>::init()
{
  mBoxSize[0] = sDefaultBoxSize[0];
  mBoxSize[1] = sDefaultBoxSize[1];
  mBoxSize[2] = sDefaultBoxSize[2];

  mLow[0] = mLow[1] = mLow[2] = 0;
}

template <class ElementClass>
IndexType IDXArray<ElementClass>::localIndex(IndexType i) const
{
  // Figure out whether this sample is already loaded
  return 0;
}


}


#endif /* IDXARRAY_H_ */
