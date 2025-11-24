function(DETERMINE_PNG PNG_TARGET PNG_DLLS USE_BUNDLED)
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

    find_package(PNG QUIET)
    if(NOT PNG_FOUND)
        message(FATAL_ERROR
            "System PNG not found. If you are building a 32-bit x86 Windows binary, "
            "enable -DSUPPORT_BUNDLED_PNG=ON"
        )
    endif()

    message(STATUS "Using system PNG and ZLIB:")
    message(STATUS "  PNG include dirs : ${PNG_INCLUDE_DIRS}")
    message(STATUS "  PNG libraries    : ${PNG_LIBRARIES}")

    # Note: at this point we do not know if there are DLLs involved or not.
    # There is no easy way to figure that out, so we return an empty list.
    set(${PNG_TARGET} PNG::PNG PARENT_SCOPE)
    set(${PNG_DLLS}   ""       PARENT_SCOPE)
endfunction()
