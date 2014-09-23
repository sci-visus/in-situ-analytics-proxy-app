INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src  
    ${FLEXARRAY_INCLUDE_DIR}
    ${MPI_CXX_INCLUDE_PATH}
)

SET(LINK_LIBRARIES parallelMT )

ADD_EXECUTABLE(generateData  generateData.cpp)   

ADD_EXECUTABLE(createDataBlock createDataBlock.cpp)

ADD_EXECUTABLE(localCompute localCompute.cpp)   
TARGET_LINK_LIBRARIES(localCompute ${LINK_LIBRARIES} ${MPI_CXX_LIBRARIES})

ADD_EXECUTABLE(joinTrees joinTrees.cpp)   
TARGET_LINK_LIBRARIES(joinTrees ${LINK_LIBRARIES} ${MPI_CXX_LIBRARIES})

#IF (TALASS_ENABLE_PARALLEL)
#    INCLUDE_DIRECTORIES(
#        ${DIY_INCLUDE_DIR}
#    )
#
#    ADD_EXECUTABLE(parallelTest parallelTest.cpp)   
#    TARGET_LINK_LIBRARIES(parallelTest ${LINK_LIBRARIES} ${MPI_CXX_LIBRARIES} ${DIY_LIBRARIES})
#ENDIF()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "adir='${dir}'")
endforeach()
