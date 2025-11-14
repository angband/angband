# Option to control verbosity of DLL deduction
option(DLL_DEDUCE_VERBOSE "Show debug messages during DLL deduction" OFF)

function(_deduce_dll_path lib_path dll_out dll_pattern)
    # dll_pattern: optional glob pattern, e.g., "zlib*.dll"
    if(NOT EXISTS "${lib_path}")
        if(DLL_DEDUCE_VERBOSE)
            message(STATUS "[_deduce_dll_path] Library does not exist: ${lib_path}")
        endif()
        set(${dll_out} "" PARENT_SCOPE)
        return()
    endif()

    get_filename_component(_lib_dir  "${lib_path}" DIRECTORY)
    get_filename_component(_lib_name "${lib_path}" NAME_WE)
    set(_dll_candidate "")
    set(_pattern "${dll_pattern}")

    if(NOT _pattern)
        set(_pattern "${_lib_name}*.dll")
        if(DLL_DEDUCE_VERBOSE)
            message(STATUS "[_deduce_dll_path] Using derived pattern: ${_pattern}")
        endif()
    endif()

    set(_search_dirs
        "${_lib_dir}"
        "${_lib_dir}/../bin"
        "${_lib_dir}/../lib"
    )

    if(DLL_DEDUCE_VERBOSE)
        message(STATUS "[_deduce_dll_path] Searching DLLs using pattern '${_pattern}'")
    endif()

    set(_all_found "")
    foreach(_dir IN LISTS _search_dirs)
        if(EXISTS "${_dir}")
            file(GLOB _dlls "${_dir}/${_pattern}")
            list(APPEND _all_found ${_dlls})
        endif()
    endforeach()

    list(REMOVE_DUPLICATES _all_found)
    list(SORT _all_found)

    if(DLL_DEDUCE_VERBOSE)
        message(STATUS "[_deduce_dll_path] Candidate DLLs:")
        if(_all_found)
            foreach(c ${_all_found})
                message(STATUS "   - ${c}")
            endforeach()
        else()
            message(STATUS "   (none found)")
        endif()
    endif()

    # Pick the first candidate
    list(GET _all_found 0 _picked_candidate)
    if(_picked_candidate)
        set(_dll_candidate "${_picked_candidate}")
        if(DLL_DEDUCE_VERBOSE)
            message(STATUS "[_deduce_dll_path] Selected DLL: ${_dll_candidate}")
        endif()
    else()
        if(DLL_DEDUCE_VERBOSE)
            message(WARNING "[_deduce_dll_path] No DLL matched pattern '${_pattern}' for '${lib_path}'")
        endif()
    endif()

    set(${dll_out} "${_dll_candidate}" PARENT_SCOPE)
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

            list(GET PNG_LIBRARIES 0 PNG_IMPLIB_PATH)
            message(STATUS "  PNG implib:   ${PNG_IMPLIB_PATH}")

            _deduce_dll_path("${PNG_IMPLIB_PATH}" PNG_DLL_PATH "")
            message(STATUS "  PNG DLL:      ${PNG_DLL_PATH}")

            add_library(SystemPNG SHARED IMPORTED)
            set_target_properties(SystemPNG PROPERTIES
                IMPORTED_IMPLIB "${PNG_IMPLIB_PATH}"
                IMPORTED_LOCATION "${PNG_DLL_PATH}"
                INTERFACE_INCLUDE_DIRECTORIES "${PNG_INCLUDE_DIRS}"
            )

            # ZLIB implib from PNG_LIBRARIES second element
            list(LENGTH PNG_LIBRARIES PNG_LIB_COUNT)
            if(PNG_LIB_COUNT GREATER 1)
                list(GET PNG_LIBRARIES 1 ZLIB_IMPLIB_PATH)
                message(STATUS "  ZLIB implib:  ${ZLIB_IMPLIB_PATH}")

                _deduce_dll_path("${ZLIB_IMPLIB_PATH}" ZLIB_DLL_PATH "zlib*.dll")
                message(STATUS "  ZLIB DLL:     ${ZLIB_DLL_PATH}")

                add_library(SystemZLib SHARED IMPORTED)
                set_target_properties(SystemZLib PROPERTIES
                    IMPORTED_IMPLIB "${ZLIB_IMPLIB_PATH}"
                    IMPORTED_LOCATION "${ZLIB_DLL_PATH}"
                )
                # Make PNG target link zlib transitively
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
