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





#
# Try to find the FlexArray libraries
# Once done this will define
#
# FLEXARRAY_FOUND         - system has FlexArray
# FLEXARRY_INCLUDE_DIR    - path to FlexArray/Array.h
#
#

IF (NOT DEFINED FLEXARRAY_FOUND)
   SET (FLEXARRAY_FOUND FALSE)
ENDIF ()


SET(FLEXARRAY_ENABLE_OPA "NO")

IF ((NOT DEFINED TALASS_SUPER_BUILD) AND (NOT FLEXARRAY_FOUND))

   FIND_PATH(FLEXARRAY_INCLUDE_DIR FlexArray/Array.h
      ${CMAKE_SOURCE_DIR}/../FlexArray/build/include
      ${PROJECT_INSTALL_PREFIX}/include
      /usr/include
      /usr/X11/include
      /usr/X11R6/include
   )

   FIND_LIBRARY(FLEXARRAY_LIBRARIES FlexArray
      ${CMAKE_SOURCE_DIR}/../FlexArray/build/lib
      ${PROJECT_INSTALL_PREFIX}/lib
       /usr/lib
       /sw/lib
   )



   IF (FLEXARRAY_INCLUDE_DIR AND FLEXARRAY_LIBRARIES)

      SET(FLEXARRAY_FOUND TRUE)
      IF (CMAKE_VERBOSE_MAKEFILE)
         MESSAGE("Using FLEXARRAY_INCLUDE_DIR = " ${FLEXARRAY_INCLUDE_DIR}) 
         MESSAGE("Using FLEXARRAY_LIBRARIES   = " ${FLEXARRAY_LIBRARIES}) 
      ENDIF (CMAKE_VERBOSE_MAKEFILE)

   ELSE ()
   
      MESSAGE("ERROR FlexArray library not found on the system")
 
   ENDIF ()
                         
   IF (FLEXARRAY_ENABLE_OPA)

      FIND_PATH(OPA_INCLUDE_DIR opa_primitives.h
         ${ADDITIONAL_INCLUDE_PATH}
         /usr/include
         /usr/X11/include
         /usr/X11R6/include
         ${STREAMING_TOPOLOGY_INCLUDE}  
      )

      FIND_LIBRARY(OPA_LIBRARIES opa
         ${ADDITIONAL_LIBRARY_PATH}
         ${STREAMING_TOPOLOGY_LIBRARIES}
         /usr/lib
         /sw/lib
      )

      IF (OPA_INCLUDE_DIR AND OPA_LIBRARIES)

         IF (CMAKE_VERBOSE_MAKEFILE)
            MESSAGE("Using OPA_INCLUDE_DIR = " ${OPA_INCLUDE_DIR}) 
            MESSAGE("Using OPA_LIBRARIES   = " ${OPA_LIBRARIES}) 
         ENDIF (CMAKE_VERBOSE_MAKEFILE)

         SET(FLEXARRAY_INCLUDE_DIR
           ${FLEXARRAY_INCLUDE_DIR}
           ${OPA_INCLUDE_DIR}
         )

         SET(FLEXARRAY_LIBRARIES
            ${FLEXARRAY_LIBRARIES}
            ${OPA_LIBRARIES}
         )

         SET(ADDITIONAL_COMPILE_FLAGS ${ADDITIONAL_COMPILE_FLAGS} "-DFLEXARRAY_ENABLE_OPA")

      ELSE (OPA_INCLUDE_DIR AND OPA_LIBRARIES)

         MESSAGE("Required OpenPA libraries not found.")
         SET(FLEXARRAY_FOUND FALSE)

      ENDIF (OPA_INCLUDE_DIR AND OPA_LIBRARIES)
 
   ENDIF (FLEXARRAY_ENABLE_OPA)

ELSEIF (NOT FLEXARRAY_FOUND)

   FIND_PATH(FLEXARRAY_SOURCE_DIR FlexArray/CMakeLists.txt
      ${CMAKE_SOURCE_DIR}/../
   )

   ADD_SUBDIRECTORY(${FLEXARRAY_SOURCE_DIR}/FlexArray ${PROJECT_BINARY_DIR}/FlexArray)

   SET(FLEXARRAY_FOUND TRUE)
   SET(FLEXARRAY_INCLUDE_DIR ${FLEXARRAY_SOURCE_DIR}/FlexArray/src)
   SET(FLEXARRAY_LIBRARIES FlexArray)
   
    GET_PROPERTY(LOCAL_DEFINITIONS DIRECTORY ${FLEXARRAY_SOURCE_DIR}/FlexArray PROPERTY COMPILE_DEFINITIONS)
    SET_PROPERTY(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${LOCAL_DEFINITIONS})
   

ENDIF()
