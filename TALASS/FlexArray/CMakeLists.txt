#######################################################################
#
# Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
# Produced at the Lawrence Livermore National Laboratory  
# Written by bremer5@llnl.gov 
# OCEC-08-107
# All rights reserved.  
#   
# This file is part of "Streaming Topological Graphs Version 1.0."
# Please also read BSD_ADDITIONAL.txt.
#   
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#   
# @ Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the disclaimer below.
# @ Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the disclaimer (as noted below) in
#   the documentation and/or other materials provided with the
#   distribution.
# @ Neither the name of the LLNS/LLNL nor the names of its contributors
#   may be used to endorse or promote products derived from this software
#   without specific prior written permission.
#   
#  
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL LAWRENCE
# LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING
#
#######################################################################

cmake_minimum_required(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../CMake-Modules ${CMAKE_MODULE_PATH})
INCLUDE(TalassSetup)

TALASS_PROJECT(FlexArray)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/config/FlexArrayConfig.cmake)

IF (TALASS_ENABLE_IDX)
    FIND_PACKAGE(ViSUSIO)
ENDIF()
    


IF (TALASS_ENABLE_OPA)

   FIND_PATH(OPA_INCLUDE_DIR opa_primitives.h
      /usr/include
      /usr/X11/include
      /usr/X11R6/include
    )           

   FIND_LIBRARY(OPA_LIBRARIES opa
       /usr/lib
       /sw/lib
   )

   IF (OPA_INCLUDE_DIR AND OPA_LIBRARIES)

      IF (CMAKE_VERBOSE_MAKEFILE)
         MESSAGE("Using OPA_INCLUDE_DIR = " ${OPA_INCLUDE_DIR}) 
         MESSAGE("Using OPA_LIBRARIES   = " ${OPA_LIBRARIES}) 
      ENDIF (CMAKE_VERBOSE_MAKEFILE)

   ENDIF (OPA_INCLUDE_DIR AND OPA_LIBRARIES)

   ADD_DEFINITIONS("-DFLEXARRAY_ENABLE_OPA=1")
 
ENDIF ()


ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)

