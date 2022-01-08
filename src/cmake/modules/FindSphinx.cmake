# Locate sphinx-build.  If found, Sphinx_FOUND will be true and
# SPHINX_EXECUTABLE will have the path for the executable.
#
# Adapted from https://www.vortech.nl/en/integrating-sphinx-in-cmake/ .

INCLUDE(FindPackageHandleStandardArgs)

UNSET(_SEARCH_PATH)

# Sphinx may be installed near the Python interpreter.
IF(${CMAKE_VERSION} VERSION_LESS "3.12.0")
    FIND_PACKAGE(PythonInterp)
    IF(PYTHONINTERP_FOUND)
        GET_FILENAME_COMPONENT(_PYTHON_DIR "${PYTHON_EXECUTABLE}" DIRECTORY)
        LIST(
            APPEND
            _SEARCH_PATH
            "${_PYTHON_DIR}"
            "${_PYTHON_DIR}/bin"
            "${_PYTHON_DIR}/Scripts"
        )
    ENDIF()
ELSE()
    FIND_PACKAGE(Python COMPONENTS Interpreter)
    IF(Python_FOUND)
        GET_FILENAME_COMPONENT(_PYTHON_DIR "${Python_EXECUTABLE}" DIRECTORY)
        LIST(
            APPEND
            _SEARCH_PATH
            "${_PYTHON_DIR}"
            "${_PYTHON_DIR}/bin"
            "${_PYTHON_DIR}/Scripts"
        )
    ENDIF()
ENDIF()

# Check /usr/share/sphinx/scripts/python{3,2}; Ubuntu uses those.
LIST(
    APPEND
    _SEARCH_PATH
    "/usr/share/sphinx/scripts/python3"
    "/usr/share/sphinx/scripts/python2"
)

FIND_PROGRAM(
    SPHINX_EXECUTABLE
    NAMES sphinx-build sphinx-build3 sphinx-build2 sphinx-build.exe
    HINTS ${_SEARCH_PATH}
)
MARK_AS_ADVANCED(SPHINX_EXECUTABLE)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)
