
cmake_minimum_required(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../CMake-Modules ${CMAKE_MODULE_PATH})
INCLUDE(TalassSetup)
SET(ENABLE_PARALLEL ON CACHE BOOL "Enable the parallel build")
TALASS_PROJECT(ParallelTopology)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/config/PTConfig.cmake)

FIND_PACKAGE(FlexArray)
FIND_PACKAGE(MPI)
FIND_PACKAGE(DIY)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(utilities)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()
