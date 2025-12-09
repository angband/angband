macro(configure_gcu_frontend _NAME_TARGET)
    find_package(PkgConfig REQUIRED)

    if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
        # OpenBSD ships ncursesw by default; only 'ncurses.pc' exists
        pkg_check_modules(CURSES REQUIRED IMPORTED_TARGET ncurses)
    else()
        pkg_check_modules(CURSES REQUIRED IMPORTED_TARGET ncursesw)
    endif()

    if(SUPPORT_STATIC_LINKING)
        if(NOT TARGET PkgConfig::CURSES_STATIC)
            add_library(PkgConfig::CURSES_STATIC INTERFACE IMPORTED)
            set_target_properties(PkgConfig::CURSES_STATIC PROPERTIES
                INTERFACE_LINK_LIBRARIES      "${CURSES_STATIC_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${CURSES_STATIC_INCLUDE_DIRS}"
                INTERFACE_COMPILE_OPTIONS     "${CURSES_STATIC_CFLAGS_OTHER}"
                INTERFACE_LINK_OPTIONS        "${CURSES_STATIC_LDFLAGS_OTHER}"
            )
        endif()
        target_link_options(${_NAME_TARGET} PRIVATE -static)
        target_link_libraries(${_NAME_TARGET} PRIVATE PkgConfig::CURSES_STATIC)
    else()
        target_link_libraries(${_NAME_TARGET} PRIVATE PkgConfig::CURSES)
    endif()

    target_compile_definitions(${_NAME_TARGET} PRIVATE USE_GCU USE_NCURSES)

    if(WIN32)
        target_compile_definitions(${_NAME_TARGET} PRIVATE WIN32_CONSOLE_MODE)
    endif()

    if(MINGW)
        target_compile_definitions(${_NAME_TARGET} PRIVATE MSYS2_ENCODING_WORKAROUND )
    endif()

    include(CheckSymbolExists)
    set(CMAKE_REQUIRED_LIBRARIES PkgConfig::CURSES)
    set(CMAKE_REQUIRED_INCLUDES ${CURSES_INCLUDE_DIRS})
    check_symbol_exists(use_default_colors "curses.h" ANGBAND_NCURSESW_HAS_USE_DEFAULT_COLORS)
    unset(CMAKE_REQUIRED_LIBRARIES)
    unset(CMAKE_REQUIRED_INCLUDES)
    if(ANGBAND_NCURSESW_HAS_USE_DEFAULT_COLORS)
        target_compile_definitions(${_NAME_TARGET} PRIVATE HAVE_USE_DEFAULT_COLORS)
    endif()

    message(STATUS "Support for GCU front end - Ready")
endmacro()
