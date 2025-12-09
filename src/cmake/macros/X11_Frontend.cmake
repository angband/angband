macro(configure_x11_frontend _NAME_TARGET)
    find_package(X11)

    if(X11_FOUND)
        target_link_libraries(${_NAME_TARGET} PRIVATE ${X11_LIBRARIES})
        target_include_directories(${_NAME_TARGET} PRIVATE ${X11_INCLUDE_DIR})
        target_compile_definitions(${_NAME_TARGET} PRIVATE -D USE_X11)
        message(STATUS "Support for X11 front end - Ready")

    else()
        message(FATAL_ERROR "Support for X11 front end - Failed")

    endif()
endmacro()
