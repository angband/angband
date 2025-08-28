# Define check_coverage(), configure_target_for_coverage(), and
# add_coverage_targets().

# Determine if the compiler and other available tools will support generating
# code coverage information.
#
# check_coverage(
#     <variable>
# )
#
# Will set an internal cache variable whose name is <variable> to a boolean
# value indicating whether or not code coverage is available.
#
# The following non-internal cache variables affect how this function
# operates:
#
# LLVM_COV_PATH is how to invoke LLVM's llvm-cov tool.
# GCOV_PATH is how to invoke gcov.
#
# Sets these internal cache variables for use by
# configure_target_for_coverage() and add_coverage_rules():
#
# COVERAGE_C_COMPILER_FLAGS is the C compiler flags necessary for code
# coverage.
# COVERAGE_C_LINKER_FLAGS is the C compiler as linker flags necessary for
# code coverage.
# COVERAGE_C_LIBS is the libraries necesary to link with compiled C code
# for code coverage.
# COVERAGE_C_IMPLEMENTATION is the code name for how code coverage is
# implemented.  It is either GCC+GCOV (gcc with gcov to produce reports),
# CLANG+GCOV (clang with its gcov emulation mode; use "llvm-cov gcov" to
# produce reports), or NONE (we do not know how to generate code coverage
# on this system).
#
# Limitations:
# Assumes C as the language.
# Does nothing for layering other reporting tools (gcovr, fastcov, lcov, ...)
#     on top of what gcov or llvm-cov provide but does use the src/gen-coverage
#     Perl script from CMAKE_CURRENT_SOURCE_DIR when reporting on gcov results
#     (either from gcc or clang).
# Does not give the option to use clang's source coverage mode (
#     https://clang.llvm.org/docs/SourceBasedCodeCoverage.html ).
#
FUNCTION(check_coverage _VARIABLE)
    # Parse the arguments.
    CMAKE_PARSE_ARGUMENTS(_IN "" "" "" ${ARGN})
    IF(DEFINED _IN_UNPARSED_ARGUMENTS)
        MESSAGE(FATAL_ERROR "check_coverage() was called with one or more unrecognized arguments: ${_IN_UNPARSED_ARGUMENTS}")
    ENDIF()

    # We know how to work with gcc or clang (only in its gcov coverage mode).
    INCLUDE(CheckCCompilerFlag)
    INCLUDE(CheckLinkerFlag)
    IF("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
        FIND_PROGRAM(LLVM_COV_PATH llvm-cov
            DOC "LLVM's tool to emit coverage information, llvm-cov")
        SET(CMAKE_REQUIRED_LINK_OPTIONS "--coverage")
        CHECK_C_COMPILER_FLAG(--coverage HAVE_c_coverage)
        SET(CMAKE_REQUIRED_LINK_OPTIONS "--fprofile-arcs -ftest-coverage")
        CHECK_C_COMPILER_FLAG(-fprofile-arcs HAVE_c_fprofile_arcs)
        CHECK_C_COMPILER_FLAG(-ftest-coverage HAVE_c_ftest_coverage)
        IF((LLVM_COV_PATH) AND ((HAVE_c_coverage) OR ((HAVE_c_fprofile_arcs) AND (HAVE_c_ftest_coverage))))
            SET(_COVERAGE_C_COMPILER_FLAGS)
            IF(HAVE_c_coverage)
                LIST(APPEND _COVERAGE_C_COMPILER_FLAGS --coverage)
            ELSE()
                LIST(APPEND _COVERAGE_C_COMPILER_FLAGS -fprofile-arcs -ftest-coverage)
            ENDIF()
            SET("${_VARIABLE}" YES CACHE INTERNAL
                 "System supports code coverage")
            SET(COVERAGE_C_COMPILER_FLAGS ${_COVERAGE_C_COMPILER_FLAGS}
                CACHE INTERNAL "C compiler flags for code coverage")
            SET(COVERAGE_C_LINKER_FLAGS ${_COVERAGE_C_COMPILER_FLAGS}
                CACHE INTERNAL
                "C compiler as linker flags for code coverage")
            SET(COVERAGE_C_LIBS "" CACHE INTERNAL
                "Libraries for C code coverage")
            SET(COVERAGE_C_IMPLEMENTATION "CLANG+GCOV" CACHE INTERNAL
                "Code coverage implementation")
            RETURN()
        ENDIF()
    ELSEIF("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
        FIND_PROGRAM(GCOV_PATH gcov)
        CHECK_LINKER_FLAG(C -lgcov HAVE_linker_c_lgcov)
        IF((GCOV_PATH) AND (HAVE_linker_c_lgcov))
            SET(CMAKE_REQUIRED_LIBRARIES "gcov")
            CHECK_C_COMPILER_FLAG(--coverage HAVE_c_coverage)
            CHECK_C_COMPILER_FLAG(-fprofile-arcs HAVE_c_fprofile_arcs)
            CHECK_C_COMPILER_FLAG(-ftest-coverage HAVE_c_ftest_coverage)
            IF((HAVE_c_coverage) OR ((HAVE_c_fprofile_arcs) AND (HAVE_c_ftest_coverage)))
                SET(_COVERAGE_C_COMPILER_FLAGS)
                IF(HAVE_c_coverage)
                    LIST(APPEND _COVERAGE_C_COMPILER_FLAGS --coverage)
                ELSE()
                    LIST(APPEND _COVERAGE_C_COMPILER_FLAGS -fprofile-arcs -ftest-coverage)
                ENDIF()
                CHECK_C_COMPILER_FLAG(-fprofile-abs-path HAVE_c_fprofile_abs_path)
                IF(HAVE_c_fprofile_abs_path)
                    LIST(APPEND _COVERAGE_C_COMPILER_FLAGS -fprofile-abs-path)
                ENDIF()
                SET("${_VARIABLE}" YES CACHE INTERNAL
                    "System supports code coverage")
                SET(COVERAGE_C_COMPILER_FLAGS ${_COVERAGE_C_COMPILER_FLAGS}
                    CACHE INTERNAL "C compiler flags for code coverage")
                SET(COVERAGE_C_LINKER_FLAGS "" CACHE INTERNAL
                    "C compiler as linker flags for code coverage")
                SET(COVERAGE_C_LIBS "gcov" CACHE INTERNAL
                    "Libraries for C code coverage")
                SET(COVERAGE_C_IMPLEMENTATION "GCC+GCOV" CACHE INTERNAL
                    "Code coverage implementation")
                RETURN()
            ENDIF()
        ENDIF()
    ENDIF()
    SET("${_VARIABLE}" NO CACHE INTERNAL "System supports code coverage")
    SET(COVERAGE_C_COMPILER_FLAGS "" CACHE INTERNAL
        "C compiler flags for code coverage")
    SET(COVERAGE_C_LINKER_FLAGS "" CACHE INTERNAL
        "C compiler as linker flags for code coverage")
    SET(COVERAGE_C_LIBS "" CACHE INTERNAL "Libraries for C code coverage")
    SET(COVERAGE_C_IMPLEMENTATION "NONE" CACHE INTERNAL
        "Code coverage implementation")
ENDFUNCTION()


# Configure a target so it is compiled (optional) and linked with code coverage.
#
# configure_target_for_coverage(
#     <target>
#     [LINKONLY]
# )
#
# <target> is the name of a target created by a command such as add_executable()
#     or add_library() and must not be an ALIAS target.
# LINKONLY specifies that the targets will only link in code coverage:  any
#     code that is directly compiled (i.e. not through a dependency of the
#     target) for the target will be excluded from code coverage.
#
FUNCTION(configure_target_for_coverage _TARGET)
    # Parse the arguments.
    SET(_OPTIONS LINKONLY)
    SET(_ONE_VALUE_ARGS)
    SET(_MULT_VALUE_ARGS)
    CMAKE_PARSE_ARGUMENTS(_IN "${_OPTIONS}" "${_ONE_VALUE_ARGS}" "${_MULT_VALUE_ARGS}" ${ARGN})
    IF(DEFINED _IN_UNPARSED_ARGUMENTS)
        MESSAGE(FATAL_ERROR "configure_target_for_coverage() was called with one or more unrecognized arguments: ${_IN_UNPARSED_ARGUMENTS}")
    ENDIF()
    IF((NOT _IN_LINKONLY) AND (DEFINED COVERAGE_C_COMPILER_FLAGS) AND (NOT (COVERAGE_C_COMPILER_FLAGS STREQUAL "")))
        TARGET_COMPILE_OPTIONS(${_TARGET} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:${COVERAGE_C_COMPILER_FLAGS}>)
    ENDIF()
    GET_TARGET_PROPERTY(_TARGET_TYPE ${_TARGET} TYPE)
    IF((NOT (_TARGET_TYPE STREQUAL "STATIC_LIBRARY")) AND (NOT (_TARGET_TYPE STREQUAL "OBJECT_LIBRARY")))
        IF((DEFINED COVERAGE_C_LINKER_FLAGS) AND (NOT (COVERAGE_C_LINKER_FLAGS STREQUAL "")))
            TARGET_LINK_OPTIONS(${_TARGET} PUBLIC ${COVERAGE_C_LINKER_FLAGS})
        ENDIF()
    ENDIF()
    IF((DEFINED COVERAGE_C_LIBS) AND (NOT (COVERAGE_C_LIBS STREQUAL "")))
        TARGET_LINK_LIBRARIES(${_TARGET} PUBLIC ${COVERAGE_C_LIBS})
    ENDIF()
ENDFUNCTION()


# Define targets to reset accumulated coverage data, report on the accumulated
# coverage data, and a convenience target to reset accumulated coverage data,
# perform one or more operations to accumulate coverage data, and then
# report the coverage results.
#
# add_coverage_targets(
#     <reset_target>
#     <report_target>
#     <combined_target>
#     [COMBINED_SUBTARGETS <target1> ...]
# )
#
# <reset_target> is the name to use for the target to reset accumulated coverage
#     data.
# <report_target> is the name to use for the target to report on the accumulated
#     coverage data.
# <combined_target> is the name to use for the target that resets the
#     accumulated coverage data, performs one or more operations to accumulate
#     coverage data, and then report the coverage results.
# COMBINED_SUBTARGETS specifies the names of one or more targets that the
#     combined target will invoke to accumulate coverage data.
#
# Limitations
# The commands for the custom targets assume a Unix-like shell environment
# and namely the ability to use pipes and the find, rm, sed, tr, and xargs
# utilities.
# The way files are found for reporting coverage does not handle file names
# that contain newlines (do not use -print0 since do not have a way to
# portably have sed handle the null-delimited file names when it converts
# .gcda extensions to .o).
#
FUNCTION(add_coverage_targets _RESET_TARGET _REPORT_TARGET _COMBINED_TARGET)
    # Parse the arguments.
    SET(_OPTIONS)
    SET(_ONE_VALUE_ARGS)
    SET(
        _MULT_VALUE_ARGS
        COMBINED_SUBTARGETS
    )
    CMAKE_PARSE_ARGUMENTS(_IN "${_OPTIONS}" "${_ONE_VALUE_ARGS}" "${_MULT_VALUE_ARGS}" ${ARGN})
    IF(DEFINED _IN_UNPARSED_ARGUMENTS)
        MESSAGE(FATAL_ERROR "add_coverage_targets() was called with one or more unrecognized arguments: ${_IN_UNPARSED_ARGUMENTS}")
    ENDIF()

    # Use VERBATIM so CMake does the quoting/escaping necessary necessary
    # to handle tool-specific characters.  Still need to apply some
    # quoting/escaping for CMake:
    #     1) Double quote variable expansions that have to be presented to the
    #        tool as one piece, even if the variable expands to something with
    #        whitespace.
    #     2) Backslash escape parenthesis used in find commands so CMake does
    #        not parse them.
    #     3) Backslash escape dollar signs in sed commands so CMake does not
    #        parse them.
    #     4) When a backslash is needed in the command, use a double backslash.
    IF(DEFINED COVERAGE_C_IMPLEMENTATION)
        IF((COVERAGE_C_IMPLEMENTATION STREQUAL "GCC+GCOV") OR (COVERAGE_C_IMPLEMENTATION STREQUAL "CLANG+GCOV"))
            ADD_CUSTOM_TARGET(${_RESET_TARGET}
                COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" \( -name *.gcda -o -name *.gcov \) -print0 | xargs -0 rm -f
                VERBATIM)
        ELSE()
            ADD_CUSTOM_TARGET(${_RESET_TARGET})
        ENDIF()
    ELSE()
        ADD_CUSTOM_TARGET(${_RESET_TARGET})
    ENDIF()

    IF(DEFINED COVERAGE_C_IMPLEMENTATION)
        IF(COVERAGE_C_IMPLEMENTATION STREQUAL "GCC+GCOV")
            ADD_CUSTOM_TARGET(${_REPORT_TARGET}
                COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name *.gcda -print | sed s/\\.gcda\$/.o/ | tr \\n \\0 | xargs -0 "${GCOV_PATH}" -p
                COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name *.gcov -print0 | xargs -0 "${CMAKE_CURRENT_SOURCE_DIR}/src/gen-coverage"
                VERBATIM)
        ELSEIF(COVERAGE_C_IMPLEMENTATION STREQUAL "CLANG+GCOV")
            ADD_CUSTOM_TARGET(${_REPORT_TARGET}
                COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name *.gcda -print | sed s/\\.gcda\$/.o/ | tr \\n \\0 | xargs -0 "${LLVM_COV_PATH}" gcov -p
                COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name *.gcov -print0 | xargs -0 "${CMAKE_CURRENT_SOURCE_DIR}/src/gen-coverage"
                VERBATIM)
        ELSE()
            ADD_CUSTOM_TARGET(${_REPORT_TARGET})
        ENDIF()
    ELSE()
        ADD_CUSTOM_TARGET(${_REPORT_TARGET})
    ENDIF()

    IF(DEFINED COVERAGE_C_IMPLEMENTATION)
        IF(DEFINED _IN_COMBINED_SUBTARGETS)
            STRING(REPLACE ";" " " _SUBTARGETS_STR "${_IN_COMBINED_SUBTARGETS}")
            IF(COVERAGE_C_IMPLEMENTATION STREQUAL "GCC+GCOV")
                ADD_CUSTOM_TARGET(${_COMBINED_TARGET}
                    COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" \( -name *.gcda -o -name *.gcov \) -print0 | xargs -0 rm -f
                    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}" -t ${_SUBTARGETS_STR}
                    COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name *.gcda -print | sed s/\\.gcda\$/.o/ | tr \\n \\0 | xargs -0 "${GCOV_PATH}" -p
                    COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name *.gcov -print0 | xargs -0 "${CMAKE_CURRENT_SOURCE_DIR}/src/gen-coverage"
                    VERBATIM)
            ELSEIF(COVERAGE_C_IMPLEMENTATION STREQUAL "CLANG+GCOV")
                ADD_CUSTOM_TARGET(${_COMBINED_TARGET}
                    COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" \( -name *.gcda -o -name *.gcov \) -print0 | xargs -0 rm -f
                    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}" -t ${_SUBTARGETS_STR}
                    COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name *.gcda -print | sed s/\\.gcda\$/.o/ | tr \\n \\0 | xargs -0 "${LLVM_COV_PATH}" gcov -p
                    COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name *.gcov -print0 | xargs -0 "${CMAKE_CURRENT_SOURCE_DIR}/src/gen-coverage"
                    VERBATIM)
            ELSE()
                ADD_CUSTOM_TARGET(${_COMBINED_TARGET})
            ENDIF()
        ELSE()
            # Since no targets to generate coverage data were specified,
            # the combined target devolves to be the same as the reset one.
            IF((COVERAGE_C_IMPLEMENTATION STREQUAL "GCC+GCOV")
                    OR (COVERAGE_C_IMPLEMENTATION STREQUAL "CLANG+GCOV"))
                ADD_CUSTOM_TARGET(${_COMBINED_TARGET}
                    COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" \( -name *.gcda -o -name *.gcov \) -print0 | xargs -0 rm -f
                    VERBATIM)
            ELSE()
                ADD_CUSTOM_TARGET(${_COMBINED_TARGET})
            ENDIF()
        ENDIF()
    ELSE()
        ADD_CUSTOM_TARGET(${_COMBINED_TARGET})
    ENDIF()
ENDFUNCTION()
