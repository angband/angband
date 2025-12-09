macro(configure_windows_frontend _NAME_TARGET _PNG_TARGET)
    if(TARGET "${_PNG_TARGET}")
        if(SUPPORT_STATIC_LINKING)
            target_link_options(${_NAME_TARGET} PRIVATE -static)
        endif()
        target_link_libraries(${_NAME_TARGET} PRIVATE ${_PNG_TARGET} msimg32 winmm)
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
