# Usage: cmake -P copy_with_exclude.cmake -- <out> <in> [<exclude1> ...]
# Copy directory, <in>, to the directory <out> excluding any files that match
# one of the optionally specified patterns, <exclude1> and so on.

cmake_policy(VERSION 3.5...3.31)
set(_USAGE "usage: cmake [[-D <var>=<value>] ...] -P copy_with_exclude.cmake -- out in [exclude1 ...]")
set(_X 0)
while(_X LESS CMAKE_ARGC)
    if(CMAKE_ARGV${_X} STREQUAL "--")
        BREAK()
    endif()
    math(EXPR _X "${_X} + 1")
endwhile()
math(EXPR _X "${_X} + 2")
if(_X GREATER_EQUAL ${CMAKE_ARGC})
    message(FATAL_ERROR "${_USAGE}")
endif()

math(EXPR _Y "${_X} - 1")
set(_OUT "${CMAKE_ARGV${_Y}}")
set(_IN "${CMAKE_ARGV${_X}}")
unset(_PATTERNS)
math(EXPR _X "${_X} + 1")
while(_X LESS CMAKE_ARGC)
    list(APPEND _PATTERNS "PATTERN" "${CMAKE_ARGV${_X}}" "EXCLUDE")
    math(EXPR _X "${_X} + 1")
endwhile()

file(COPY "${_IN}" DESTINATION "${_OUT}" ${_PATTERNS})
