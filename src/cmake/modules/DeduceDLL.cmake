option(DLL_DEDUCE_VERBOSE
       "Print diagnostic messages while deducing DLL locations"
       OFF)

# Deduce the DLL corresponding to a given library file.
#
# Usage:
#   _deduce_dll_path(<lib_path> <dll_out> [dll_pattern])
#
# Arguments:
#   lib_path     - Full path to a library (e.g., .../libz.dll.a or zlib.lib)
#   dll_out      - Name of the variable that will receive the DLL path
#   dll_pattern  - Optional glob pattern (e.g., "zlib*.dll");
#                  if omitted, a pattern is derived from the library name.
#
# Behavior:
#   - Searches the directory that contains the given library, and sibling
#     ../bin and ../lib directories.
#   - Collects all DLLs matching the pattern.
#   - Returns the lexicographically first match in <dll_out>.
#   - Returns an empty string if no candidate is found.
#
# Notes:
#   - If DLL_DEDUCE_VERBOSE is ON, detailed diagnostics are printed.
#
# Example:
#   _deduce_dll_path("${ZLIB_LIBRARIES}" ZLIB_DLL)
#   message(STATUS "Zlib runtime: ${ZLIB_DLL}")

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
