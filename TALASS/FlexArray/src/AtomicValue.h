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

#ifndef FLEXARRAY_ATOMIC_VALUE_H
#define FLEXARRAY_ATOMIC_VALUE_H


#ifdef FLEXARRAY_ENABLE_OPA

#include "opa_primitives.h"

typedef OPA_int_t AtomicValue;

#else

typedef int AtomicValue;

#endif

namespace FlexArray {


//! Load an integer value from an AtomicValue
int load_atomic_value(AtomicValue* value);

//! Copy an integer value into an AtomicValue
void store_atomic_value(AtomicValue* value, int new_value);

//! Compare *value with old_value and if they are equal set value to new value
/*! Atomic operation to compare and swap an int sized value. The function
 *  compares value with old_value and if they are identical sets value to
 *  new_value. The function always returns the content of *value before the
 *  operation.
 *  @param value: pointer to the integer that should be modified
 *  @param old_value: the value we expect *value to have in order for a new assignment
 *  @param new_Value: the value we want to assign *value if it's in the state we expect it to be
 *  @return: the content of *value *before* the function was executed
 */
int compare_and_swap(AtomicValue* value, int old_value, int new_value);

}

#endif
