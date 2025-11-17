include(src/cmake/modules/DeduceDLL.cmake) 

function(DETERMINE_PNG_ZLIB PNG_OUT ZLIB_OUT USE_BUNDLED)
    if(USE_BUNDLED)
        message(STATUS "Using bundled PNG and Zlib")

        add_library(BundledPNG SHARED IMPORTED)
        set_target_properties(BundledPNG PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/src/win/dll/libpng12.dll"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/src/win/lib/libpng.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/win/include"
            INTERFACE_LINK_LIBRARIES BundledZLib
        )

        add_library(BundledZLib SHARED IMPORTED)
        set_target_properties(BundledZLib PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/src/win/dll/zlib1.dll"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/src/win/lib/zlib.lib"
        )

        set(${PNG_OUT}  BundledPNG  PARENT_SCOPE)
        set(${ZLIB_OUT} BundledZLib PARENT_SCOPE)
        return()
    endif()

    find_package(PNG QUIET)
    if(NOT PNG_FOUND)
        message(FATAL_ERROR
            "System PNG not found. If you are building a 32-bit x86 Windows binary, "
            "enable -DSUPPORT_BUNDLED_PNG=ON"
        )
    endif()

    add_library(SystemPNG SHARED IMPORTED)
    list(GET PNG_LIBRARIES 0 PNG_IMPLIB)
    get_filename_component(_name "${PNG_IMPLIB}" NAME)
    string(TOLOWER "${_name}" _name_lower)
    if(NOT _name_lower MATCHES "png")
        message(WARNING
            "First PNG library does not look like a PNG import library: ${PNG_IMPLIB}"
        )
    endif()

    _deduce_dll_path("${PNG_IMPLIB}" PNG_DLL "")

    set_target_properties(SystemPNG PROPERTIES
        IMPORTED_IMPLIB   "${PNG_IMPLIB}"
        IMPORTED_LOCATION "${PNG_DLL}"
        INTERFACE_INCLUDE_DIRECTORIES "${PNG_INCLUDE_DIRS}"
    )

    list(LENGTH PNG_LIBRARIES COUNT)
    if(COUNT GREATER 1)
        list(GET PNG_LIBRARIES 1 ZLIB_IMPLIB)
        get_filename_component(_zname "${ZLIB_IMPLIB}" NAME)
        string(TOLOWER "${_zname}" _zname_lower)
        if(NOT _zname_lower MATCHES "libz" AND NOT _zname_lower MATCHES "zlib")
            message(WARNING
                "Second library in PNG_LIBRARIES does not look like ZLib: ${ZLIB_IMPLIB}"
            )
        endif()

        _deduce_dll_path("${ZLIB_IMPLIB}" ZLIB_DLL "zlib*.dll")

        add_library(SystemZLib SHARED IMPORTED)
        set_target_properties(SystemZLib PROPERTIES
            IMPORTED_IMPLIB "${ZLIB_IMPLIB}"
            IMPORTED_LOCATION "${ZLIB_DLL}"
        )

        # Ensure PNG links to ZLib transitively
        set_target_properties(SystemPNG PROPERTIES
            INTERFACE_LINK_LIBRARIES SystemZLib)
    endif()

    message(STATUS "Using system PNG+Zlib:")
    message(STATUS "  PNG include dirs : ${PNG_INCLUDE_DIRS}")
    message(STATUS "  PNG libraries    : ${PNG_LIBRARIES}")
    message(STATUS "  PNG DLL          : ${PNG_DLL}")
    message(STATUS "  ZLib implib      : ${ZLIB_IMPLIB}")
    message(STATUS "  ZLib DLL         : ${ZLIB_DLL}")

    set(${PNG_OUT}  SystemPNG  PARENT_SCOPE)
    set(${ZLIB_OUT} SystemZLib PARENT_SCOPE)
endfunction()
