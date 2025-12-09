macro(configure_sdl2_frontend _NAME_TARGET)

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_TTF QUIET IMPORTED_TARGET SDL2_ttf>=2.0.0)
    pkg_check_modules(SDL2_IMAGE QUIET IMPORTED_TARGET SDL2_image>=2.0.0)

    if(SDL2_FOUND AND SDL2_IMAGE_FOUND AND SDL2_TTF_FOUND)

        if(SUPPORT_STATIC_LINKING)
            message(STATUS "Support for SDL2 front end - Configuring static linking")

            if(NOT TARGET PkgConfig::SDL2_STATIC) # Also defined in SDL2_Sound.cmake
                add_library(PkgConfig::SDL2_STATIC INTERFACE IMPORTED)
                set_target_properties(PkgConfig::SDL2_STATIC PROPERTIES
                    INTERFACE_LINK_LIBRARIES      "${SDL2_STATIC_LIBRARIES}"
                    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_STATIC_INCLUDE_DIRS}"
                    INTERFACE_COMPILE_OPTIONS     "${SDL2_STATIC_CFLAGS}"
                    INTERFACE_LINK_OPTIONS        "${SDL2_STATIC_LDFLAGS}"
                )
            endif()

            if(NOT TARGET PkgConfig::SDL2_TTF_STATIC)
                add_library(PkgConfig::SDL2_TTF_STATIC INTERFACE IMPORTED)
                set_target_properties(PkgConfig::SDL2_TTF_STATIC PROPERTIES
                    INTERFACE_LINK_LIBRARIES      "${SDL2_TTF_STATIC_LIBRARIES}"
                    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_TTF_STATIC_INCLUDE_DIRS}"
                    INTERFACE_COMPILE_OPTIONS     "${SDL2_TTF_STATIC_CFLAGS}"
                    INTERFACE_LINK_OPTIONS        "${SDL2_TTF_STATIC_LDFLAGS}"
                )
            endif()

            if(NOT TARGET PkgConfig::SDL2_IMAGE_STATIC)
                # pkg-config is missing another -lstdc++ after -lLerc on MSYS2
                add_library(PkgConfig::SDL2_IMAGE_STATIC INTERFACE IMPORTED)
                set_target_properties(PkgConfig::SDL2_IMAGE_STATIC PROPERTIES
                    INTERFACE_LINK_LIBRARIES      "${SDL2_IMAGE_STATIC_LIBRARIES};stdc++"
                    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_IMAGE_STATIC_INCLUDE_DIRS}"
                    INTERFACE_COMPILE_OPTIONS     "${SDL2_IMAGE_STATIC_CFLAGS}"
                    INTERFACE_LINK_OPTIONS        "${SDL2_IMAGE_STATIC_LDFLAGS};-lstdc++"
                )
            endif()

            target_link_options(${_NAME_TARGET} PRIVATE -static)
            target_link_libraries(${_NAME_TARGET} PRIVATE
                PkgConfig::SDL2_STATIC
                PkgConfig::SDL2_TTF_STATIC
                PkgConfig::SDL2_IMAGE_STATIC
            )
        else()
            TARGET_LINK_LIBRARIES(${_NAME_TARGET} PRIVATE
                PkgConfig::SDL2
                PkgConfig::SDL2_TTF
                PkgConfig::SDL2_IMAGE
            )
        endif()
        target_compile_definitions(${_NAME_TARGET} PRIVATE USE_SDL2)

        message(STATUS "Support for SDL2 front end - Ready")

    else()

        message(FATAL_ERROR "Support for SDL2 front end - Failed")

    endif()

endmacro()
