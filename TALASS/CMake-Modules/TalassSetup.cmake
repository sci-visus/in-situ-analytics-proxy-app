# This is the main Talass configuration module to be called fom the
# head project

MACRO(TALASS_PROJECT project_name)

   # If the super build variable is already assigned there is no need to
   # re-execute any of the setup code
   IF (NOT DEFINED TALASS_SUPER_BUILD)

      # If no super build has been defined than this is the top most 
      # project and thus we call PROJECT
      PROJECT(${project_name})

      # If the user has made an explicit choice 
      IF (DEFINED USE_SUPER_BUILD)
         SET(TALASS_SUPER_BUILD ${USE_SUPER_BUILD}) 
      ELSE() # Otherwise, use the default which is to use a super build
         SET(TALASS_SUPER_BUILD TRUE)
      ENDIF()
      
      # Now we define internal variables controlling the installation
      IF (NOT DEFINED PROJECT_INSTALL_PREFIX) 
         SET(PROJECT_INSTALL_PREFIX ${PROJECT_BINARY_DIR} CACHE PATH "Project wide install prefix")           
      ENDIF()

      SET(PROJECT_INCLUDE_DIR ${PROJECT_INSTALL_PREFIX}/include/talass CACHE PATH "Where to install the include files")
      SET(PROJECT_EXE_DIR ${PROJECT_INSTALL_PREFIX}/bin CACHE PATH "Where to install the binaries")
      SET(PROJECT_LIBRARY_DIR ${PROJECT_INSTALL_PREFIX}/lib CACHE PATH "Where to install the libraries")


      # Now we set all the global typedefs
      IF (NOT DEFINED FUNCTION_TYPE)
         SET(FUNCTION_TYPE "float" CACHE STRING "The data type used for data")
      ELSE()
         SET(FUNCTION_TYPE ${FUNCTION_TYPE} CACHE STRING "The data type used for data")
      ENDIF()
       
      IF (NOT DEFINED GLOBAL_INDEX_TYPE)
         SET(GLOBAL_INDEX_TYPE "uint32_t" CACHE STRING "The data type used for the global index space")
      ELSE()
         SET(GLOBAL_INDEX_TYPE ${GLOBAL_INDEX_TYPE} CACHE STRING "The data type used for the global index space")
      ENDIF()
       
      IF (NOT DEFINED LOCAL_INDEX_TYPE)
         SET(LOCAL_INDEX_TYPE "uint32_t" CACHE STRING "The data type used for the local index space")
      ELSE()
         SET(LOCAL_INDEX_TYPE ${LOCAL_INDEX_TYPE} CACHE STRING "The data type used for the local index space")
      ENDIF()
       
      IF(CMAKE_VERBOSE_MAKEFILE)
          MESSAGE("Talass configuration:")
          MESSAGE("\tFunctionType    = ${FUNCTION_TYPE}")
          MESSAGE("\tGlobalIndexType = ${GLOBAL_INDEX_TYPE}")
          MESSAGE("\tLocalIndexType  = ${GLOBAL_INDEX_TYPE}")
      ENDIF()
             
      FIND_FILE(TALASS_CONFIG TalassConfig.cmake
                PATHS ${CMAKE_MODULE_PATH}
                )
      CONFIGURE_FILE(${TALASS_CONFIG} ${PROJECT_BINARY_DIR}/TalassConfig.h)
      INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
      INSTALL(FILES ${PROJECT_BINARY_DIR}/TalassConfig.h
              DESTINATION ${PROJECT_INCLUDE_DIR})


      # Set a a flag to produce a special Exe type. Useful especially for GUI stuff
      IF (APPLE)
        SET(EXECUTABLE_TYPE "MACOSX_BUNDLE")
      ENDIF()
    

      #Check if win32 multiprocessor compile option is available and set flags.
      IF (WIN32)
         OPTION(BUILD_WITH_MP "Enables the /MP multi-processor compiler option for Visual Studio 2005 and above" ON)
         MARK_AS_ADVANCED(BUILD_WITH_MP)
         IF(BUILD_WITH_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
         ENDIF()
      ENDIF()

      # By default we want to see everything
      SET(CMAKE_VERBOSE_MAKEFILE ON)# CACHE BOOL "Use a verbose makefile")

      # By default we will build in debug mode
      IF ( CMAKE_BUILD_TYPE STREQUAL "")
         SET(CMAKE_BUILD_TYPE "Debug")
      ENDIF (CMAKE_BUILD_TYPE STREQUAL "")

      IF (MSVC)
         # Force to always compile with W4
         IF (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
            string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
         ELSE()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
         ENDIF()
      ELSEIF (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
         # Update if necessary
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
      ENDIF()

   ENDIF()

   # Set the local binary directory to */project_name 
   SET(LOCAL_BINARY_DIR  ${PROJECT_BINARY_DIR}/${project_name})

ENDMACRO()


MACRO(TEST_CONFIG var default type doc)

    IF (NOT DEFINED TALASS_${var} AND NOT DEFINED ${var})
        SET(TALASS_${var} ${${default}} CACHE ${type} ${doc})
        IF(CMAKE_VERBOSE_MAKEFILE)
            MESSAGE("Setting TALASS_${var} = " ${${var}})
        ENDIF()
    ELSEIF(NOT DEFINED ${var})
        IF(CMAKE_VERBOSE_MAKEFILE)
            MESSAGE("Re-using old definition TALASS_${var} = " ${TALASS_${var}})
        ENDIF()
    ELSE ()
        SET(TALASS_${var} ${${var}} CACHE ${type} ${doc})
        IF(CMAKE_VERBOSE_MAKEFILE)
            MESSAGE("Setting TALASS_${var} = " ${${var}})
        ENDIF()
    ENDIF ()
 
    # The ccmake system uses ON/OFF for bools
    IF (${type} STREQUAL BOOL)
        IF(TALASS_${var})
            SET(TALSS_${var} ON)
        ELSE()
            SET(TALASS_${var} OFF)
        ENDIF()
    ENDIF()    
     
    
    SET(${var})
 
ENDMACRO()



   
