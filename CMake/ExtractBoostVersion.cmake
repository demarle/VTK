SET(BOOST_VERSION 0)
SET(BOOST_MAJOR_VERSION 0)
SET(BOOST_MINOR_VERSION 0)
SET(BOOST_SUBMINOR_VERSION 0)
SET(BOOST_LIB_VERSION "")

IF(Boost_INCLUDE_DIR AND EXISTS "${Boost_INCLUDE_DIR}/boost/version.hpp")
  # Read the whole file:
  #
  FILE(READ "${Boost_INCLUDE_DIR}/boost/version.hpp" BOOST_VERSION_HPP_CONTENTS)

  # Split it into lines:
  #
  STRING(REGEX REPLACE ";" "\\\\;" BOOST_VERSION_HPP_CONTENTS "${BOOST_VERSION_HPP_CONTENTS}")
  STRING(REGEX REPLACE "\n" "EOL;" BOOST_VERSION_HPP_CONTENTS "${BOOST_VERSION_HPP_CONTENTS}")

  FOREACH(line ${BOOST_VERSION_HPP_CONTENTS})
    # Save BOOST_VERSION:
    #
    IF("${line}" MATCHES "^#define BOOST_VERSION ([0-9]+).*EOL$")
      STRING(REGEX REPLACE
        "^#define BOOST_VERSION ([0-9]+).*EOL$" "\\1"
        BOOST_VERSION "${line}")
    ENDIF("${line}" MATCHES "^#define BOOST_VERSION ([0-9]+).*EOL$")

    # And BOOST_LIB_VERSION:
    #
    IF("${line}" MATCHES "^#define BOOST_LIB_VERSION \"([0-9_]+)\".*EOL$")
      STRING(REGEX REPLACE
        "^#define BOOST_LIB_VERSION \"([0-9_]+)\".*EOL$" "\\1"
        BOOST_LIB_VERSION "${line}")
    ENDIF("${line}" MATCHES "^#define BOOST_LIB_VERSION \"([0-9_]+)\".*EOL$")
  ENDFOREACH(line)

  # Compute major, minor and subminor versions according to comments in
  # boost/version.hpp:
  #
  IF(NOT "${BOOST_VERSION}" STREQUAL "0")
    MATH(EXPR BOOST_MAJOR_VERSION "${BOOST_VERSION} / 100000")
    MATH(EXPR BOOST_MINOR_VERSION "${BOOST_VERSION} / 100 % 1000")
    MATH(EXPR BOOST_SUBMINOR_VERSION "${BOOST_VERSION} % 100")
  ENDIF(NOT "${BOOST_VERSION}" STREQUAL "0")
ENDIF(Boost_INCLUDE_DIR AND EXISTS "${Boost_INCLUDE_DIR}/boost/version.hpp")

#MESSAGE("BOOST_VERSION='${BOOST_VERSION}'")
#MESSAGE("BOOST_MAJOR_VERSION='${BOOST_MAJOR_VERSION}'")
#MESSAGE("BOOST_MINOR_VERSION='${BOOST_MINOR_VERSION}'")
#MESSAGE("BOOST_SUBMINOR_VERSION='${BOOST_SUBMINOR_VERSION}'")
#MESSAGE("BOOST_LIB_VERSION='${BOOST_LIB_VERSION}'")
