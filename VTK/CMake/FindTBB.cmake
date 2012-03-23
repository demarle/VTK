# Try to find (x)Kaapi
# Once done, this will define
#
#  XKAAPI_FOUND - system has (x)Kaapi
#  XKAAPI_INCLUDE_DIRS - the (x)Kaapi include directories
#  XKAAPI_LIBRARIES - link these to use (x)Kaapi

set(TBB_HOME $ENV{TBB_HOME} CACHE PATH "Path to the TBB install dir")

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_TBB QUIET tbb)
endif(PKG_CONFIG_FOUND)

find_path(TBB_INCLUDE_DIR tbb.h
  HINTS
  ${TBB_HOME}/include
  ${PC_TBB_INCLUDEDIR}
  ${PC_TBB_INCLUDE_DIRS}
  PATH_SUFFIXES "" "tbb"
)

find_library(TBB_LIBRARY tbb
  HINTS
  ${TBB_HOME}/lib
  ${PC_TBB_LIBDIR}
  ${PC_TBB_LIBRARY_DIRS}
  PATH_SUFFIXES "" "tbb"
)

set(TBB_LIBRARIES ${TBB_LIBRARY})
set(TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set XKAAPI_FOUND to TRUE if all listed variables are TRUE
find_package_handle_standard_args(TBB DEFAULT_MSG TBB_LIBRARIES TBB_INCLUDE_DIRS)

mark_as_advanced(TBB_INCLUDE_DIR TBB_LIBRARY)
set(TBB_LIBRARIES ${TBB_LIBRARIES} ${CMAKE_THREAD_LIBS})
