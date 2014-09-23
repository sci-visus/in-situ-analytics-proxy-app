

SET (PT_HEADERS
  FiFoBuffer.h
  TopoInputStream.h
  SerialInputStream.h
  DataSpaceInputStream.h
  TopoOutputStream.h
  SerialOutputStream.h
  DataSpaceOutputStream.h
  TopoCommunicator.h
  SerialCommunicator.h
  Token.h
  TokenInputBuffer.h
  TokenOutputBuffer.h
  Algorithm.h
  PatchAlgorithm.h
  LocalComputeAlgorithm.h
  ScatterAlgorithm.h
  GatherAlgorithm.h
  ControlFlow.h
  MergeTreeLite.h
  ModuloFlow.h
  ParallelFlow.h
  ModuloScatter.h
  ModuloGather.h
  Neighborhood.h
  BoxSorter.h
  TopoControl.h
  SerialControl.h
  PointIndex.h
  DistributedDefinitions.h
  MergeTree.h
  StreamingGatherAlgorithm.h
  ParallelGatherAlgorithm.h
  Box.h
  MergeTreeBuilder.h
  UnionFind.h
  SpatialFlow.h
  InputStream.h
  SortedInputStream.h
)

IF (TALASS_ENABLE_PARALLEL)
  SET (PT_HEADERS ${PT_HEADERS}
      ParallelCommunicator.h
      ParallelOutputStream.h
      ParallelInputStream.h
      ParallelControl.h
      FixLocalTreeAlgorithm.h
      SortedFixLocalTreeAlgorithm.h
      ParallelSortedInputStream.h
  )
ENDIF()
  


SET(PT_SOURCES
  FiFoBuffer.cpp
  TopoInputStream.cpp
  SerialInputStream.cpp
  TopoOutputStream.cpp
  SerialOutputStream.cpp
  DataSpaceInputStream.cpp
  SerialCommunicator.cpp
  DataSpaceOutputStream.cpp
  Token.cpp
  TokenInputBuffer.cpp
  TokenOutputBuffer.cpp
  Algorithm.cpp
  PatchAlgorithm.cpp
  LocalComputeAlgorithm.cpp
  ScatterAlgorithm.cpp
  GatherAlgorithm.cpp
  ModuloFlow.cpp
  SpatialFlow.cpp
  ModuloScatter.cpp
  ModuloGather.cpp
  SerialControl.cpp
  PointIndex.cpp
  MergeTree.cpp
  StreamingGatherAlgorithm.cpp
  ParallelGatherAlgorithm.cpp
  SortedInputStream.cpp
 )
 
IF (TALASS_ENABLE_PARALLEL)
  SET (PT_SOURCES ${PT_SOURCES}
      ParallelCommunicator.cpp
      ParallelInputStream.cpp
      ParallelOutputStream.cpp
      ParallelControl.cpp
      ParallelFlow.cpp
      FixLocalTreeAlgorithm.cpp
      SortedFixLocalTreeAlgorithm.cpp
      ParallelSortedInputStream.cpp
  )
ENDIF()

 
 
INCLUDE_DIRECTORIES (
   ${CMAKE_SOURCE_DIR}/profiling
   ${FLEXARRAY_INCLUDE_DIR}
)

IF (TALASS_ENABLE_PARALLEL)
    INCLUDE_DIRECTORIES (
        ${MPI_C_INCLUDE_PATH}
        ${MPI_CXX_INCLUDE_PATH}
    )
ENDIF()


IF (ENABLE_STREAMING_TOPOLOGY)
   INCLUDE_DIRECTORIES(${TOPO_PARSER_INCLUDE_DIR})
ENDIF (ENABLE_STREAMING_TOPOLOGY)

 
ADD_LIBRARY(parallelMT STATIC ${PT_HEADERS} ${PT_SOURCES})


INSTALL(TARGETS  parallelMT 
        ARCHIVE DESTINATION ${PROJECT_INSTALL_PREFIX}/lib
        LIBRARY DESTINATION ${PROJECT_INSTALL_PREFIX}/lib
)

