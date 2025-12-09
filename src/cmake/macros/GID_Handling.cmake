macro(configure_gid_handling _NAME_TARGET)
    # Test for and then set the appropriate preprocessor macros expected by
    # z-file.c to describe how the system handles changing the group ID.
    include(CheckSymbolExists)

    # Linux man page only mentions unistd.h; FreeBSD's also has sys/types.h.
    # The Linux man page mentions _GNU_SOURCE as necessary.
    set(OLD_CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
    set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -D_GNU_SOURCE=)
    check_symbol_exists(setresgid "sys/types.h;unistd.h" SETRESGID_EXISTS)
    set(CMAKE_REQUIRED_DEFINITIONS ${OLD_CMAKE_REQUIRED_DEFINITIONS})
    if(SETRESGID_EXISTS)
        TARGET_COMPILE_DEFINITIONS(${_NAME_TARGET} PRIVATE -D _GNU_SOURCE)
        TARGET_COMPILE_DEFINITIONS(${_NAME_TARGET} PRIVATE -D HAVE_SETRESGID)
    endif()

    check_symbol_exists(setegid "unistd.h" SETEGID_EXISTS)
    if(SETEGID_EXISTS)
        target_compile_definitions(${_NAME_TARGET} PRIVATE -D HAVE_SETEGID)
    endif()

    if((SETRESGID_EXISTS) OR (SETEGID_EXISTS))
        message(STATUS "GID handling configured")
    else()
        message(FATAL_ERROR "Neither setresgid() nor setegid() appears to be available; at least one is needed for a shared setgid installation")
    endif()
endmacro()
