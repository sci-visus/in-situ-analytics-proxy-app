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


#ifndef FA_ARRAY_H
#define FA_ARRAY_H

#include <vector>
#include "TalassConfig.h"

namespace FlexArray {

//! Basic typedef defining the internal index used 

//! Array interface
/*! An array defines the interface for all FlexArray array
 *  classes. The interface is roughly modeled after a std::vector yet
 *  only a small subset of the operations are supported. Furthermore,
 *  there is *no* expectation that an array behaves like a vector or
 *  stores its elements like a vector. The speficic behavior is
 *  defined in the derived classes and can differ substantially
 */
template <class ElementClass, typename IndexTypeTemplate>
class Array
{
public:

  typedef IndexTypeTemplate IndexType;

  //! Default constructor
  Array() {}
  
  //! Copy constructor
  Array(const Array& a) {}

  //! Destructor
  virtual ~Array() {}

   //! Return a reference to the element of index i 
  virtual ElementClass& at(IndexType i) = 0;

  //! Return a const reference to the element of index i 
  virtual const ElementClass& at(IndexType i) const = 0;

  //! Return a reference to the element of index i
  virtual ElementClass& operator[](IndexType i) {return at(i);}

  //! Return a reference to the element of index i
  virtual const ElementClass& operator[](IndexType i) const {return at(i);}

  //! Return a reference to the last element
  virtual ElementClass& back() = 0;
  
  //! Return a const reference to the last element
  virtual const ElementClass& back() const = 0; 

  //! Return the current size
  virtual IndexType size() const = 0;

  //! Resize the array
  virtual int resize(IndexType size) = 0;

  //! Add an element to the array and return its index
  virtual IndexType push_back(const ElementClass& element) = 0;

  //! Dump the content of the array to disk in binary format
  virtual int dumpBinary(FILE* output) const { sterror(true,"dumBinary function not implemented."); return 0;}

};

  

} // namespace FlexArray

#endif
