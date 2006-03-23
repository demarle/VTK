# Generate the VTKConfig.cmake file in the build tree.  Also configure
# one for installation.  The file tells external projects how to use
# VTK.

# Help store a literal dollar in a string.  CMake 2.2 allows escaped
# dollars but we have to support CMake 2.0.
SET(DOLLAR "$")

#-----------------------------------------------------------------------------
# Settings shared between the build tree and install tree.

IF(VTK_USE_MPI)
  SET(VTK_MPIRUN_EXE_CONFIG ${VTK_MPIRUN_EXE})
  SET(VTK_MPI_MAX_NUMPROCS_CONFIG ${VTK_MPI_MAX_NUMPROCS})
  SET(VTK_MPI_POSTFLAGS_CONFIG ${VTK_MPI_POSTFLAGS})
  SET(VTK_MPI_PREFLAGS_CONFIG ${VTK_MPI_PREFLAGS})
  SET(VTK_MPI_CLIENT_PREFLAGS_CONFIG ${VTK_MPI_CLIENT_PREFLAGS})
  SET(VTK_MPI_SERVER_PREFLAGS_CONFIG ${VTK_MPI_SERVER_PREFLAGS})
ELSE(VTK_USE_MPI)
  SET(VTK_MPIRUN_EXE_CONFIG "")
  SET(VTK_MPI_MAX_NUMPROCS_CONFIG "")
  SET(VTK_MPI_POSTFLAGS_CONFIG "")
  SET(VTK_MPI_PREFLAGS_CONFIG "")
  SET(VTK_MPI_CLIENT_PREFLAGS_CONFIG "")
  SET(VTK_MPI_SERVER_PREFLAGS_CONFIG "")
ENDIF(VTK_USE_MPI)

# Some setting for the remote testing
SET(VTK_MPI_CLIENT_POSTFLAGS_CONFIG ${VTK_MPI_CLIENT_POSTFLAGS})
SET(VTK_MPI_SERVER_POSTFLAGS_CONFIG ${VTK_MPI_SERVER_POSTFLAGS})

#-----------------------------------------------------------------------------
# Settings specific to the build tree.

# The install-only section is empty for the build tree.
SET(VTK_CONFIG_INSTALL_ONLY)

# The "use" file.
SET(VTK_USE_FILE ${VTK_BINARY_DIR}/UseVTK.cmake)

# The build settings file.
SET(VTK_BUILD_SETTINGS_FILE ${VTK_BINARY_DIR}/VTKBuildSettings.cmake)

# The directory containing class list files for each kit.
SET(VTK_KITS_DIR_CONFIG ${VTK_BINARY_DIR}/Utilities)

# The wrapping hints file.
SET(VTK_WRAP_HINTS_CONFIG ${VTK_WRAP_HINTS})

# Library directory.
SET(VTK_LIBRARY_DIRS_CONFIG ${LIBRARY_OUTPUT_PATH})

# Runtime library directory.
SET(VTK_RUNTIME_LIBRARY_DIRS_CONFIG ${LIBRARY_OUTPUT_PATH})

# Determine the include directories needed.
SET(VTK_INCLUDE_DIRS_CONFIG
  ${VTK_INCLUDE_DIRS_BUILD_TREE}
  ${VTK_INCLUDE_DIRS_SOURCE_TREE}
  ${VTK_INCLUDE_DIRS_SYSTEM}
)

# Executable locations.
SET(VTK_TCL_HOME_CONFIG "")
SET(VTK_JAVA_JAR_CONFIG "")
SET(VTK_PARSE_JAVA_EXE_CONFIG "")
SET(VTK_WRAP_JAVA_EXE_CONFIG "")
SET(VTK_WRAP_PYTHON_EXE_CONFIG "")
SET(VTK_WRAP_PYTHON_INIT_EXE_CONFIG "")
SET(VTK_WRAP_TCL_EXE_CONFIG "")
SET(VTK_WRAP_TCL_INIT_EXE_CONFIG "")
IF(VTK_WRAP_TCL)
  SET(VTK_WRAP_TCL_EXE_CONFIG ${VTK_WRAP_TCL_EXE})
  SET(VTK_WRAP_TCL_INIT_EXE_CONFIG ${VTK_WRAP_TCL_INIT_EXE})
  SET(VTK_TCL_HOME_CONFIG ${VTK_TCL_HOME})
ENDIF(VTK_WRAP_TCL)
IF(VTK_WRAP_PYTHON)
  SET(VTK_WRAP_PYTHON_EXE_CONFIG ${VTK_WRAP_PYTHON_EXE})
  SET(VTK_WRAP_PYTHON_INIT_EXE_CONFIG ${VTK_WRAP_PYTHON_INIT_EXE})
ENDIF(VTK_WRAP_PYTHON)
IF(VTK_WRAP_JAVA)
  SET(VTK_PARSE_JAVA_EXE_CONFIG ${VTK_PARSE_JAVA_EXE})
  SET(VTK_WRAP_JAVA_EXE_CONFIG ${VTK_WRAP_JAVA_EXE})
  SET(VTK_JAVA_JAR_CONFIG ${LIBRARY_OUTPUT_PATH}/vtk.jar)
ENDIF(VTK_WRAP_JAVA)

# VTK style script locations.
SET(VTK_DOXYGEN_HOME_CONFIG ${VTK_SOURCE_DIR}/Utilities/Doxygen)
SET(VTK_HEADER_TESTING_PY_CONFIG ${VTK_SOURCE_DIR}/Common/Testing/HeaderTesting.py)
SET(VTK_FIND_STRING_TCL_CONFIG ${VTK_SOURCE_DIR}/Common/Testing/Tcl/FindString.tcl)
SET(VTK_PRINT_SELF_CHECK_TCL_CONFIG ${VTK_SOURCE_DIR}/Common/Testing/Tcl/PrintSelfCheck.tcl)
SET(VTK_RT_IMAGE_TEST_TCL_CONFIG ${VTK_SOURCE_DIR}/Common/Testing/Tcl/rtImageTest.tcl)

IF(VTK_USE_PARALLEL)
  SET(VTK_PRT_IMAGE_TEST_TCL_CONFIG ${VTK_SOURCE_DIR}/Common/Testing/Tcl/prtImageTest.tcl)
ELSE(VTK_USE_PARALLEL)
  SET(VTK_PRT_IMAGE_TEST_TCL_CONFIG "")
ENDIF(VTK_USE_PARALLEL)

# Location of tk internal headers and resources provided by VTK.
IF(VTK_RENDERING_NEED_TK_INTERNAL)
  SET(VTK_TK_INTERNAL_DIR_CONFIG ${TK_INTERNAL_PATH})
ELSE(VTK_RENDERING_NEED_TK_INTERNAL)
  SET(VTK_TK_INTERNAL_DIR_CONFIG "")
ENDIF(VTK_RENDERING_NEED_TK_INTERNAL)

# The resources dir is only worth exporting if we are building with
# Tcl/Tk static
SET(VTK_TK_RESOURCES_DIR_CONFIG "")
IF(VTK_USE_TK AND VTK_TCL_TK_STATIC)
  SET(VTK_TK_RESOURCES_DIR_CONFIG ${VTK_TK_RESOURCES_DIR})
ENDIF(VTK_USE_TK AND VTK_TCL_TK_STATIC)

SET(VTK_TCL_SUPPORT_LIBRARY_PATH_CONFIG ${VTK_TCL_SUPPORT_LIBRARY_PATH})
SET(VTK_TK_SUPPORT_LIBRARY_PATH_CONFIG ${VTK_TK_SUPPORT_LIBRARY_PATH})

# CMake extension module directory.
SET(VTK_LOAD_CMAKE_EXTENSIONS_MACRO_CONFIG
    "${VTK_SOURCE_DIR}/CMake/vtkLoadCMakeExtensions.cmake")
SET(VTK_CMAKE_DIR_CONFIG
    "${VTK_SOURCE_DIR}/CMake")
SET(VTK_TCL_TK_MACROS_MODULE_CONFIG
    "${VTK_SOURCE_DIR}/CMake/vtkTclTkMacros.cmake")
SET(VTK_CMAKE_EXTENSIONS_DIR_CONFIG ${VTK_BINARY_DIR}/CMake)

# Library dependencies file.
SET(VTK_LIBRARY_DEPENDS_FILE "${VTK_BINARY_DIR}/VTKLibraryDepends.cmake")

# Build configuration information.
SET(VTK_CONFIGURATION_TYPES_CONFIG ${CMAKE_CONFIGURATION_TYPES})
SET(VTK_BUILD_TYPE_CONFIG ${CMAKE_BUILD_TYPE})

# Hack to give source tree access for a build tree configuration.
STRING(ASCII 35 VTK_STRING_POUND)
STRING(ASCII 64 VTK_STRING_AT)
SET(VTK_CONFIG_BACKWARD_COMPATIBILITY_HACK
  "\n${VTK_STRING_POUND} For backward compatability.  DO NOT USE.\nSET(VTK_SOURCE_DIR \"${VTK_SOURCE_DIR}\")\nIF(NOT TCL_LIBRARY)\n  SET(TCL_LIBRARY \"${TCL_LIBRARY}\" CACHE FILEPATH \"Location of Tcl library imported from VTK.  This may mean your project is depending on VTK to get this setting.  Consider using FindTCL.cmake.\")\nENDIF(NOT TCL_LIBRARY)\nIF(NOT TK_LIBRARY)\n  SET(TK_LIBRARY \"${TK_LIBRARY}\" CACHE FILEPATH \"Location of Tk library imported from VTK.  This may mean your project is depending on VTK to get this setting.  Consider using FindTCL.cmake.\")\nENDIF(NOT TK_LIBRARY)\nMARK_AS_ADVANCED(TCL_LIBRARY TK_LIBRARY)\n")

#-----------------------------------------------------------------------------
# Configure VTKConfig.cmake for the build tree.
CONFIGURE_FILE(${VTK_SOURCE_DIR}/VTKConfig.cmake.in
               ${VTK_BINARY_DIR}/VTKConfig.cmake @ONLY IMMEDIATE)

#-----------------------------------------------------------------------------
# Settings specific to the install tree.

# The "use" file.
SET(VTK_USE_FILE ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/UseVTK.cmake)

# The build settings file.
SET(VTK_BUILD_SETTINGS_FILE ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/VTKBuildSettings.cmake)

# The directory containing class list files for each kit.
SET(VTK_KITS_DIR_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR})

# The wrapping hints file.
IF(VTK_WRAP_HINTS)
  GET_FILENAME_COMPONENT(VTK_HINTS_FNAME ${VTK_WRAP_HINTS} NAME)
  SET(VTK_WRAP_HINTS_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/${VTK_HINTS_FNAME})
ENDIF(VTK_WRAP_HINTS)

# Include directories.
SET(VTK_INCLUDE_DIRS_CONFIG
  ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_INCLUDE_DIR}
  ${VTK_INCLUDE_DIRS_SYSTEM}
)

# Link directories.
IF(CYGWIN AND BUILD_SHARED_LIBS)
  # In Cygwin programs directly link to the .dll files.
  SET(VTK_LIBRARY_DIRS_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_BIN_DIR})
ELSE(CYGWIN AND BUILD_SHARED_LIBS)
  SET(VTK_LIBRARY_DIRS_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_LIB_DIR})
ENDIF(CYGWIN AND BUILD_SHARED_LIBS)

# Runtime directories.
IF(WIN32)
  SET(VTK_RUNTIME_LIBRARY_DIRS_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_BIN_DIR})
ELSE(WIN32)
  SET(VTK_RUNTIME_LIBRARY_DIRS_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_LIB_DIR})
ENDIF(WIN32)

IF(WIN32)
  SET(VTK_EXE_EXT ".exe")
ENDIF(WIN32)

# Executable locations.
SET(VTK_TCL_HOME_CONFIG "")
SET(VTK_JAVA_JAR_CONFIG "")
SET(VTK_PARSE_JAVA_EXE_CONFIG "")
SET(VTK_WRAP_JAVA_EXE_CONFIG "")
SET(VTK_WRAP_PYTHON_EXE_CONFIG "")
SET(VTK_WRAP_PYTHON_INIT_EXE_CONFIG "")
SET(VTK_WRAP_TCL_EXE_CONFIG "")
SET(VTK_WRAP_TCL_INIT_EXE_CONFIG "")
SET(VTK_DOXYGEN_HOME_CONFIG "")
IF(VTK_WRAP_TCL)
  SET(VTK_WRAP_TCL_EXE_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_BIN_DIR}/vtkWrapTcl${VTK_EXE_EXT})
  SET(VTK_WRAP_TCL_INIT_EXE_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_BIN_DIR}/vtkWrapTclInit${VTK_EXE_EXT})
  SET(VTK_TCL_HOME_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_TCL_DIR})
ENDIF(VTK_WRAP_TCL)
IF(VTK_WRAP_PYTHON)
  SET(VTK_WRAP_PYTHON_EXE_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_BIN_DIR}/vtkWrapPython${VTK_EXE_EXT})
  SET(VTK_WRAP_PYTHON_INIT_EXE_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_BIN_DIR}/vtkWrapPythonInit${VTK_EXE_EXT})
ENDIF(VTK_WRAP_PYTHON)
IF(VTK_WRAP_JAVA)
  SET(VTK_PARSE_JAVA_EXE_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_BIN_DIR}/vtkParseJava${VTK_EXE_EXT})
  SET(VTK_WRAP_JAVA_EXE_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_BIN_DIR}/vtkWrapJava${VTK_EXE_EXT})
  SET(VTK_JAVA_JAR_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_JAVA_DIR}/vtk.jar)
ENDIF(VTK_WRAP_JAVA)

# VTK style script locations.
SET(VTK_DOXYGEN_HOME_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_DOXYGEN_DIR})
SET(VTK_HEADER_TESTING_PY_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/testing/HeaderTesting.py)
SET(VTK_FIND_STRING_TCL_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/testing/FindString.tcl)
SET(VTK_PRINT_SELF_CHECK_TCL_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/testing/PrintSelfCheck.tcl)
SET(VTK_RT_IMAGE_TEST_TCL_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/testing/rtImageTest.tcl)

IF(VTK_USE_PARALLEL)
  SET(VTK_PRT_IMAGE_TEST_TCL_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/testing/prtImageTest.tcl)
ELSE(VTK_USE_PARALLEL)
  SET(VTK_PRT_IMAGE_TEST_TCL_CONFIG "")
ENDIF(VTK_USE_PARALLEL)

# Location of tk internal headers and resources provided by VTK.
SET(VTK_TK_INTERNAL_DIR_CONFIG "")
IF(VTK_RENDERING_NEED_TK_INTERNAL AND TK_INTERNAL_PATH)
  VTK_GET_TCL_TK_VERSION ("TCL_TK_MAJOR_VERSION" "TCL_TK_MINOR_VERSION")
  SET (TCL_TK_VERSION "${TCL_TK_MAJOR_VERSION}.${TCL_TK_MINOR_VERSION}")
  IF("${TK_INTERNAL_PATH}" MATCHES 
     "Utilities/TclTk/internals/tk${TCL_TK_VERSION}")
    SET(VTK_TK_INTERNAL_DIR_CONFIG 
        ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_INCLUDE_DIR}/TclTk/internals/tk${TCL_TK_VERSION})
  ENDIF("${TK_INTERNAL_PATH}" MATCHES 
        "Utilities/TclTk/internals/tk${TCL_TK_VERSION}")
ENDIF(VTK_RENDERING_NEED_TK_INTERNAL AND TK_INTERNAL_PATH)

SET(VTK_TK_INTERNAL_ROOT_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_INCLUDE_DIR})

# The resources dir is only worth exporting if we are building with
# Tcl/Tk static
SET(VTK_TK_RESOURCES_DIR_CONFIG "")
IF(VTK_USE_TK AND VTK_TCL_TK_STATIC)
  IF(VTK_TK_RESOURCES_DIR)
    VTK_GET_TCL_TK_VERSION ("TCL_TK_MAJOR_VERSION" "TCL_TK_MINOR_VERSION")
    SET (TCL_TK_VERSION "${TCL_TK_MAJOR_VERSION}.${TCL_TK_MINOR_VERSION}")
    IF("${VTK_TK_RESOURCES_DIR}" MATCHES 
        "Utilities/TclTk/resources/tk${TCL_TK_VERSION}")
      SET(VTK_TK_RESOURCES_DIR_CONFIG 
        ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_INCLUDE_DIR}/TclTk/resources/tk${TCL_TK_VERSION}/win/rc)
    ENDIF("${VTK_TK_RESOURCES_DIR}" MATCHES 
      "Utilities/TclTk/resources/tk${TCL_TK_VERSION}")
  ENDIF(VTK_TK_RESOURCES_DIR)
ENDIF(VTK_USE_TK AND VTK_TCL_TK_STATIC)

IF(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)
  VTK_GET_TCL_TK_VERSION ("TCL_TK_MAJOR_VERSION" "TCL_TK_MINOR_VERSION")
  SET (TCL_TK_VERSION "${TCL_TK_MAJOR_VERSION}.${TCL_TK_MINOR_VERSION}")
  SET(VTK_TCL_SUPPORT_LIBRARY_PATH_CONFIG 
    ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_TCL_DIR}/tcl${TCL_TK_VERSION})
  SET(VTK_TK_SUPPORT_LIBRARY_PATH_CONFIG 
    ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_TCL_DIR}/tk${TCL_TK_VERSION})
ELSE(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)
  SET(VTK_TCL_SUPPORT_LIBRARY_PATH_CONFIG ${VTK_TCL_SUPPORT_LIBRARY_PATH})
  SET(VTK_TK_SUPPORT_LIBRARY_PATH_CONFIG ${VTK_TK_SUPPORT_LIBRARY_PATH})
ENDIF(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

# CMake extension module directory and macro file.
SET(VTK_LOAD_CMAKE_EXTENSIONS_MACRO_CONFIG
    "${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/CMake/vtkLoadCMakeExtensions.cmake")
SET(VTK_CMAKE_DIR_CONFIG
    "${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/CMake")
SET(VTK_TCL_TK_MACROS_MODULE_CONFIG
    "${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/CMake/vtkTclTkMacros.cmake")
SET(VTK_CMAKE_EXTENSIONS_DIR_CONFIG ${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/CMake)

# Library dependencies file.
SET(VTK_LIBRARY_DEPENDS_FILE "${DOLLAR}{VTK_INSTALL_PREFIX}${VTK_INSTALL_PACKAGE_DIR}/VTKLibraryDepends.cmake")

# No backward compatibility hack needed for installed path
SET(VTK_CONFIG_BACKWARD_COMPATIBILITY_HACK)

#-----------------------------------------------------------------------------
# Configure VTKConfig.cmake for the install tree.

# Construct the proper number of GET_FILENAME_COMPONENT(... PATH)
# calls to compute the installation prefix from VTK_DIR.
STRING(REGEX REPLACE "/" ";" VTK_INSTALL_PACKAGE_DIR_COUNT
  "${VTK_INSTALL_PACKAGE_DIR}")
SET(VTK_CONFIG_INSTALL_ONLY "
# Compute the installation prefix from VTK_DIR.
SET(VTK_INSTALL_PREFIX \"${DOLLAR}{VTK_DIR}\")
")
FOREACH(p ${VTK_INSTALL_PACKAGE_DIR_COUNT})
  SET(VTK_CONFIG_INSTALL_ONLY
    "${VTK_CONFIG_INSTALL_ONLY}GET_FILENAME_COMPONENT(VTK_INSTALL_PREFIX \"${DOLLAR}{VTK_INSTALL_PREFIX}\" PATH)\n"
    )
ENDFOREACH(p)

# The install tree only has one configuration.
SET(VTK_CONFIGURATION_TYPES_CONFIG)

IF(CMAKE_CONFIGURATION_TYPES)
  # There are multiple build configurations.  Configure one
  # VTKConfig.cmake for each configuration.
  FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
    SET(VTK_BUILD_TYPE_CONFIG ${config})
    CONFIGURE_FILE(${VTK_SOURCE_DIR}/VTKConfig.cmake.in
                   ${VTK_BINARY_DIR}/Utilities/${config}/VTKConfig.cmake
                   @ONLY IMMEDIATE)
  ENDFOREACH(config)

  # Install the config file corresponding to the build configuration
  # specified when building the install target.  The BUILD_TYPE variable
  # will be set while CMake is processing the install files.
  IF(NOT VTK_INSTALL_NO_DEVELOPMENT)
    INSTALL_FILES(${VTK_INSTALL_PACKAGE_DIR} FILES
      ${VTK_BINARY_DIR}/Utilities/${DOLLAR}{BUILD_TYPE}/VTKConfig.cmake)
  ENDIF(NOT VTK_INSTALL_NO_DEVELOPMENT)
ELSE(CMAKE_CONFIGURATION_TYPES)
  # There is only one build configuration.  Configure one VTKConfig.cmake.
  SET(VTK_BUILD_TYPE_CONFIG ${CMAKE_BUILD_TYPE})
  CONFIGURE_FILE(${VTK_SOURCE_DIR}/VTKConfig.cmake.in
                 ${VTK_BINARY_DIR}/Utilities/VTKConfig.cmake @ONLY IMMEDIATE)

  # Setup an install rule for the config file.
  IF(NOT VTK_INSTALL_NO_DEVELOPMENT)
    INSTALL_FILES(${VTK_INSTALL_PACKAGE_DIR} FILES
      ${VTK_BINARY_DIR}/Utilities/VTKConfig.cmake)
  ENDIF(NOT VTK_INSTALL_NO_DEVELOPMENT)
ENDIF(CMAKE_CONFIGURATION_TYPES)
