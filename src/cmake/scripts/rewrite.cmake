# Usage: cmake -P rewrite.cmake -- <out> <int> <target1> <replace1> ...
# Take the file <in> as a template.  Replace occurents of @<target1>@ with
# <replace1>, @<target2>@ with <replace2>, and so on to generate the output
# file <out>.  The target names _IN, _OUT, _X, _Y, and CMAKE_ARGC, and
# CMAKE_ARGV<anything> are reserved for use by the rewrite.cmake and will
# generate a fatal error if used.

CMAKE_POLICY(VERSION 3.5...3.31)
SET(_USAGE "usage: cmake [[-D <var>=<value>] ...] -P rewrite.cmake -- out in [target replacement] ...")
SET(_X 0)
WHILE(_X LESS CMAKE_ARGC)
    IF(CMAKE_ARGV${_X} STREQUAL "--")
        BREAK()
    ENDIF()
    MATH(EXPR _X "${_X} + 1")
ENDWHILE()
MATH(EXPR _X "${_X} + 1")
IF(_X GREATER_EQUAL ${CMAKE_ARGC})
    MESSAGE(FATAL_ERROR "${_USAGE}")
ENDIF()
MATH(EXPR _Y "(${CMAKE_ARGC} - ${_X}) % 2")
IF (NOT (_Y EQUAL 0))
    MESSAGE(FATAL_ERROR "${_USAGE}")
ENDIF()

SET(_OUT "${CMAKE_ARGV${_X}}")
MATH(EXPR _X "${_X} + 1")
SET(_IN "${CMAKE_ARGV${_X}}")
MATH(EXPR _X "${_X} + 1")
WHILE(_X LESS CMAKE_ARGC)
    IF ((CMAKE_ARGV${_X} STREQUAL "_IN")
            OR (CMAKE_ARGV${_X} STREQUAL "_OUT")
            OR (CMAKE_ARGV${_X} STREQUAL "_X")
            OR (CMAKE_ARGV${_X} STREQUAL "_Y")
            OR (CMAKE_ARGV${_X} STREQUAL "CMAKE_ARGC")
            OR (CMAKE_ARGV${_X} MATCHES "^CMAKE_ARGV"))
        MESSAGE(FATAL_ERROR "target name, ${CMAKE_ARGV${_X}}, is used internally by rewrite.cmake")
    ENDIF()
    MATH(EXPR _Y "${_X} + 1")
    SET("${CMAKE_ARGV${_X}}" "${CMAKE_ARGV${_Y}}")
    MATH(EXPR _X "${_X} + 2")
ENDWHILE()

CONFIGURE_FILE("${_IN}" "${_OUT}" @ONLY)
