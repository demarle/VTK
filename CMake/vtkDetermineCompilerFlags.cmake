SET(VTK_REQUIRED_C_FLAGS)
SET(VTK_REQUIRED_CXX_FLAGS)

# make sure Crun is linked in with the native compiler, it is
# not used by default for shared libraries and is required for
# things like java to work.
IF(CMAKE_SYSTEM MATCHES "SunOS.*")
  IF(NOT CMAKE_COMPILER_IS_GNUCXX)
    FIND_LIBRARY(VTK_SUNCC_CRUN_LIBRARY Crun /opt/SUNWspro/lib)
    IF(VTK_SUNCC_CRUN_LIBRARY)
      LINK_LIBRARIES(${VTK_SUNCC_CRUN_LIBRARY})
    ENDIF(VTK_SUNCC_CRUN_LIBRARY)
    FIND_LIBRARY(VTK_SUNCC_CSTD_LIBRARY Cstd /opt/SUNWspro/lib)
    IF(VTK_SUNCC_CSTD_LIBRARY)
      LINK_LIBRARIES(${VTK_SUNCC_CSTD_LIBRARY})
    ENDIF(VTK_SUNCC_CSTD_LIBRARY)
  ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM MATCHES "SunOS.*")

# A GCC compiler.
IF(CMAKE_COMPILER_IS_GNUCXX)
  # Quiet warning about strstream deprecation if appropriate.
  IF(NOT VTK_LEGACY_REMOVE)
    SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -Wno-deprecated")
  ENDIF(NOT VTK_LEGACY_REMOVE)
  IF(WIN32)
# The platform is gcc on cygwin.
    SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -mwin32")
    SET(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} -mwin32")
  ENDIF(WIN32)
  IF(MINGW)
    SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -mthreads")
    SET(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} -mthreads")
    SET(VTK_REQUIRED_EXE_LINKER_FLAGS "${VTK_REQUIRED_EXE_LINKER_FLAGS} -mthreads")
    SET(VTK_REQUIRED_SHARED_LINKER_FLAGS "${VTK_REQUIRED_SHARED_LINKER_FLAGS} -mthreads")
    SET(VTK_REQUIRED_MODULE_LINKER_FLAGS "${VTK_REQUIRED_MODULE_LINKER_FLAGS} -mthreads")
    LINK_LIBRARIES(-lgdi32)
  ENDIF(MINGW)
  IF(CMAKE_SYSTEM MATCHES "SunOS.*")
# Disable warnings that occur in X11 headers.
    IF(DART_ROOT AND BUILD_TESTING)
      SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -Wno-unknown-pragmas")
      SET(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} -Wno-unknown-pragmas")
    ENDIF(DART_ROOT AND BUILD_TESTING)
  ENDIF(CMAKE_SYSTEM MATCHES "SunOS.*")
ELSE(CMAKE_COMPILER_IS_GNUCXX)
  IF(CMAKE_ANSI_CFLAGS)
    SET(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
  ENDIF(CMAKE_ANSI_CFLAGS)
  IF(CMAKE_SYSTEM MATCHES "OSF1-V.*")
     SET(VTK_REQUIRED_CXX_FLAGS
         "${VTK_REQUIRED_CXX_FLAGS} -timplicit_local -no_implicit_include")
  ENDIF(CMAKE_SYSTEM MATCHES "OSF1-V.*")
  IF(CMAKE_SYSTEM MATCHES "IRIX.*")
    SET(VTK_REQUIRED_CXX_FLAGS
      "${VTK_REQUIRED_CXX_FLAGS} -Wl,-woff84 -woff 15 -woff 84 -woff 3439 -woff 1424 -woff 3201")
    SET(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} -woff 3439")
  ENDIF(CMAKE_SYSTEM MATCHES "IRIX.*")
  IF(CMAKE_SYSTEM MATCHES "AIX.*")
    # allow t-ypeid and d-ynamic_cast usage (normally off by default on xlC)
    SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -qrtti=all")
    # silence duplicate symbol warnings on AIX
    SET(VTK_REQUIRED_EXE_LINKER_FLAGS "${VTK_REQUIRED_EXE_LINKER_FLAGS} -bhalt:5")
    SET(VTK_REQUIRED_SHARED_LINKER_FLAGS "${VTK_REQUIRED_SHARED_LINKER_FLAGS} -bhalt:5")
    SET(VTK_REQUIRED_MODULE_LINKER_FLAGS "${VTK_REQUIRED_MODULE_LINKER_FLAGS} -bhalt:5")
  ENDIF(CMAKE_SYSTEM MATCHES "AIX.*")
  IF(CMAKE_SYSTEM MATCHES "HP-UX.*")
     SET(VTK_REQUIRED_C_FLAGS
         "${VTK_REQUIRED_C_FLAGS} +W2111 +W2236 +W4276")
     SET(VTK_REQUIRED_CXX_FLAGS
         "${VTK_REQUIRED_CXX_FLAGS} +W2111 +W2236 +W4276")
  ENDIF(CMAKE_SYSTEM MATCHES "HP-UX.*")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

IF(APPLE)
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -Wl,-flat_namespace,-U,_environ")
  SET(CMAKE_SHARED_MODULE_CREATE_C_FLAGS "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -Wl,-flat_namespace,-U,_environ")
  IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} -no-cpp-precomp")
    SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -no-cpp-precomp")
    IF(NOT BUILD_SHARED_LIBS)
      SET(VTK_REQUIRED_C_FLAGS "${VTK_REQUIRED_C_FLAGS} -mlong-branch")
      SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -mlong-branch")
    ENDIF(NOT BUILD_SHARED_LIBS)
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(APPLE)

# figure out whether the compiler might be the Intel compiler
SET(_MAY_BE_INTEL_COMPILER FALSE)
IF(UNIX)
  IF(CMAKE_CXX_COMPILER_ID)
    IF(CMAKE_CXX_COMPILER_ID MATCHES Intel)
      SET(_MAY_BE_INTEL_COMPILER TRUE)
    ENDIF(CMAKE_CXX_COMPILER_ID MATCHES Intel)
  ELSE(CMAKE_CXX_COMPILER_ID)
    IF(NOT CMAKE_COMPILER_IS_GNUCXX)
      SET(_MAY_BE_INTEL_COMPILER TRUE)
    ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)
  ENDIF(CMAKE_CXX_COMPILER_ID)
ENDIF(UNIX)

#if so, test whether -i_dynamic is needed
IF(_MAY_BE_INTEL_COMPILER)
  INCLUDE(${VTK_CMAKE_DIR}/TestNO_ICC_IDYNAMIC_NEEDED.cmake)
  TESTNO_ICC_IDYNAMIC_NEEDED(NO_ICC_IDYNAMIC_NEEDED ${VTK_CMAKE_DIR})
  IF(NO_ICC_IDYNAMIC_NEEDED)
    SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS}")
  ELSE(NO_ICC_IDYNAMIC_NEEDED)
    SET(VTK_REQUIRED_CXX_FLAGS "${VTK_REQUIRED_CXX_FLAGS} -i_dynamic")
  ENDIF(NO_ICC_IDYNAMIC_NEEDED)
ENDIF(_MAY_BE_INTEL_COMPILER)


IF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake|VCExpress)")
# Use the highest warning level for visual studio.
  SET(CMAKE_CXX_WARNING_LEVEL 4)
  IF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  ELSE(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  ENDIF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
ENDIF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake|VCExpress)")

# Disable deprecation warnings for standard C and STL functions in VS2005
# and later
IF(CMAKE_COMPILER_2005)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_DEPRECATE)
ENDIF(CMAKE_COMPILER_2005) 

#-----------------------------------------------------------------------------
# Add compiler flags VTK needs to work on this platform.  This must be
# done after the call to CMAKE_EXPORT_BUILD_SETTINGS, but before any
# try-compiles are done.
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VTK_REQUIRED_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VTK_REQUIRED_CXX_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${VTK_REQUIRED_EXE_LINKER_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${VTK_REQUIRED_SHARED_LINKER_FLAGS}")
SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${VTK_REQUIRED_MODULE_LINKER_FLAGS}")
