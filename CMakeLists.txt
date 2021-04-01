CMAKE_MINIMUM_REQUIRED(VERSION 3.5 FATAL_ERROR)
PROJECT(Angband VERSION 4.2.1 LANGUAGES C)

IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    MESSAGE(FATAL_ERROR "Not support to CMake in Windows")
ENDIF()

OPTION(SUPPORT_NCURSES_FRONTEND "Support to NCurses Frontend." OFF)
OPTION(SUPPORT_SDL_FRONTEND "Support to SDL Frontend." OFF)
OPTION(SUPPORT_X11_FRONTEND "Support to X11 Frontend." ON)
OPTION(SUPPORT_SDL_SOUND "Support to Sound System with SDL." OFF)

# For default, X11 is the frontend, if the user want specific
# a new frontend is needed set to false (OFF) the frontend X11
IF (SUPPORT_NCURSES_FRONTEND OR SUPPORT_SDL_FRONTEND)
    SET(SUPPORT_X11_FRONTEND OFF)
ENDIF()

IF (NOT SUPPORT_SDL_FRONTEND)
    SET(SUPPORT_SDL_SOUND OFF)
ENDIF()

ADD_EXECUTABLE(Angband
        src/buildid.c
        src/cave-map.c
        src/cave-square.c
        src/cave-view.c
        src/cave.c
        src/cmd-cave.c
        src/cmd-core.c
        src/cmd-misc.c
        src/cmd-obj.c
        src/cmd-pickup.c
        src/cmd-wizard.c
        src/datafile.c
        src/debug.c
        src/effects.c
        src/effects-info.c
        src/game-event.c
        src/game-input.c
        src/game-world.c
        src/gen-cave.c
        src/gen-chunk.c
        src/gen-monster.c
        src/gen-room.c
        src/gen-util.c
        src/generate.c
        src/grafmode.c
        src/guid.c
        src/init.c
        src/load.c
#        src/main-cocoa.m
        $<$<BOOL:${SUPPORT_NCURSES_FRONTEND}>:src/main-gcu.c>
#        src/main-nds.c
        $<$<BOOL:${SUPPORT_SDL_FRONTEND}>:src/main-sdl.c>
#        src/main-sdl2.c
#        src/main-stats.c
#        src/main-test.c
#        src/main-win.c
        $<$<BOOL:${SUPPORT_X11_FRONTEND}>:src/main-x11.c>
#        src/main-xxx.c
        src/main.c
        src/message.c
        src/mon-attack.c
        src/mon-blows.c
        src/mon-desc.c
        src/mon-group.c
        src/mon-init.c
        src/mon-list.c
        src/mon-lore.c
        src/mon-make.c
        src/mon-move.c
        src/mon-msg.c
        src/mon-predicate.c
        src/mon-spell.c
        src/mon-summon.c
        src/mon-timed.c
        src/mon-util.c
        src/obj-chest.c
        src/obj-curse.c
        src/obj-desc.c
        src/obj-gear.c
        src/obj-ignore.c
        src/obj-info.c
        src/obj-init.c
        src/obj-knowledge.c
        src/obj-list.c
        src/obj-make.c
        src/obj-pile.c
        src/obj-power.c
        src/obj-properties.c
        src/obj-randart.c
        src/obj-slays.c
        src/obj-tval.c
        src/obj-util.c
        src/option.c
        src/parser.c
        src/player-attack.c
        src/player-birth.c
        src/player-calcs.c
        src/player-class.c
        src/player-history.c
        src/player-path.c
        src/player-properties.c
        src/player-quest.c
        src/player-race.c
        src/player-spell.c
        src/player-timed.c
        src/player-util.c
        src/player.c
        src/project-feat.c
        src/project-mon.c
        src/project-obj.c
        src/project-player.c
        src/project.c
        src/randname.c
        src/save.c
        src/save-charoutput.c
        src/savefile.c
        src/score.c

        $<$<BOOL:${SUPPORT_SDL_SOUND}>:src/snd-sdl.c>

        src/sound-core.c
        src/source.c
        src/store.c
        src/target.c
        src/trap.c
        src/ui-birth.c
        src/ui-command.c
        src/ui-context.c
        src/ui-curse.c
        src/ui-death.c
        src/ui-display.c
        src/ui-entry-combiner.c
        src/ui-entry-renderers.c
        src/ui-entry.c
        src/ui-equip-cmp.c
        src/ui-event.c
        src/ui-game.c
        src/ui-help.c
        src/ui-history.c
        src/ui-init.c
        src/ui-input.c
        src/ui-keymap.c
        src/ui-knowledge.c
        src/ui-map.c
        src/ui-menu.c
        src/ui-mon-list.c
        src/ui-mon-lore.c
        src/ui-obj-list.c
        src/ui-object.c
        src/ui-options.c
        src/ui-output.c
        src/ui-player.c
        src/ui-prefs.c
        src/ui-score.c
        src/ui-signals.c
        src/ui-spell.c
        src/ui-store.c
        src/ui-target.c
        src/ui-term.c
        src/ui-visuals.c
        src/wiz-debug.c
        src/wiz-spoil.c
        src/wiz-stats.c
        src/z-bitflag.c
        src/z-color.c
        src/z-dice.c
        src/z-expression.c
        src/z-file.c
        src/z-form.c
        src/z-quark.c
        src/z-queue.c
        src/z-rand.c
        src/z-set.c
        src/z-textblock.c
        src/z-type.c
        src/z-util.c
        src/z-virt.c
)

SET_TARGET_PROPERTIES(Angband PROPERTIES C_STANDARD 99)

IF (SUPPORT_X11_FRONTEND)

    INCLUDE(src/cmake/macros/X11_Frontend.cmake)
    CONFIGURE_X11_FRONTEND(Angband)

ELSEIF(SUPPORT_NCURSES_FRONTEND)

    INCLUDE(src/cmake/macros/CURSES_Frontend.cmake)
    CONFIGURE_CURSES_FRONTEND(Angband)

ELSEIF(SUPPORT_SDL_FRONTEND)

    INCLUDE(src/cmake/macros/SDL_Frontend.cmake)
    CONFIGURE_SDL_FRONTEND(Angband)

    IF (SUPPORT_SDL_SOUND)

        INCLUDE(src/cmake/macros/SDL_Sound.cmake)
        CONFIGURE_SDL_SOUND(Angband)

    ENDIF()

ENDIF()


#[[
For Linux Systems the compilation without the libm library
throw undefined references to mathematical functions, this requirement
is only present in Linux Systems.

Wikipedia say: Under Linux and BSD, the mathematical functions
(as declared in <math.h>) are bundled separately in the mathematical
library libm. Therefore, if any of those functions are used, the linker
must be given the directive -lm.

Ref: https://en.wikipedia.org/wiki/C_mathematical_functions#libm
]]
TARGET_LINK_LIBRARIES(Angband PRIVATE $<$<PLATFORM_ID:Linux>:m>)

MESSAGE(STATUS "Moving files needed")

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
