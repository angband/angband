MACRO(CONFIGURE_SDL2_SOUND _NAME_TARGET _ONLY_DEFINES)
    SET(PREVIOUS_INVOCATION ${CONFIGURE_SDL2_SOUND_INVOKED_PREVIOUSLY})
    FIND_PACKAGE(PkgConfig REQUIRED)
    PKG_CHECK_MODULES(SDL2 QUIET IMPORTED_TARGET sdl2)
    PKG_CHECK_MODULES(SDL2_MIXER QUIET IMPORTED_TARGET SDL2_mixer>=2.0.0)
    IF(SDL2_FOUND AND SDL2_MIXER_FOUND)
        IF(NOT _ONLY_DEFINES)
            if (SUPPORT_STATIC_LINKING)
                if (NOT TARGET PkgConfig::SDL2_STATIC) # Also defined in SDL2_Frontend.cmake
                    add_library(PkgConfig::SDL2_STATIC INTERFACE IMPORTED)
                    set_target_properties(PkgConfig::SDL2_STATIC PROPERTIES
                        INTERFACE_LINK_LIBRARIES      "${SDL2_STATIC_LIBRARIES}"
                        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_STATIC_INCLUDE_DIRS}"
                        INTERFACE_COMPILE_OPTIONS     "${SDL2_STATIC_CFLAGS}"
                        INTERFACE_LINK_OPTIONS        "${SDL2_STATIC_LDFLAGS}"
                    )
                endif()
                if (NOT TARGET PkgConfig::SDL2_MIXER_STATIC)
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
        ENDIF()
        TARGET_COMPILE_DEFINITIONS(${_NAME_TARGET} PRIVATE -D SOUND_SDL2)
        TARGET_COMPILE_DEFINITIONS(${_NAME_TARGET} PRIVATE -D SOUND)
        IF(NOT PREVIOUS_INVOCATION)
            MESSAGE(STATUS "Support for sound with SDL2 - Ready")
        ENDIF()
        SET(CONFIGURE_SDL2_SOUND_INVOKED_PREVIOUSLY YES CACHE
            INTERNAL "Mark if CONFIGURE_SDL_SOUND called successfully" FORCE)
    ELSE()
        MESSAGE(FATAL_ERROR "Support for sound with SDL2 - Failed")
    ENDIF()
ENDMACRO()
