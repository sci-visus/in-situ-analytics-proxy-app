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

#ifndef FLEXARRAY_ATOMIC_LOCK_H
#define FLEXARRAY_ATOMIC_LOCK_H



#ifdef WIN32
#include <windows.h>
#define sleepmillisec(value) Sleep(value)
#else

#include <unistd.h> 

#ifdef SunOS 
#include <poll.h> 
#define sleepmillisec(value) \
{ \
  pollfd_t unused; \
  poll(&unused,0,value); \
} 
#else
#define sleepmillisec(value) \
{ \
  usleep(1000*value);\
} 
#endif

#endif

#include "AtomicValue.h"

namespace FlexArray {


class AtomicLock
{
public:
  
  //! The number of milliseconds to sleep when waiting for a lock
  static const int sLockWait = 10;

  //! Default constructor creating an unlocked lock
  AtomicLock() {store_atomic_value(&mLock,0);}

  //! Destructor
  ~AtomicLock() {}

  //! Type conversion to int primarily for test output
  operator int() {return load_atomic_value(&mLock);}

  //! Try to acquire the lock until you succeed
  void acquire() {while (compare_and_swap(&mLock,0,1)) sleepmillisec(sLockWait);}

  //! Release a lock. 
  /*! This function releases the lock by setting its value to 0. For performance
   *  reasons there are no checks performed on whether the lock was actually set.
   */
  void release() {store_atomic_value(&mLock,0);}
  

private:

  explicit AtomicLock(const AtomicLock& lock) {}

  AtomicValue mLock;
};

}


#endif
