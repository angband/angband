macro(configure_test_frontend _NAME_TARGET)

    target_compile_definitions(${_NAME_TARGET} PRIVATE -D USE_TEST)
    message(STATUS "Support for test front end - Ready")

endmacro()
