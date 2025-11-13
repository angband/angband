MACRO(CONFIGURE_WINDOWS_FRONTEND _NAME_TARGET _ONLY_DEFINES)
    # Define imported libraries once globally
    if(NOT TARGET OurWindowsZLib)
        ADD_LIBRARY(OurWindowsZLib SHARED IMPORTED)
        SET_TARGET_PROPERTIES(OurWindowsZLib PROPERTIES
            IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/src/win/lib/zlib.lib")
        TARGET_INCLUDE_DIRECTORIES(OurWindowsZLib INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/win/include")
    endif()

    if(NOT TARGET OurWindowsPNGLib)
        ADD_LIBRARY(OurWindowsPNGLib SHARED IMPORTED)
        SET_TARGET_PROPERTIES(OurWindowsPNGLib PROPERTIES
            IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/src/win/lib/libpng.lib")
        TARGET_INCLUDE_DIRECTORIES(OurWindowsPNGLib INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/win/include")
    endif()

    # Always link and define flags for this target
    if(NOT _ONLY_DEFINES)
        TARGET_LINK_LIBRARIES(${_NAME_TARGET}
            PRIVATE OurWindowsZLib OurWindowsPNGLib msimg32 winmm)
        SET_TARGET_PROPERTIES(${_NAME_TARGET} PROPERTIES WIN32_EXECUTABLE ON)
    endif()

    TARGET_COMPILE_DEFINITIONS(${_NAME_TARGET} 
        PRIVATE 
            -D USE_WIN
            -D USE_PRIVATE_PATHS
            -D SOUND
            -D WINDOWS
            -D _CRT_SECURE_NO_WARNINGS
    )

    message(STATUS "Configured Windows front end for target ${_NAME_TARGET}")
ENDMACRO()
