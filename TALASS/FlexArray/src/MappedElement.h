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


#ifndef FA_MAPPEDELEMENT_H
#define FA_MAPPEDELEMENT_H

namespace FlexArray {

//! Element of a MappedArray
/*! All elements of a MappedArray must implement this interface which
 *  provides the basic functionality to map between local and global
 *  indices and allows the skip list to be stored in place of the
 *  global index.
 */
template <typename GlobalIndexType, typename LocalIndexType>
class MappedElement {
public:

  //! The index representing an invalid index
  static const GlobalIndexType sGNull = (GlobalIndexType)(-1);

  //! Default constructor
  MappedElement(GlobalIndexType id=sGNull) : mId(id) {}

  //! Destructor
  ~MappedElement() {}

  //! Assignment operator
  MappedElement& operator=(const MappedElement& e) {mId = e.mId; return *this;}

  //! Return the global id
  GlobalIndexType id() const {return mId;}

  //! Return the current hole index
  LocalIndexType hole() const {return static_cast<LocalIndexType>(mId);}

  //! Set the hole index
  void hole(LocalIndexType h) {mId = h;}

protected:

  //! The global id which doubles as hole index
  GlobalIndexType mId;
};

template <typename GlobalIndexType, typename LocalIndexType>
class MappedFunctionElement : public MappedElement<GlobalIndexType,LocalIndexType>
{
public:

  //! The index representing an invalid index
  static const GlobalIndexType sGNull = (GlobalIndexType)(-1);

  //! Default constructor
  MappedFunctionElement(FunctionType f=0, GlobalIndexType id=sGNull) : MappedElement<GlobalIndexType,LocalIndexType>(id), mFunc(f) {}

  //! Destructor
  ~MappedFunctionElement() {}

  operator FunctionType() const {return mFunc;}

  MappedFunctionElement& operator=(const MappedFunctionElement& elem) {this->mId = elem.mId;mFunc=elem.mFunc;return *this;}

  bool operator<(const MappedFunctionElement& elem) const {return (this->mFunc < elem.mFunc);}

  bool operator>(const MappedFunctionElement& elem) const {return (this->mFunc > elem.mFunc);}

  bool operator==(const MappedFunctionElement& elem) const {return (this->mFunc == elem.mFunc);}
private:

  //! The function value
  FunctionType mFunc;
};

}

#endif
