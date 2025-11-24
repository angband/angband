# ----------------------------------------------
# MSYS2 MinGW64 Toolchain file for GCU frontend
# ----------------------------------------------
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Prefer static linking for console ncurses build
set(CMAKE_EXE_LINKER_FLAGS "-static")

# MSYS2 GCU build-specific definitions
add_compile_definitions(WIN32_CONSOLE_MODE NCURSES_STATIC MSYS2_ENCODING_WORKAROUND)