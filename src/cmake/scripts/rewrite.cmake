# Usage: cmake -P rewrite.cmake -- <out> <int> <target1> <replace1> ...
# Take the file <in> as a template.  Replace occurents of @<target1>@ with
# <replace1>, @<target2>@ with <replace2>, and so on to generate the output
# file <out>.  The target names _IN, _OUT, _X, _Y, and CMAKE_ARGC, and
# CMAKE_ARGV<anything> are reserved for use by the rewrite.cmake and will
# generate a fatal error if used.

cmake_policy(VERSION 3.5...3.31)
set(_USAGE "usage: cmake [[-D <var>=<value>] ...] -P rewrite.cmake -- out in [target replacement] ...")
set(_X 0)
while(_X LESS CMAKE_ARGC)
    if(CMAKE_ARGV${_X} STREQUAL "--")
        break()
    endif()
    math(EXPR _X "${_X} + 1")
endwhile()
math(EXPR _X "${_X} + 1")
if(_X GREATER_EQUAL ${CMAKE_ARGC})
    message(FATAL_ERROR "${_USAGE}")
endif()
math(EXPR _Y "(${CMAKE_ARGC} - ${_X}) % 2")
if(NOT (_Y EQUAL 0))
    message(FATAL_ERROR "${_USAGE}")
endif()

set(_OUT "${CMAKE_ARGV${_X}}")
math(EXPR _X "${_X} + 1")
set(_IN "${CMAKE_ARGV${_X}}")
math(EXPR _X "${_X} + 1")
while(_X LESS CMAKE_ARGC)
    if((CMAKE_ARGV${_X} STREQUAL "_IN")
            OR (CMAKE_ARGV${_X} STREQUAL "_OUT")
            OR (CMAKE_ARGV${_X} STREQUAL "_X")
            OR (CMAKE_ARGV${_X} STREQUAL "_Y")
            OR (CMAKE_ARGV${_X} STREQUAL "CMAKE_ARGC")
            OR (CMAKE_ARGV${_X} MATCHES "^CMAKE_ARGV"))
        message(FATAL_ERROR "target name, ${CMAKE_ARGV${_X}}, is used internally by rewrite.cmake")
    endif()
    math(EXPR _Y "${_X} + 1")
    set("${CMAKE_ARGV${_X}}" "${CMAKE_ARGV${_Y}}")
    math(EXPR _X "${_X} + 2")
endwhile()

configure_file("${_IN}" "${_OUT}" @ONLY)
