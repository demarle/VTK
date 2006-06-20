# Nothing here yet
IF(CMAKE_GENERATOR MATCHES "Visual Studio 7")
  INCLUDE(CMakeVS7BackwardCompatibility)
  SET(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio 7")
IF(CMAKE_GENERATOR MATCHES "Visual Studio 6")
  INCLUDE(CMakeVS6BackwardCompatibility)
  SET(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio 6")

IF(NOT CMAKE_SKIP_COMPATIBILITY_TESTS)
  INCLUDE (CheckTypeSize)
  CHECK_TYPE_SIZE(int      CMAKE_SIZEOF_INT)
  CHECK_TYPE_SIZE(long     CMAKE_SIZEOF_LONG)
  CHECK_TYPE_SIZE("void*"  CMAKE_SIZEOF_VOID_P)
  CHECK_TYPE_SIZE(char     CMAKE_SIZEOF_CHAR)
  CHECK_TYPE_SIZE(short    CMAKE_SIZEOF_SHORT)
  CHECK_TYPE_SIZE(float    CMAKE_SIZEOF_FLOAT)
  CHECK_TYPE_SIZE(double   CMAKE_SIZEOF_DOUBLE)

  INCLUDE (CheckIncludeFile)
  CHECK_INCLUDE_FILE("limits.h"       CMAKE_HAVE_LIMITS_H)
  CHECK_INCLUDE_FILE("unistd.h"       CMAKE_HAVE_UNISTD_H)
  CHECK_INCLUDE_FILE("pthread.h"      CMAKE_HAVE_PTHREAD_H)

  INCLUDE (CheckIncludeFiles)
  CHECK_INCLUDE_FILES("sys/types.h;sys/prctl.h"    CMAKE_HAVE_SYS_PRCTL_H)

  INCLUDE (TestBigEndian)
  TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)
  IF(NOT APPLE)
    INCLUDE (FindX11)
  ENDIF(NOT APPLE)

  IF("${X11_X11_INCLUDE_PATH}" MATCHES "^/usr/include$")
    SET (CMAKE_X_CFLAGS "" CACHE STRING "X11 extra flags.")
  ELSE("${X11_X11_INCLUDE_PATH}" MATCHES "^/usr/include$")
    SET (CMAKE_X_CFLAGS "-I${X11_X11_INCLUDE_PATH}" CACHE STRING
         "X11 extra flags.")
  ENDIF("${X11_X11_INCLUDE_PATH}" MATCHES "^/usr/include$")
  SET (CMAKE_X_LIBS "${X11_LIBRARIES}" CACHE STRING
       "Libraries and options used in X11 programs.")
  SET (CMAKE_HAS_X "${X11_FOUND}" CACHE INTERNAL "Is X11 around.")

  INCLUDE (FindThreads)

  SET (CMAKE_THREAD_LIBS        "${CMAKE_THREAD_LIBS_INIT}" CACHE STRING 
    "Thread library used.")

  SET (CMAKE_USE_PTHREADS       "${CMAKE_USE_PTHREADS_INIT}" CACHE BOOL
     "Use the pthreads library.")

  SET (CMAKE_USE_WIN32_THREADS  "${CMAKE_USE_WIN32_THREADS_INIT}" CACHE BOOL
       "Use the win32 thread library.")

  SET (CMAKE_HP_PTHREADS        ${CMAKE_HP_PTHREADS_INIT} CACHE BOOL
     "Use HP pthreads.")

  SET (CMAKE_USE_SPROC          ${CMAKE_USE_SPROC_INIT} CACHE BOOL 
     "Use sproc libs.")
ENDIF(NOT CMAKE_SKIP_COMPATIBILITY_TESTS)

MARK_AS_ADVANCED(
CMAKE_HP_PTHREADS
CMAKE_THREAD_LIBS
CMAKE_USE_PTHREADS
CMAKE_USE_SPROC
CMAKE_USE_WIN32_THREADS
CMAKE_X_CFLAGS
CMAKE_X_LIBS
)

