macro(configure_spoil_frontend _NAME_TARGET)

    target_compile_definitions(${_NAME_TARGET} PRIVATE -D USE_SPOIL)
    message(STATUS "Support for spoiler front end - Ready")

endmacro()
