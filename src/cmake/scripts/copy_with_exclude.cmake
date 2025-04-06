# Usage: cmake -P copy_with_exclude.cmake -- <out> <in> [<exclude1> ...]
# Copy directory, <in>, to the directory <out> excluding any files that match
# one of the optionally specified patterns, <exclude1> and so on.

CMAKE_POLICY(VERSION 3.5...3.31)
SET(_USAGE "usage: cmake [[-D <var>=<value>] ...] -P copy_with_exclude.cmake -- out in [exclude1 ...]")
SET(_X 0)
WHILE(_X LESS CMAKE_ARGC)
    IF(CMAKE_ARGV${_X} STREQUAL "--")
        BREAK()
    ENDIF()
    MATH(EXPR _X "${_X} + 1")
ENDWHILE()
MATH(EXPR _X "${_X} + 2")
IF(_X GREATER_EQUAL ${CMAKE_ARGC})
    MESSAGE(FATAL_ERROR "${_USAGE}")
ENDIF()

MATH(EXPR _Y "${_X} - 1")
SET(_OUT "${CMAKE_ARGV${_Y}}")
SET(_IN "${CMAKE_ARGV${_X}}")
UNSET(_PATTERNS)
MATH(EXPR _X "${_X} + 1")
WHILE(_X LESS CMAKE_ARGC)
    LIST(APPEND _PATTERNS "PATTERN" "${CMAKE_ARGV${_X}}" "EXCLUDE")
    MATH(EXPR _X "${_X} + 1")
ENDWHILE()

FILE(COPY "${_IN}" DESTINATION "${_OUT}" ${_PATTERNS})
