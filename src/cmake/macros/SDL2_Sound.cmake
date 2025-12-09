macro(configure_sdl2_sound _NAME_TARGET _ONLY_DEFINES)
    set(PREVIOUS_INVOCATION ${CONFIGURE_SDL2_SOUND_INVOKED_PREVIOUSLY})
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_MIXER QUIET IMPORTED_TARGET SDL2_mixer>=2.0.0)
    if(SDL2_FOUND AND SDL2_MIXER_FOUND)
        if(NOT _ONLY_DEFINES)
            if(SUPPORT_STATIC_LINKING)
                if(NOT TARGET PkgConfig::SDL2_STATIC) # Also defined in SDL2_Frontend.cmake
                    add_library(PkgConfig::SDL2_STATIC INTERFACE IMPORTED)
                    set_target_properties(PkgConfig::SDL2_STATIC PROPERTIES
                        INTERFACE_LINK_LIBRARIES      "${SDL2_STATIC_LIBRARIES}"
                        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_STATIC_INCLUDE_DIRS}"
                        INTERFACE_COMPILE_OPTIONS     "${SDL2_STATIC_CFLAGS}"
                        INTERFACE_LINK_OPTIONS        "${SDL2_STATIC_LDFLAGS}"
                    )
                endif()
                if(NOT TARGET PkgConfig::SDL2_MIXER_STATIC)
                    add_library(PkgConfig::SDL2_MIXER_STATIC INTERFACE IMPORTED)
                    set_target_properties(PkgConfig::SDL2_MIXER_STATIC PROPERTIES
                        INTERFACE_LINK_LIBRARIES      "${SDL2_MIXER_STATIC_LIBRARIES}"
                        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_MIXER_STATIC_INCLUDE_DIRS}"
                        INTERFACE_COMPILE_OPTIONS     "${SDL2_MIXER_STATIC_CFLAGS}"
                        INTERFACE_LINK_OPTIONS        "${SDL2_MIXER_STATIC_LDFLAGS}"
                    )
                endif()
                target_link_options(${_NAME_TARGET} PRIVATE -static)
                target_link_libraries(${_NAME_TARGET} PRIVATE PkgConfig::SDL2_STATIC PkgConfig::SDL2_MIXER_STATIC)
            else()
                target_link_libraries(${_NAME_TARGET} PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_MIXER)
            endif()
        endif()
        target_compile_definitions(${_NAME_TARGET} PRIVATE -D SOUND_SDL2)
        target_compile_definitions(${_NAME_TARGET} PRIVATE -D SOUND)
        if(NOT PREVIOUS_INVOCATION)
            message(STATUS "Support for sound with SDL2 - Ready")
        endif()
        set(CONFIGURE_SDL2_SOUND_INVOKED_PREVIOUSLY YES CACHE
            INTERNAL "Mark if CONFIGURE_SDL_SOUND called successfully" FORCE)
    else()
        message(FATAL_ERROR "Support for sound with SDL2 - Failed")
    endif()
endmacro()
