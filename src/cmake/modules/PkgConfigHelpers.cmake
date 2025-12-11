# ==========================================================
# PkgConfigHelpers.cmake
# Helper functions for selecting static vs dynamic PkgConfig targets
# ==========================================================

# Selects either the static or dynamic PkgConfig target based on SUPPORT_STATIC_LINKING
# Usage:
#   angband_pkgconfig_select_target(<PKG_NAME> <OUT_TARGET>)
# Example:
#   angband_pkgconfig_select_target(CURSES CURSES_SELECTED)
function(angband_pkgconfig_select_target PKG_NAME OUT_TARGET)
    set(DYN_TARGET    "PkgConfig::${PKG_NAME}")
    set(STATIC_TARGET "PkgConfig::${PKG_NAME}_STATIC")

    if(SUPPORT_STATIC_LINKING)
        if(NOT TARGET ${STATIC_TARGET})
            add_library(${STATIC_TARGET} INTERFACE IMPORTED)
            set_target_properties(${STATIC_TARGET} PROPERTIES
                INTERFACE_LINK_LIBRARIES      "${${PKG_NAME}_STATIC_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${${PKG_NAME}_STATIC_INCLUDE_DIRS}"
                INTERFACE_COMPILE_OPTIONS     "${${PKG_NAME}_STATIC_CFLAGS_OTHER}"
                INTERFACE_LINK_OPTIONS        "${${PKG_NAME}_STATIC_LDFLAGS_OTHER}"
            )

            target_link_options(${STATIC_TARGET} INTERFACE -static)

            # Work around missing transitive dependency:
            # SDL2_image → Lerc → C++ runtime (not declared in pkg-config)
            if(MINGW)
                target_link_libraries(${STATIC_TARGET} INTERFACE stdc++)
            endif()
        endif()
        set(${OUT_TARGET} ${STATIC_TARGET} PARENT_SCOPE)
    else()
        set(${OUT_TARGET} ${DYN_TARGET} PARENT_SCOPE)
    endif()
endfunction()
