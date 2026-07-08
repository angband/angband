# Given the name of an imported library target for linking with curses
# in _TARGET, adds the ANGBAND_PROP_CURSES_HAS_USE_DEFAULT_COLORS property to
# that target to reflect whether the library supports use_default_colors().
#
function(angband_curses_test_use_default_colors _TARGET)
    get_target_property(_HEADER ${_TARGET} ANGBAND_PROP_CURSES_HEADER_NAME)
    if(_HEADER STREQUAL _HEADER-NOTFOUND)
        set_target_properties(${_TARGET} PROPERTIES
            ANGBAND_PROP_CURSES_HAS_USE_DEFAULT_COLORS OFF
        )
        message(WARNING "Target, ${_TARGET}, does not have the ANGBAND_PROP_CURSES_HEADER_NAME property")
        return()
    endif()
    include(CheckSymbolExists)
    include(CMakePushCheckState)
    cmake_push_check_state()
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${_TARGET})
    get_target_property(_PROP ${_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    if(NOT(_PROP STREQUAL "_PROP-NOTFOUND"))
        set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${_PROP})
    endif()
    get_target_property(_PROP ${_TARGET} INTERFACE_COMPILE_DEFINITIONS)
    if(NOT(_PROP STREQUAL "_PROP-NOTFOUND"))
        set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${_PROP})
    endif()
    check_symbol_exists(use_default_colors "${_HEADER}" _HAS_USE_DEFAULT_COLORS)
    cmake_pop_check_state()
    if(_HAS_USE_DEFAULT_COLORS)
        set_target_properties(${_TARGET} PROPERTIES
            ANGBAND_PROP_CURSES_HAS_USE_DEFAULT_COLORS ON
        )
    else()
        set_target_properties(${_TARGET} PROPERTIES
            ANGBAND_PROP_CURSES_HAS_USE_DEFAULT_COLORS OFF
        )
    endif()
endfunction()

# Creates a target as a imported library for linking with curses and sets
# _TARGET_OUT to the name of the target.  Will not modify _TARGET_OUT if curses
# could not be found.
#
# Besides normal properties on an imported library target, will set the
# following properties:
#
# ANGBAND_PROP_CURSES_NCURSES
#     Will be ON if the library is ncurses or OFF if it is not ncurses.
# ANGBAND_PROP_CURSES_HAS_USE_DEFAULT_COLORS
#     Will be ON if the library supports use_default_colors() or OFF if it
#     does not.
# ANGBAND_PROP_CURSES_HEADER_NAME
#     Is the name of the include file, appropriate for substituting into
#     a C include directive, to use for the library.
#
function(angband_curses_create_target _TARGET_OUT)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
            # OpenBSD ships ncursesw by default; only 'ncurses.pc' exists
            pkg_check_modules(CURSES IMPORTED_TARGET ncurses)
        else()
            pkg_check_modules(CURSES IMPORTED_TARGET ncursesw)
        endif()

        if(CURSES_FOUND)
            include(PkgConfigHelpers)
            angband_pkgconfig_select_target(CURSES _TARGET)
            set_target_properties(${_TARGET} PROPERTIES
                ANGBAND_PROP_CURSES_NCURSES ON
                ANGBAND_PROP_CURSES_HEADER_NAME "ncurses.h"
            )
            angband_curses_test_use_default_colors(${_TARGET})
            set(${_TARGET_OUT} ${_TARGET} PARENT_SCOPE)
            return()
        endif()
    endif()

    set(_TARGET Curses::Curses)
    # Need 3.10 or later required for CURSES_NEED_WIDE
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.10)
        set(CURSES_NEED_WIDE TRUE)
        set(CURSES_NEED_NCURSES TRUE)
        find_package(Curses)
        if(Curses_FOUND)
            if(NOT TARGET ${_TARGET})
                add_library(${_TARGET} INTERFACE IMPORTED)
                set_target_properties(${_TARGET} PROPERTIES
                    INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
                    INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
                    ANGBAND_PROP_CURSES_NCURSES ON
                    ANGBAND_PROP_CURSES_HEADER_NAME "ncurses.h"
                )
                angband_curses_test_use_default_colors(${_TARGET})
            endif()
            set(${_TARGET_OUT} ${_TARGET} PARENT_SCOPE)
            return()
        endif()
    endif()

    # Need 3.14 or later for CMAKE_REQUIRED_LINK_OPTIONS.
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.14)
        include(CheckSymbolExists)
        include(CMakePushCheckState)

        if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 4)
            set(_LIBRARIES "LINKER:-l,ncurses")
        else()
            set(_LIBRARIES "-lncurses")
        endif()
        cmake_push_check_state()
        set(CMAKE_REQUIRED_LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS} "-lncurses")
        check_symbol_exists("mvaddnwstr" "ncurses.h" HAVE_MVADDNWSTR_NCURSES)
        cmake_pop_check_state()
        if(HAVE_MVADDNWSTR_NCURSES)
            if(NOT TARGET ${_TARGET})
                add_library(${_TARGET} INTERFACE IMPORTED)
                set_target_properties(${_TARGET} PROPERTIES
                    INTERFACE_LINK_LIBRARIES ${_LIBRARIES}
                    ANGBAND_PROP_CURSES_NCURSES ON
                    ANGBAND_PROP_CURSES_HEADER_NAME "ncurses.h"
                )
                angband_curses_test_use_default_colors(${_TARGET})
            endif()
            set(${_TARGET_OUT} ${_TARGET} PARENT_SCOPE)
            return()
        endif()

        # Try again but with _XOPEN_SOURCE_EXTENDED set (needed on some
        # platforms, macOS is one, to see mvwaddnwstr).
        cmake_push_check_state()
        set(CMAKE_REQUIRED_LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS} "-lncurses")
        set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} "-D_XOPEN_SOURCE_EXTENDED")
        check_symbol_exists("mvaddnwstr" "ncurses.h" HAVE_MVADDNWSTR_NCURSES_XOPEN)
        cmake_pop_check_state()
        if(HAVE_MVADDNWSTR_NCURSES_XOPEN)
            if(NOT TARGET ${_TARGET})
                add_library(${_TARGET} INTERFACE IMPORTED)
                set_target_properties(${_TARGET} PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS -D_XOPEN_SOURCE_EXTENDED
                    INTERFACE_LINK_LIBRARIES ${_LIBRARIES}
                    ANGBAND_PROP_CURSES_NCURSES ON
                    ANGBAND_PROP_CURSES_HEADER_NAME "ncurses.h"
                )
                angband_curses_test_use_default_colors(${_TARGET})
            endif()
            set(${_TARGET_OUT} ${_TARGET} PARENT_SCOPE)
            return()
        endif()

        if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 4)
            set(_LIBRARIES "LINKER:-l,curses")
        else()
            set(_LIBRARIES "-lcurses")
        endif()
        cmake_push_check_state()
        set(CMAKE_REQUIRED_LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS} "-lcurses")
        check_symbol_exists("mvaddnwstr" "curses.h" HAVE_MVADDNSTR_CURSES)
        cmake_pop_check_state()
        if(HAVE_MVADDNWSTR_CURSES)
            if(NOT TARGET ${_TARGET})
                add_library(${_TARGET} INTERFACE IMPORTED)
                set_target_properties(${_TARGET} PROPERTIES
                    INTERFACE_LINK_LIBRARIES ${_LIBRARIES}
                    ANGBAND_PROP_CURSES_NCURSES OFF
                    ANGBAND_PROP_CURSES_HEADER_NAME "curses.h"
                )
                angband_curses_test_use_default_colors(${_TARGET})
            endif()
            return()
            set(${_TARGET_OUT} ${_TARGET} PARENT_SCOPE)
        endif()
    endif()
endfunction()

macro(configure_gcu_frontend _NAME_TARGET)
    angband_curses_create_target(CURSES_SELECTED)
    if(TARGET ${CURSES_SELECTED})
        target_link_libraries(${_NAME_TARGET} PRIVATE ${CURSES_SELECTED})
        target_compile_definitions(${_NAME_TARGET} PRIVATE
            USE_GCU
            $<$<BOOL:$<TARGET_PROPERTY:${CURSES_SELECTED},ANGBAND_PROP_CURSES_NCURSES>>:USE_NCURSES>
            $<$<BOOL:$<TARGET_PROPERTY:${CURSES_SELECTED},ANGBAND_PROP_CURSES_HAS_USE_DEFAULT_COLORS>>:HAVE_USE_DEFAULT_COLORS>
            $<$<BOOL:${WIN32}>:WIN32_CONSOLE_MODE>
            $<$<BOOL:${MINGW}>:MSYS2_ENCODING_WORKAROUND>
        )

        message(STATUS "Support for GCU front end - Ready")
    else()
        message(FATAL_ERROR "Support for GCU front end - curses not found")
    endif()
endmacro()
