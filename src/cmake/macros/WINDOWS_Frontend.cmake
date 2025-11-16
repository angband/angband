macro(CONFIGURE_WINDOWS_FRONTEND _NAME_TARGET _ONLY_DEFINES)
    if(NOT _ONLY_DEFINES)
        if(NOT TARGET ${PNG_TARGET})
            message(FATAL_ERROR "CONFIGURE_WINDOWS_FRONTEND: PNG_TARGET '${PNG_TARGET}' is not a valid target")
        endif()

        if(NOT TARGET ${ZLIB_TARGET})
            message(FATAL_ERROR "CONFIGURE_WINDOWS_FRONTEND: ZLIB_TARGET '${ZLIB_TARGET}' is not a valid target")
        endif()

        target_link_libraries(${_NAME_TARGET}
            PRIVATE ${PNG_TARGET} ${ZLIB_TARGET} msimg32 winmm
        )
        set_target_properties(${_NAME_TARGET} PROPERTIES WIN32_EXECUTABLE ON)
    endif()

    target_compile_definitions(${_NAME_TARGET}
        PRIVATE 
            USE_WIN
            USE_PRIVATE_PATHS
            SOUND
            WINDOWS
            _CRT_SECURE_NO_WARNINGS
    )
endmacro()
