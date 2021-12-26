MACRO(CONFIGURE_GID_HANDLING _NAME_TARGET)
    # Test for and then set the appropriate preprocessor macros expected by
    # z-file.c to describe how the system handles changing the group ID.
    INCLUDE(CheckSymbolExists)

    # Linux man page only mentions unistd.h; FreeBSD's also has sys/types.h.
    # The Linux man page mentions _GNU_SOURCE as necessary.
    SET(OLD_CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
    SET(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -D_GNU_SOURCE=)
    CHECK_SYMBOL_EXISTS(setresgid "sys/types.h;unistd.h" SETRESGID_EXISTS)
    SET(CMAKE_REQUIRED_DEFINITIONS ${OLD_CMAKE_REQUIRED_DEFINITIONS})
    IF(SETRESGID_EXISTS)
        TARGET_COMPILE_DEFINITIONS(${_NAME_TARGET} PRIVATE -D _GNU_SOURCE)
        TARGET_COMPILE_DEFINITIONS(${_NAME_TARGET} PRIVATE -D HAVE_SETRESGID)
    ENDIF()

    CHECK_SYMBOL_EXISTS(setegid "unistd.h" SETEGID_EXISTS)
    IF(SETEGID_EXISTS)
        TARGET_COMPILE_DEFINITIONS(${_NAME_TARGET} PRIVATE -D HAVE_SETEGID)
    ENDIF()

    IF((SETRESGID_EXISTS) OR (SETEGID_EXISTS))
        MESSAGE(STATUS "GID handling configured")
    ELSE()
        MESSAGE(FATAL_ERROR "Neither setresgid() nor setegid() appears to be available; at least one is needed for a shared setgid installation")
    ENDIF()
ENDMACRO()
