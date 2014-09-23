#######################################################################
#
# Copyright (c) 2008, Lawrence Livermore National Security, LLC.  
# Produced at the Lawrence Livermore National Laboratory  
# Written by bremer5@llnl.gov, jcbenne@sandia.gov
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
# Try to find the Statistics libraries
# Once done this will define
#
# HDF5_FOUND          - system has HDF5
# HDF5_INCLUDE_DIR    - path to h5cpp.h
# HDF5_LIBRARIES      - path to libhdf5.a
#
#


FIND_PATH(HDF5_INCLUDE_DIR H5Cpp.h)

FIND_LIBRARY(HDF5_LIBRARY hdf5)
FIND_LIBRARY(HDF5_CPP_LIBRARY hdf5_cpp)

IF (HDF5_LIBRARY AND HDF5_CPP_LIBRARY)
  SET(HDF5_LIBRARIES ${HDF5_LIBRARY} ${HDF5_CPP_LIBRARY})
ENDIF (HDF5_LIBRARY AND HDF5_CPP_LIBRARY)

IF (HDF5_INCLUDE_DIR AND HDF5_LIBRARIES)

   SET(HDF5_FOUND "YES")
   IF (CMAKE_VERBOSE_MAKEFILE)
      MESSAGE("Using HDF5_INCLUDE_DIR = " ${HDF5_INCLUDE_DIR}) 
      MESSAGE("Using HDF5_LIBRARIES   = " ${HDF5_LIBRARIES}) 
   ENDIF (CMAKE_VERBOSE_MAKEFILE)

ELSE (HDF5_INCLUDE_DIR AND HDF5_LIBRARIES)
   
   MESSAGE("ERROR HDF5 library not found on the system")
 
ENDIF (HDF5_INCLUDE_DIR AND HDF5_LIBRARIES)
                         
