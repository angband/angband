function(_deduce_dll_path lib_path dll_out)
    # This function tries to deduce a DLL name from an import library (.dll.a or .lib)
    # Works reliably for MSYS2/MinGW, also falls back to naive heuristic for others

    if(NOT EXISTS "${lib_path}")
        set(${dll_out} "" PARENT_SCOPE)
        return()
    endif()

    get_filename_component(_lib_dir "${lib_path}" DIRECTORY)
    get_filename_component(_lib_name "${lib_path}" NAME_WE)

    message(STATUS "---- DLL deduction start ----")
    message(STATUS "Import lib path input: '${lib_path}'")
    message(STATUS " Directory: ${_lib_dir}")
    message(STATUS " Base name: ${_lib_name}")

    set(_dll_candidate "")

    # --- Detect MSYS2/MinGW automatically ---
    if(EXISTS "${_lib_dir}/../bin")  # heuristic: MSYS2 places DLLs in /mingw32/bin
        set(_bin_dir "${_lib_dir}/../bin")
        message(STATUS "[MSYS2] Searching in: ${_bin_dir}")
        file(GLOB _dlls "${_bin_dir}/*${_lib_name}*.dll")
        if(_dlls)
            list(GET _dlls 0 _dll_candidate)
            message(STATUS "[MSYS2] Chosen DLL: ${_dll_candidate}")
        endif()
    endif()

    # --- fallback: naive heuristics ---
    if(NOT _dll_candidate)
        set(_candidates
            "${_lib_dir}/${_lib_name}.dll"
            "${_lib_dir}/../bin/${_lib_name}.dll"
            "${_lib_dir}/../lib/${_lib_name}.dll"
            "${_lib_dir}/../lib/${_lib_name}1.dll"
            "${_lib_dir}/../bin/${_lib_name}1.dll"
        )
        foreach(_cand IN LISTS _candidates)
            if(EXISTS "${_cand}")
                set(_dll_candidate "${_cand}")
                break()
            endif()
        endforeach()
    endif()

    if(_dll_candidate)
        message(STATUS "DLL deduced: ${_dll_candidate}")
        set(${dll_out} "${_dll_candidate}" PARENT_SCOPE)
    else()
        message(WARNING "No DLL was found for '${lib_path}'")
        set(${dll_out} "" PARENT_SCOPE)
    endif()
    message(STATUS "---- DLL deduction end ----")
endfunction()

MACRO(CONFIGURE_WINDOWS_FRONTEND _NAME_TARGET _ONLY_DEFINES)
    # --- PNG detection ---
    find_package(PNG QUIET)
    if(PNG_FOUND)
        if (NOT TARGET SystemPNG)
            message(STATUS "Using system PNG:")
            message(STATUS "  Version:      ${PNG_VERSION_STRING}")
            message(STATUS "  Include dirs: ${PNG_INCLUDE_DIRS}")
            message(STATUS "  Libraries:    ${PNG_LIBRARIES}")

            # Try to extract the first library path for PNG
            list(GET PNG_LIBRARIES 0 PNG_IMPLIB_PATH)
            message(STATUS "  Implib:       ${PNG_IMPLIB_PATH}")

            _deduce_dll_path("${PNG_IMPLIB_PATH}" PNG_DLL_PATH)
            message(STATUS "  DLL:          ${PNG_DLL_PATH}")

            # Create imported targets
            add_library(SystemPNG SHARED IMPORTED)
            set_target_properties(SystemPNG PROPERTIES
                IMPORTED_IMPLIB "${PNG_IMPLIB_PATH}"
                IMPORTED_LOCATION "${PNG_DLL_PATH}"
                INTERFACE_INCLUDE_DIRECTORIES "${PNG_INCLUDE_DIRS}"
            )

            # Detect ZLIB if present
            if(ZLIB_FOUND)
                message(STATUS "Using system ZLIB: ${ZLIB_LIBRARY}")
                message(STATUS "  Version:      ${ZLIB_VERSION_STRING}")
                message(STATUS "  Include dirs: ${ZLIB_INCLUDE_DIRS}")
                message(STATUS "  Libraries:    ${ZLIB_LIBRARIES}")
                list(GET ZLIB_LIBRARIES 0 ZLIB_IMPLIB_PATH)
                message(STATUS "  Implib:       ${ZLIB_IMPLIB_PATH}")
                _deduce_dll_path("${ZLIB_IMPLIB_PATH}" ZLIB_DLL_PATH)
                message(STATUS "  DLL:          ${ZLIB_DLL_PATH}")

                add_library(SystemZLib SHARED IMPORTED)
                set_target_properties(SystemZLib PROPERTIES
                    IMPORTED_IMPLIB "${ZLIB_IMPLIB_PATH}"
                    IMPORTED_LOCATION "${ZLIB_DLL_PATH}"
                    INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
                )
                set_target_properties(SystemPNG PROPERTIES INTERFACE_LINK_LIBRARIES SystemZLib)

                set(ZLIB_TO_LINK SystemZLib)
            endif()

            set(PNG_TO_LINK SystemPNG)
        endif()

    else()
        # --- Bundled PNG + ZLIB ---
        if(NOT TARGET OurWindowsPNGLib)
            message(STATUS "Using bundled PNG + ZLIB")

            add_library(OurWindowsPNGLib SHARED IMPORTED)
            set_target_properties(OurWindowsPNGLib PROPERTIES
                IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/src/win/dll/libpng12.dll"
                IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/src/win/lib/libpng.lib"
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/win/include"
                INTERFACE_LINK_LIBRARIES OurWindowsZLib
            )

            add_library(OurWindowsZLib SHARED IMPORTED)
            set_target_properties(OurWindowsZLib PROPERTIES
                IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/src/win/dll/zlib1.dll"
                IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/src/win/lib/zlib.lib"
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/win/include"
            )

            set(PNG_TO_LINK OurWindowsPNGLib)
            set(ZLIB_TO_LINK OurWindowsZLib)
        endif()
    endif()

    # --- Link libraries if not only defines ---
    if(NOT _ONLY_DEFINES)
        target_link_libraries(${_NAME_TARGET}
            PRIVATE ${PNG_TO_LINK} ${ZLIB_TO_LINK} msimg32 winmm
        )
        set_target_properties(${_NAME_TARGET} PROPERTIES WIN32_EXECUTABLE ON)
    endif()

    # --- Compile definitions ---
    target_compile_definitions(${_NAME_TARGET}
        PRIVATE 
            USE_WIN
            USE_PRIVATE_PATHS
            SOUND
            WINDOWS
            _CRT_SECURE_NO_WARNINGS
    )

    IF(NOT CONFIGURE_WINDOWS_FRONTEND_INVOKED_PREVIOUSLY)
        MESSAGE(STATUS "Support for Windows front end - Ready")
        SET(CONFIGURE_WINDOWS_FRONTEND_INVOKED_PREVIOUSLY YES CACHE
            INTERNAL "Mark if CONFIGURE_WINDOWS_FRONTEND called successfully" FORCE)
    ENDIF()
ENDMACRO()
