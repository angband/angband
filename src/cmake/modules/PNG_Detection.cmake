function(determine_png PNG_TARGET PNG_DLLS USE_BUNDLED)
    if(USE_BUNDLED)
        message(STATUS "Using bundled PNG and ZLIB")

        add_library(BundledZLib SHARED IMPORTED)
        set_target_properties(BundledZLib PROPERTIES
            IMPORTED_LOCATION   "${CMAKE_CURRENT_SOURCE_DIR}/src/win/dll/zlib1.dll"
            IMPORTED_IMPLIB     "${CMAKE_CURRENT_SOURCE_DIR}/src/win/lib/zlib.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/win/include"
        )

        add_library(BundledPNG SHARED IMPORTED)
        set_target_properties(BundledPNG PROPERTIES
            IMPORTED_LOCATION   "${CMAKE_CURRENT_SOURCE_DIR}/src/win/dll/libpng12.dll"
            IMPORTED_IMPLIB     "${CMAKE_CURRENT_SOURCE_DIR}/src/win/lib/libpng.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/win/include"
            INTERFACE_LINK_LIBRARIES BundledZLib
        )

        set(${PNG_TARGET} BundledPNG PARENT_SCOPE)
        set(${PNG_DLLS}
            "${CMAKE_CURRENT_SOURCE_DIR}/src/win/dll/libpng12.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/win/dll/zlib1.dll"
            PARENT_SCOPE
        )
        return()
    endif()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PNG QUIET IMPORTED_TARGET libpng)
    if(NOT PNG_FOUND)
        message(FATAL_ERROR
            "System PNG not found. If you are building a 32-bit x86 Windows binary, "
            "enable -DSUPPORT_BUNDLED_PNG=ON"
        )
    endif()

    include(PkgConfigHelpers)
    angband_pkgconfig_select_target(PNG PNG_SELECTED)

    if(SUPPORT_STATIC_LINKING)
        message(STATUS "Using system PNG and ZLIB (static)")
        message(STATUS "  PNG static libraries    : ${PNG_STATIC_LIBRARIES}")
        message(STATUS "  PNG static include dirs : ${PNG_STATIC_INCLUDE_DIRS}")
    else()
        message(STATUS "Using system PNG and ZLIB (dynamic)")
        message(STATUS "  PNG include dirs        : ${PNG_INCLUDE_DIRS}")
        message(STATUS "  PNG libraries           : ${PNG_LIBRARIES}")
    endif()

    # For system libraries, we do not attempt to guess runtime DLLs
    set(${PNG_TARGET} ${PNG_SELECTED} PARENT_SCOPE)
    set(${PNG_DLLS}   ""              PARENT_SCOPE)
endfunction()
