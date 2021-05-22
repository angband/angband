CMAKE_MINIMUM_REQUIRED(VERSION 3.5 FATAL_ERROR)
PROJECT(Angband LANGUAGES C)
SET(OUR_EXECUTABLE_NAME ${PROJECT_NAME})

IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    MESSAGE(FATAL_ERROR "No support for CMake on Windows")
ENDIF()

OPTION(SUPPORT_GCU_FRONTEND "Support for GCU front end." OFF)
OPTION(SUPPORT_NCURSES_FRONTEND "Synonym for SUPPORT_GCU_FRONTEND for backwards compatibility." OFF)
OPTION(SUPPORT_SDL_FRONTEND "Support for SDL front end." OFF)
OPTION(SUPPORT_SDL2_FRONTEND "Support for SDL2 front end." OFF)
OPTION(SUPPORT_X11_FRONTEND "Support for X11 Frontend." OFF)
OPTION(SUPPORT_SDL_SOUND "Support for sound with SDL." OFF)
OPTION(SUPPORT_SDL2_SOUND "Support for sound with SDL2." OFF)
OPTION(SUPPORT_SPOIL_FRONTEND "Support for spoiler front end." ON)
OPTION(SUPPORT_STATS_FRONTEND "Support for statistics front end; requires sqlite3 development library." OFF)
OPTION(SUPPORT_TEST_FRONTEND "Support for test front end." OFF)
OPTION(SUPPORT_STATS_BACKEND "Enable backend support for statistics and related debugging commands.  Implied by SUPPORT_STATS_FRONTEND." OFF)

IF ((SUPPORT_NCURSES_FRONTEND) AND (NOT SUPPORT_GCU_FRONTEND))
    SET(SUPPORT_GCU_FRONTEND ON)
ENDIF()
IF((SUPPORT_STATS_FRONTEND) AND (NOT SUPPORT_STATS_BACKEND))
    SET(SUPPORT_STATS_BACKEND ON)
ENDIF()
# If none of the graphical front ends will be configured, configure the X11 one.
IF ((NOT SUPPORT_GCU_FRONTEND) AND (NOT SUPPORT_SDL_FRONTEND) AND (NOT SUPPORT_SDL2_FRONTEND) AND (NOT SUPPORT_X11_FRONTEND))
    SET(SUPPORT_X11_FRONTEND ON)
ENDIF()
# Can not have support for SDL and SDL2 at the same time.
IF (SUPPORT_SDL2_FRONTEND OR SUPPORT_SDL2_SOUND)
    IF (SUPPORT_SDL_FRONTEND OR SUPPORT_SDL_SOUND)
        MESSAGE(FATAL_ERROR "Can not configure with support for both SDL and SDL2")
    ENDIF()
ENDIF()

ADD_LIBRARY(OurCoreLib OBJECT
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
        src/cmd-spoil.c
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
        src/ui-spoil.c
        src/ui-store.c
        src/ui-target.c
        src/ui-term.c
        src/ui-visuals.c
        src/ui-wizard.c
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

SET_TARGET_PROPERTIES(OurCoreLib PROPERTIES C_STANDARD 99)
SET(ANGBAND_CORE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src")
SET(ANGBAND_CORE_LINK_LIBRARIES "")
FIND_LIBRARY(MATH_LIBRARY m)
IF(MATH_LIBRARY)
    LIST(APPEND ANGBAND_CORE_LINK_LIBRARIES ${MATH_LIBRARY})
ENDIF()

IF(SUPPORT_SDL_SOUND OR SUPPORT_SDL2_SOUND)
    ADD_LIBRARY(OurSoundSupportLib OBJECT
            src/snd-sdl.c
    )
    SET_TARGET_PROPERTIES(OurSoundSupportLib PROPERTIES C_STANDARD 99)
    SET(SOUND_SUPPORT_LIB OurSoundSupportLib)
ELSE()
    SET(SOUND_SUPPORT_LIB "")
ENDIF()

ADD_EXECUTABLE(OurExecutable
        $<TARGET_OBJECTS:OurCoreLib>
        $<$<BOOL:${SOUND_SUPPORT_LIB}>:$<TARGET_OBJECTS:${SOUND_SUPPORT_LIB}>>
        $<$<BOOL:${SUPPORT_GCU_FRONTEND}>:src/main-gcu.c>
        $<$<BOOL:${SUPPORT_SDL_FRONTEND}>:src/main-sdl.c>
        $<$<BOOL:${SUPPORT_SDL2_FRONTEND}>:src/main-sdl2.c>
        $<$<BOOL:${SUPPORT_X11_FRONTEND}>:src/main-x11.c>
        $<$<BOOL:${SUPPORT_SPOIL_FRONTEND}>:src/main-spoil.c>
        $<$<BOOL:${SUPPORT_STATS_FRONTEND}>:src/main-stats.c>
        $<$<BOOL:${SUPPORT_STATS_FRONTEND}>:src/stats/db.c>
        $<$<BOOL:${SUPPORT_TEST_FRONTEND}>:src/main-test.c>
        src/main.c
)

SET_TARGET_PROPERTIES(OurExecutable PROPERTIES
    C_STANDARD 99
    OUTPUT_NAME ${OUR_EXECUTABLE_NAME}
)
TARGET_INCLUDE_DIRECTORIES(OurExecutable PRIVATE
    ${ANGBAND_CORE_INCLUDE_DIRS}
)
TARGET_LINK_LIBRARIES(OurExecutable PRIVATE ${ANGBAND_CORE_LINK_LIBRARIES})

IF (SUPPORT_X11_FRONTEND)
    INCLUDE(src/cmake/macros/X11_Frontend.cmake)
    CONFIGURE_X11_FRONTEND(OurExecutable)
ENDIF()
IF(SUPPORT_GCU_FRONTEND)
    INCLUDE(src/cmake/macros/GCU_Frontend.cmake)
    CONFIGURE_GCU_FRONTEND(OurExecutable)
ENDIF()
IF(SUPPORT_SDL_FRONTEND)
    INCLUDE(src/cmake/macros/SDL_Frontend.cmake)
    CONFIGURE_SDL_FRONTEND(OurExecutable)
ELSEIF(SUPPORT_SDL2_FRONTEND)
    INCLUDE(src/cmake/macros/SDL2_Frontend.cmake)
    CONFIGURE_SDL2_FRONTEND(OurExecutable)
ENDIF()

IF(SUPPORT_SDL_SOUND)
    INCLUDE(src/cmake/macros/SDL_Sound.cmake)
    CONFIGURE_SDL_SOUND(OurExecutable NO)
    CONFIGURE_SDL_SOUND(OurCoreLib YES)
    CONFIGURE_SDL_SOUND(${SOUND_SUPPORT_LIB} NO)
ENDIF()

IF (SUPPORT_SDL2_SOUND)
    INCLUDE(src/cmake/macros/SDL2_Sound.cmake)
    CONFIGURE_SDL2_SOUND(OurExecutable NO)
    CONFIGURE_SDL2_SOUND(OurCoreLib YES)
    CONFIGURE_SDL2_SOUND(${SOUND_SUPPORT_LIB} NO)
ENDIF()

IF (SUPPORT_SPOIL_FRONTEND)

    INCLUDE(src/cmake/macros/SPOIL_Frontend.cmake)
    CONFIGURE_SPOIL_FRONTEND(OurExecutable)

ENDIF()

IF(SUPPORT_STATS_FRONTEND)
    INCLUDE(src/cmake/macros/STATS_Frontend.cmake)
    CONFIGURE_STATS_FRONTEND(OurExecutable)
ENDIF()

IF(SUPPORT_STATS_BACKEND)
    INCLUDE(src/cmake/macros/STATS_Backend.cmake)
    CONFIGURE_STATS_BACKEND(OurExecutable)
    CONFIGURE_STATS_BACKEND(OurCoreLib)
ENDIF()

IF(SUPPORT_TEST_FRONTEND)
    INCLUDE(src/cmake/macros/TEST_Frontend.cmake)
    CONFIGURE_TEST_FRONTEND(OurExecutable)
ENDIF()

# Set the build ID.  Assumes a Unix-like environment.
EXECUTE_PROCESS(COMMAND "${CMAKE_SOURCE_DIR}/scripts/version.sh" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/scripts" OUTPUT_VARIABLE ANGBAND_BUILD_ID)
IF (NOT (ANGBAND_BUILD_ID MATCHES "^[ \t\n\r]*$"))

     SET(ANGBAND_BUILD_ID_OPTION "BUILD_ID=${ANGBAND_BUILD_ID}")
ELSE()
     SET(ANGBAND_BUILD_ID_OPTION "")
ENDIF()
TARGET_COMPILE_DEFINITIONS(OurExecutable PRIVATE "${ANGBAND_BUILD_ID_OPTION}")
TARGET_COMPILE_DEFINITIONS(OurCoreLib PRIVATE "${ANGBAND_BUILD_ID_OPTION}")

MESSAGE(STATUS "Copying needed files")

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# Set up tests.
# First copy some scripts and, as necessary, test case data from the source
# tree.
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/run-tests
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/unittests/)
IF(SUPPORT_TEST_FRONTEND)
    FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
ENDIF()
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unittests/bin)
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/run-tests
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-test
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tests)

# Set up target for running all tests.  Those (from tests in the source tree)
# using the test front end will be run directly.  The unit tests will be
# handled through a dependency.
ADD_CUSTOM_TARGET(alltests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run-tests ./${OUR_EXECUTABLE_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
ADD_CUSTOM_TARGET(allunittests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unittests/run-tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
IF(SUPPORT_TEST_FRONTEND)
    ADD_DEPENDENCIES(alltests OurExecutable)
ENDIF()
ADD_DEPENDENCIES(alltests allunittests)

# Set up an object library with the shared code for unit tests.
ADD_LIBRARY(OurUnitTestLib OBJECT EXCLUDE_FROM_ALL
        src/tests/test-utils.c
        src/tests/unit-test.c
)
SET_TARGET_PROPERTIES(OurUnitTestLib PROPERTIES C_STANDARD 99)
SET(ANGBAND_UNIT_TEST_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/tests")
TARGET_INCLUDE_DIRECTORIES(OurUnitTestLib PRIVATE
    ${ANGBAND_CORE_INCLUDE_DIRS}
)
TARGET_COMPILE_DEFINITIONS(OurUnitTestLib PRIVATE "${ANGBAND_BUILD_ID_OPTION}")

# Set up targets for exercising the unit tests (from src/tests in the source
# tree).
EXECUTE_PROCESS(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/list-tests" -d
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/tests"
    OUTPUT_VARIABLE ANGBAND_TEST_DIRS
)
STRING(REGEX REPLACE "[ \t\n\r]" ";" ANGBAND_TEST_DIR_LIST ${ANGBAND_TEST_DIRS})
FOREACH(ANGBAND_TEST_DIR ${ANGBAND_TEST_DIR_LIST})
    FILE(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/unittests/bin/${ANGBAND_TEST_DIR}")
ENDFOREACH()
EXECUTE_PROCESS(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/list-tests"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/tests"
    OUTPUT_VARIABLE ANGBAND_TEST_CASES
)
STRING(REGEX REPLACE "[ \t\n\r]" ";" ANGBAND_TEST_CASE_LIST ${ANGBAND_TEST_CASES})
SET(ANGBAND_TEST_CASE_INDEX 1)
FOREACH(ANGBAND_TEST_CASE ${ANGBAND_TEST_CASE_LIST})
    SET(ANGBAND_TEST_CASE_NAME "unittest${ANGBAND_TEST_CASE_INDEX}")
    SET(ANGBAND_TEST_CASE_PATH "unittests/bin/${ANGBAND_TEST_CASE}")
    ADD_EXECUTABLE(${ANGBAND_TEST_CASE_NAME} EXCLUDE_FROM_ALL
            "src/tests/${ANGBAND_TEST_CASE}.c"
            $<TARGET_OBJECTS:OurUnitTestLib>
            $<TARGET_OBJECTS:OurCoreLib>
            $<$<BOOL:${SOUND_SUPPORT_LIB}>:$<TARGET_OBJECTS:${SOUND_SUPPORT_LIB}>>
    )
    SET_TARGET_PROPERTIES(${ANGBAND_TEST_CASE_NAME} PROPERTIES
        C_STANDARD 99 OUTPUT_NAME "${ANGBAND_TEST_CASE_PATH}")
    TARGET_INCLUDE_DIRECTORIES(${ANGBAND_TEST_CASE_NAME} PRIVATE
        ${ANGBAND_CORE_INCLUDE_DIRS}
        ${ANGBAND_UNIT_TEST_INCLUDE_DIRS}
    )
    TARGET_COMPILE_DEFINITIONS(${ANGBAND_TEST_CASE_NAME} PRIVATE "${ANGBAND_BUILD_ID_OPTION}")
    TARGET_LINK_LIBRARIES(${ANGBAND_TEST_CASE_NAME} PRIVATE
        ${ANGBAND_CORE_LINK_LIBRARIES}
    )
    IF(SUPPORT_SDL_SOUND)
        CONFIGURE_SDL_SOUND(${ANGBAND_TEST_CASE_NAME} NO)
    ENDIF()
    IF(SUPPORT_SDL2_SOUND)
        CONFIGURE_SDL2_SOUND(${ANGBAND_TEST_CASE_NAME} NO)
    ENDIF()
    ADD_DEPENDENCIES(allunittests ${ANGBAND_TEST_CASE_NAME})
    MATH(EXPR ANGBAND_TEST_CASE_INDEX "${ANGBAND_TEST_CASE_INDEX} + 1")
ENDFOREACH()
