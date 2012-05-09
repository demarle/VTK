# Try to find Cilk
# Once done, this will define
#
#  CILK_FOUND - system has (x)Kaapi

set(CILK_HOME $ENV{CILK_HOME} CACHE PATH "Path to the Cilk install dir")

find_program(CILK_COMPILER
  NAMES cilk++
  DOC "Find the cilk compiler"
  HINTS ${CILK_HOME} /opt/cilk++/bin
)

set(CILK_FOUND TRUE)

IF("${CILK_COMPILER}" STREQUAL "")
  set(CILK_FOUND FALSE)
ENDIF("${CILK_COMPILER}" STREQUAL "")
