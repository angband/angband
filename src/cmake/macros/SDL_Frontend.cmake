macro(configure_sdl_frontend _NAME_TARGET)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL QUIET IMPORTED_TARGET sdl)
    pkg_check_modules(SDL_TTF QUIET IMPORTED_TARGET SDL_ttf)
    pkg_check_modules(SDL_IMAGE QUIET IMPORTED_TARGET SDL_image)

    if(SDL_FOUND AND SDL_TTF_FOUND AND SDL_IMAGE_FOUND)

        target_link_libraries(${_NAME_TARGET} PRIVATE PkgConfig::SDL PkgConfig::SDL_TTF PkgConfig::SDL_IMAGE)

        target_compile_definitions(${_NAME_TARGET} PRIVATE USE_SDL)
        message(STATUS "Support for SDL front end - Ready")

    else()

        message(FATAL_ERROR "Support for SDL front end - Failed")

    endif()

endmacro()
