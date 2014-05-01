INCLUDE(TestingSetup)

# We should have one place that points to the cxx source directory and the cxx
# binary directory
SET(cxx_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
SET(cxx_binary_dir ${CMAKE_CURRENT_BINARY_DIR})

# CXX Add Dependencies Macro
# Author: Brian Panneton
# Description: This macro adds the cxx test dependencies.
# 	  Note: The tests already depend on their own file
# Parameters:         
#              dependencies         = any dependencies needed for cxx tests
MACRO(ADD_TEST_CXX_DEPENDENCIES dependencies)
    IF(NOT ("${dependencies}" STREQUAL ""))
        SET_PROPERTY(GLOBAL APPEND PROPERTY CXX_TEST_DEPENDENCIES
            "${dependencies}"
 	)
    ENDIF(NOT ("${dependencies}" STREQUAL ""))
ENDMACRO(ADD_TEST_CXX_DEPENDENCIES dependencies)

# Cxx Add LDPath  Macro
# Author: Brian Panneton
# Description: This macro adds the cxx test ldpaths.
# Parameters:         
#               ld  = any ldpaths needed for cxx tests
MACRO(ADD_TEST_CXX_LDPATH ld)
    GET_PROPERTY(ldpath GLOBAL PROPERTY CXX_TEST_LDPATH)
    IF("${ld}" STRGREATER "")
        SET_PROPERTY(GLOBAL PROPERTY CXX_TEST_LDPATH 
                "${ldpath}${sep}${ld}" 
        )
    ENDIF("${ld}" STRGREATER "")
ENDMACRO(ADD_TEST_CXX_LDPATH ld)

# Cxx Add Path  Macro
# Author: Brian Panneton
# Description: This macro adds the cxx test paths.
# Parameters:         
#               p  = any paths needed for cxx tests
MACRO(ADD_TEST_CXX_PATH p)
    GET_PROPERTY(path GLOBAL PROPERTY CXX_TEST_PATH)
    IF("${p}" STRGREATER "")
        SET_PROPERTY(GLOBAL PROPERTY CXX_TEST_PATH 
                "${path}${sep}${p}" 
        )
    ENDIF("${p}" STRGREATER "")
ENDMACRO(ADD_TEST_CXX_PATH p)

# CXX Test Macro
# Author: Brian Panneton
# Description: This macro builds and add the cxx test in one shot.
# Parameters:         
#		executable      = executable name
#		dup		= duplicate number
#		tdep		= test dependency (Full Test Name with Prefix)
#             	${ARGN}         = any arguments for the executable
MACRO(ADD_TEST_CXX executable)
    PARSE_TEST_ARGS("${ARGN}")
    
    IF(EXISTS ${cxx_source_dir}/${executable}.cpp)
        ADD_EXECUTABLE(${executable}${dup} ${cxx_source_dir}/${executable}.cpp)
    ENDIF(EXISTS ${cxx_source_dir}/${executable}.cpp)

    IF(EXISTS ${cxx_source_dir}/${executable}.cxx)
        ADD_EXECUTABLE(${executable}${dup} ${cxx_source_dir}/${executable}.cxx)
    ENDIF(EXISTS ${cxx_source_dir}/${executable}.cxx)
	
    GET_PROPERTY(cxx_dependencies GLOBAL PROPERTY CXX_TEST_DEPENDENCIES)
    GET_PROPERTY(cxx_ldpath GLOBAL PROPERTY CXX_TEST_LDPATH)
    GET_PROPERTY(cxx_path GLOBAL PROPERTY CXX_TEST_PATH)
    TARGET_LINK_LIBRARIES(${executable}${dup} ${cxx_dependencies})
 
    # Take care of windowisims
    IF(WIN32)
        SET_TARGET_PROPERTIES(${executable}${dup} PROPERTIES 
            PREFIX ../
            RUNTIME_OUTPUT_DIRECTORY ${cxx_binary_dir}
            LIBRARY_OUTPUT_DIRECTORY ${cxx_binary_dir}
            ARCHIVE_OUTPUT_DIRECTORY ${cxx_binary_dir})

        IF("${cxx_path}" STREQUAL "")
            SET(cxx_path ${cxx_ldpath})
        ENDIF("${cxx_path}" STREQUAL "")
    ENDIF(WIN32)

    SET_CORE("${cxx_binary_dir}")
    ADD_TEST(Cxx${is_core}_${executable}${dup} ${CMAKE_COMMAND}
            -D "EXECUTABLE=${executable}${dup}"
            -D "ARGUMENTS=${arguments}"
            -D "LDPATH=${cxx_ldpath}"
            -D "PATH=${cxx_path}"
            -D "SEPARATOR=${sep}"
            -P "${cxx_binary_dir}/TestDriverCxx.cmake"
    )
    IF(NOT "${tdep}" STREQUAL "")
	    SET_TESTS_PROPERTIES(Cxx${is_core}_${executable}${dup}
            PROPERTIES DEPENDS ${tdep})
    ENDIF(NOT "${tdep}" STREQUAL "")
ENDMACRO(ADD_TEST_CXX executable)

# CXX Clean Macro
# Author: Brian Panneton
# Description: This macro sets up the cxx test for a make clean.
# Parameters:         
#		executable      = executable name
#             	${ARGN}         = files that the executable created
MACRO(CLEAN_TEST_CXX executable)
    set_property(DIRECTORY APPEND PROPERTY 
        ADDITIONAL_MAKE_CLEAN_FILES ${ARGN} 
    )
ENDMACRO(CLEAN_TEST_CXX executable)

 # Configure the cxx 'driver' file
CONFIGURE_FILE(${TESTING_SUITE_DIR}/TestingSuite/TestDriverCxx.cmake.in ${cxx_binary_dir}/TestDriverCxx.cmake @ONLY)

