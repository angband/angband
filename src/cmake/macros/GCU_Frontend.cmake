macro(configure_gcu_frontend _NAME_TARGET)
    find_package(PkgConfig REQUIRED)

    if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
        # OpenBSD ships ncursesw by default; only 'ncurses.pc' exists
        pkg_check_modules(CURSES REQUIRED IMPORTED_TARGET ncurses)
    else()
        pkg_check_modules(CURSES REQUIRED IMPORTED_TARGET ncursesw)
    endif()

    include(PkgConfigHelpers)
    angband_pkgconfig_select_target(CURSES CURSES_SELECTED)
    target_link_libraries(${_NAME_TARGET} PRIVATE ${CURSES_SELECTED})

    target_compile_definitions(${_NAME_TARGET} PRIVATE
        USE_GCU
        USE_NCURSES
        $<$<BOOL:${WIN32}>:WIN32_CONSOLE_MODE>
        $<$<BOOL:${MINGW}>:MSYS2_ENCODING_WORKAROUND>
    )

    # Check if use_default_colors() exists
    include(CheckSymbolExists)
    set(CMAKE_REQUIRED_LIBRARIES ${CURSES_SELECTED})
    set(CMAKE_REQUIRED_INCLUDES ${CURSES_INCLUDE_DIRS})
    check_symbol_exists(use_default_colors "curses.h" ANGBAND_NCURSESW_HAS_USE_DEFAULT_COLORS)
    unset(CMAKE_REQUIRED_LIBRARIES)
    unset(CMAKE_REQUIRED_INCLUDES)
    if(ANGBAND_NCURSESW_HAS_USE_DEFAULT_COLORS)
        target_compile_definitions(${_NAME_TARGET} PRIVATE HAVE_USE_DEFAULT_COLORS)
    endif()

    message(STATUS "Support for GCU front end - Ready")
endmacro()
