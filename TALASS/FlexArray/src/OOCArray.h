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


#ifndef FA_OOCARRAY_H
#define FA_OOCARRAY_H

#include <vector>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <cerrno>

#include <cstring>

#if  _WIN32 || _WIN64

#else

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#endif

#include "BlockedArray.h"

namespace FlexArray {

//! Dynamic out-of-core array using memory-mapped files
/*! This class implements an extendable out-of-core array which is
 *  memory mapped into an arbitrary number of files depending on a
 *  given block size.
 */
template <class ElementClass, typename IndexType>
class OOCArray : public BlockedArray<ElementClass, IndexType>
{
public:

  typedef  BlockedArray<ElementClass, IndexType> BaseClass;
  //! Default directory name
  /*! To store all the out of core blocks an OOCArray will create a
   *  local directory called sDirNameTemplate in which it will create
   *  unique filenames for each block. Note, that many OOCArrays in
   *  many threads might use this mechanism.
   */
  static const char* sDirNameTemplate;

  //! Default filename template
  static const char* sFileNameTemplate;

  //! String used to make each block unique
  static const char* sBlockTemplate;

  //! Default constructor
  explicit OOCArray(uint8_t block_bits=BaseClass::sBlockBits,IndexType size=0);

  //! Destructor
  ~OOCArray();

  //! Resize the array
  /*! Grow or shrink the array in order to fit at least size many
   *  elements and at most (size + mBlockSize-1) elements. Memory will
   *  be allocated in blocks and all blocks not effected by the resize
   *  will maintain their pointers
   *  @param size: The new size of the array
   *  @return: 1 if successful; 0 otherwise
   */
  int resize(IndexType size); 

private:

  //! Filename template for the blocks
  char mNameTemplate[200];
  
  //! The array of file pointers for each block
  std::vector<int> mFiles;

  //! Private copy constructor
  /*! The copy constructor is private to alert the user that copying OOCArrays
   *  for now is not possible. Note, that the destructor closes and erases the
   *  necesssary files. Thus until we implement some reference counting scheme a
   *  copy constructor (and in fact any kind of assignment is out of the
   *  question.
   */
  OOCArray(const OOCArray& array);
  
};

template <class ElementClass, typename IndexType>
const char* OOCArray<ElementClass,IndexType>::sDirNameTemplate = "tmp_ooc";

template <class ElementClass, typename IndexType>
const char* OOCArray<ElementClass,IndexType>::sFileNameTemplate = "arrayXXXXXX";

template <class ElementClass, typename IndexType>
const char* OOCArray<ElementClass,IndexType>::sBlockTemplate = "_block_%04d";


template <class ElementClass, typename IndexType>
OOCArray<ElementClass,IndexType>::OOCArray(uint8_t block_bits,IndexType size) :
  BlockedArray<ElementClass,IndexType>(block_bits)
{
  char path_template[500] = "";
  int guard;
  int tmp;

  // First, we see whether a global SCRATCH variable s defined for us to put our
  // files into
  if (getenv("SCRATCH") != NULL) 
    strcpy(path_template,getenv("SCRATCH"));
  else
    strcpy(path_template,".");

    // Add the system specific directory seperator
#if _WIN32 || _WIN64
    strcat(path_template,"\\");
#else  
    strcat(path_template,"/");
#endif

  // Second we assemble the directory template name using either SCRATCH or the
  // local directory as root path
  strcat(path_template,sDirNameTemplate);

  //fprintf(stderr,"Will try to create the guard directory \"%s\"\n\n",path_template);

  // Third, we try to create our directory
  tmp = mkdir(path_template,S_IRWXU);
 
  sterror((tmp == -1) && (errno != EEXIST),"Could not create directory \"%s\" for OOCArray.",sDirNameTemplate);


#if _WIN32 || _WIN64
  strcat(path_template,"\\");
#else  
  strcat(path_template,"/");
#endif

  // Finally, we assemble the file name template
  strcat(path_template,sFileNameTemplate);

  //fprintf(stderr,"Creating guard file from template \"%s\"\n",path_template);

  // Now let the operating system create a unique file name for
  // us. Furthermore, the mkstemp function automatically creates the
  // file and returns an open file descriptor avoiding the race
  // conditions between creating a unique name and opening the
  // file. We will immediately close the file again but it will serve
  // as guard in the future to ensure that no other array uses the
  // same name again
  guard = mkstemp(path_template);

  sterror((guard == -1),"Could not create temporary file name for OOCArray.");

  close(guard);

  // Finally, we attach block numbers to the end of the template and
  // use the resulting file names as our name template
  strcpy(mNameTemplate,path_template);
  strcat(mNameTemplate,sBlockTemplate); // The len

  
  // Resize the array to the given default size
  resize(size);
}

template <class ElementClass, typename IndexType>
OOCArray<ElementClass,IndexType>::OOCArray(const OOCArray<ElementClass,IndexType>& array) :
  BlockedArray<ElementClass,IndexType>(array), mFiles(array.mFiles)
{
  strcpy(mNameTemplate,array.mNameTemplate);
}


template <class ElementClass, typename IndexType>
OOCArray<ElementClass,IndexType>::~OOCArray()
{
  std::vector<int>::iterator fIt;
  typename std::vector<ElementClass*>::iterator bIt;
 
#if _WIN32 || _WIN64
#else
  // Unmap all remaining blocks
  for (bIt=this->mArray.begin();bIt!=this->mArray.end();bIt++)
    munmap(*bIt,this->mBlockSize*sizeof(ElementClass));
#endif

  // Close all file pointers and remove the files
  char filename[300];
  for (fIt=mFiles.begin();fIt!=mFiles.end();fIt++) {
    close(*fIt);
    sprintf(filename,mNameTemplate,fIt-mFiles.begin());
    remove(filename);
  }

  // Remove the guard file
  strcpy(filename+strlen(mNameTemplate)-strlen(sBlockTemplate),"\0");
  remove(filename);

}

template <class ElementClass, typename IndexType>
int OOCArray<ElementClass,IndexType>::resize(IndexType size)
{

#if _WIN32 || _WIN64
  return BlockArray<ElementClass,IndexType>::resize(size);
#else

  // First we check whether the array needs to shrink. While we can
  // remove a block. Note that the first half of the if-condition is
  // necessary to handle unsigned IndexTypes
  while ((this->mCE >= this->mBlockSize) && (size < (this->mCE - this->mBlockSize))) {
    char filename[300];
    
    // Unmap the block
    munmap(this->mArray.back(),this->mBlockSize*sizeof(ElementClass));

    // Close and remove the file 
    close(mFiles.back());
    sprintf(filename,mNameTemplate,mFiles.size()-1);
    remove(filename);
    
    mFiles.pop_back();
    this->mArray.pop_back();

    this->mCE -= this->mBlockSize;
  }

  
  // Second, check whether the array must grow

  char filename[300];
  int tmp;
  ElementClass* block;

  // While we need to allocate more blocks
  while (size > this->mCE) {
  
    // Create the new files
    sprintf(filename,mNameTemplate,this->mCE >> this->mBlockBits);
    tmp = open(filename,O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

    // Make sure each file is large enough to contain the block by seeking to the end
    lseek(tmp,this->mBlockSize*sizeof(ElementClass)-1,SEEK_SET);

    // and writing one copy of the empty string
    write(tmp,"",1);

    // Store the file pointer
    mFiles.push_back(tmp);
    
    // Map the file to a new block
    block = (ElementClass*)mmap(NULL,this->mBlockSize*sizeof(ElementClass),
                                PROT_READ |  PROT_WRITE, MAP_SHARED, mFiles.back(), 0);
    
    sterror(block==MAP_FAILED,"Cannot map additional block got error [%s]\n",strerror(errno));
  
    // Store the new block
    this->mArray.push_back(block);
    this->mCE += this->mBlockSize;
  }
    
  this->mNE = size;

  return 1;
#endif
}

} // namespace FlexArray

#endif
