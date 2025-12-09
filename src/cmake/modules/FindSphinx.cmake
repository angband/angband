# Locate sphinx-build.  If found, Sphinx_FOUND will be true and
# SPHINX_EXECUTABLE will have the path for the executable.
#
# Adapted from https://www.vortech.nl/en/integrating-sphinx-in-cmake/ .

include(FindPackageHandleStandardArgs)

unset(_SEARCH_PATH)

# Sphinx may be installed near the Python interpreter.
if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
    find_package(PythonInterp)
    if(PYTHONINTERP_FOUND)
        get_filename_component(_PYTHON_DIR "${PYTHON_EXECUTABLE}" DIRECTORY)
        list(
            APPEND
            _SEARCH_PATH
            "${_PYTHON_DIR}"
            "${_PYTHON_DIR}/bin"
            "${_PYTHON_DIR}/Scripts"
        )
    endif()
else()
    find_package(Python COMPONENTS Interpreter)
    if(Python_FOUND)
        get_filename_component(_PYTHON_DIR "${Python_EXECUTABLE}" DIRECTORY)
        list(
            APPEND
            _SEARCH_PATH
            "${_PYTHON_DIR}"
            "${_PYTHON_DIR}/bin"
            "${_PYTHON_DIR}/Scripts"
        )
    endif()
endif()

# Check /usr/share/sphinx/scripts/python{3,2}; Ubuntu uses those.
list(
    APPEND
    _SEARCH_PATH
    "/usr/share/sphinx/scripts/python3"
    "/usr/share/sphinx/scripts/python2"
)

find_program(
    SPHINX_EXECUTABLE
    NAMES sphinx-build sphinx-build3 sphinx-build2 sphinx-build.exe
    HINTS ${_SEARCH_PATH}
)
mark_as_advanced(SPHINX_EXECUTABLE)

find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)
