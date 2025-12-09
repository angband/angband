macro(configure_sdl2_frontend _NAME_TARGET)

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_TTF QUIET IMPORTED_TARGET SDL2_ttf>=2.0.0)
    pkg_check_modules(SDL2_IMAGE QUIET IMPORTED_TARGET SDL2_image>=2.0.0)

    if(SDL2_FOUND AND SDL2_IMAGE_FOUND AND SDL2_TTF_FOUND)

        include(PkgConfigHelpers)
        angband_pkgconfig_select_target(SDL2       SDL2_SELECTED)
        angband_pkgconfig_select_target(SDL2_TTF   SDL2_TTF_SELECTED)
        angband_pkgconfig_select_target(SDL2_IMAGE SDL2_IMAGE_SELECTED)

        target_link_libraries(${_NAME_TARGET} PRIVATE
            ${SDL2_SELECTED}
            ${SDL2_TTF_SELECTED}
            ${SDL2_IMAGE_SELECTED}
        )
        target_compile_definitions(${_NAME_TARGET} PRIVATE USE_SDL2)

        message(STATUS "Support for SDL2 front end - Ready")

    else()

        message(FATAL_ERROR "Support for SDL2 front end - Failed")

    endif()

endmacro()
