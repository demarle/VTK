#
# This module add the VTK include paths to a project
# It should be included after the FindVTK module
#
IF (USE_INSTALLED_VTK)
  IF (VTK_INSTALL_PATH)
    LOAD_CACHE(${VTK_INSTALL_PATH}/include/vtk 
      EXCLUDE
        BUILD_SHARED_LIBS
        BUILD_TESTING
        BUILD_EXAMPLES
        LIBRARY_OUTPUT_PATH
        EXECUTABLE_OUTPUT_PATH
        MAKECOMMAND
        SITE
        BUILDNAME
      INCLUDE_INTERNALS
        VTK_BINARY_DIR
        VTK_LIBRARY_PATH
        VTK_BUILD_SHARED_LIBS
      )
    INCLUDE_DIRECTORIES ( ${VTK_INSTALL_PATH}/include/vtk )
    LINK_DIRECTORIES(${VTK_INSTALL_PATH}/lib/vtk)
  ENDIF (VTK_INSTALL_PATH)
ELSE (USE_INSTALLED_VTK)
  IF (VTK_BINARY_PATH)
    LOAD_CACHE(${VTK_BINARY_PATH} 
      EXCLUDE
        BUILD_SHARED_LIBS
        BUILD_TESTING
        BUILD_EXAMPLES
        LIBRARY_OUTPUT_PATH
        EXECUTABLE_OUTPUT_PATH
        MAKECOMMAND 
        SITE
        BUILDNAME
      INCLUDE_INTERNALS
        VTK_BINARY_DIR
        VTK_LIBRARY_PATH
        VTK_BUILD_SHARED_LIBS
      )
    INCLUDE (${VTK_SOURCE_DIR}/vtkCMakeOptions.cmake)
    LINK_DIRECTORIES(${VTK_LIBRARY_PATH})
  ENDIF (VTK_BINARY_PATH)
ENDIF (USE_INSTALLED_VTK)

IF (VTK_USE_RENDERING)
  # add in opengl
  IF (WIN32)
    LINK_LIBRARIES ( ${OPENGL_LIBRARY} )
  ELSE (WIN32)
    IF (OPENGL_LIBRARY)
      LINK_LIBRARIES (${OPENGL_LIBRARY})
    ENDIF (OPENGL_LIBRARY)
  ENDIF (WIN32)
  IF (VTK_MANGLE_MESA)
    LINK_LIBRARIES( ${MESA_LIBRARY})
    IF ( MESA_OS_LIBRARY )
      LINK_LIBRARIES(${MESA_OS_LIBRARY})
    ENDIF ( MESA_OS_LIBRARY )
  ENDIF (VTK_MANGLE_MESA)

  # add in xwindows stuff
  IF (CMAKE_HAS_X)
    LINK_LIBRARIES(-lXt)
    LINK_LIBRARIES(${CMAKE_X_LIBS})
    ADD_DEFINITIONS(${CMAKE_X_CFLAGS})
  ENDIF (CMAKE_HAS_X)
ENDIF (VTK_USE_RENDERING)

IF (UNIX)
  LINK_LIBRARIES(${THREAD_LIBS} ${DL_LIBS} -lm)
ENDIF (UNIX)
