============
How It Works
============

This document describes how Angband actually *works* at a high level. Individual
sections referenced from the TOC are marked with anchors in square brackets to
make grepping for them easier.

.. contents:: Contents
   :local:


The Game
========

As you probably know if you're reading this, Angband is a roguelike game set in
a high-fantasy universe. The game world is made up of levels, numbered from zero
("the town") to some maximum depth. Levels are increasingly dangerous the deeper
they are into the dungeon. Levels are filled with monsters, traps, and objects.
Monsters move and act on their own, traps react to creatures entering their
square, and objects are inert unless used by a creature. The objective of the
game is to find Morgoth at depth 100 and kill him.

Data Structures
===============

There are three important top-level data structures in Angband: the 'chunk', the
player, and the static data tables.

The Chunk
---------
A chunk represents an area of dungeon, and contains everything inside it; this
includes any monsters, objects, or traps inside the bounds of that chunk. A
chunk also keeps a map of the terrain in its area. For unpleasant historical
reasons, all monsters/objects/traps in a chunk are stored in arrays and usually
referred to by index; each square of a chunk knows the indexes (if any) of
monsters/objects/traps contained in it. A chunk also stores AI pathfinding data
for its contained area. All data in the 'current' chunk is lost when leaving the
level.

The Player
----------

The player is a global object containing information about, well, the player.
All the information in the player is level-independent. This structure contains
stats, any current effects, hunger status, sex/race/class, the player's
inventory, and a grab-bag of other information. Although there is a global
player object, many functions instead take a player object explicitly to make
them easier to test.

The Static Data
---------------

Angband's static data - player and monster races, object types, artifacts, et
cetera - is loaded from the `gamedata Files`_. Once loaded, this
data is stored in global tables, sometimes referred to as the 'info arrays'.
These arrays are generally declared in the header files of the code that uses
them most, but they are mostly initialized by the edit-file code. The sizes of
these arrays are stored in a 'maxima' structure, called z_info.

The Z Layer
===========

The lowest-level code in Angband is the "Z" layer, which provides
platform-independent abstractions and generic data structures. Currently, the Z
layer provides:

=================   ========================================
``z-bitflag``       Densely-packed bit flag arrays
``z-color``         Colors
``z-debug``         Debugging annotations
``z-dice``          Dice expressions
``z-expression``    Mathematical expressions
``z-file``          File I/O
``z-form``          String formatting
``z-msg``           Rich messages
``z-msg``           Message buffering -lis
``z-quark``         String interning
``z-queue``         Queues
``z-rand``          Randomness
``z-set``           Sets
``z-textblock``     Wrapped text
``z-type``          Basic types
``z-util``          Random utility macros
``z-virt``          malloc() wrappers
=================   ========================================

Code in the Z layer may not depend on files outside the Z layer.

Key Abstractions
================

Certain game-specific abstractions are important and widely used in Angband to
glue the UI code to the game engine. These are the command queue, which sends
player commands to the game engine, and events, which indicate to the UI that
the state of the game changed.

The command queue
-----------------

TBD

Events
------

TBD

Files
=====

Angband uses three types of files for storing data: gamedata files, which contain
the game's static data, pref files, which contain UI settings,
and save files, which contain the state of a game in progress.

Gamedata Files
--------------

Gamedata files use a line-oriented format where fields are separated by colons. The
parser for this format is in ``parser.h``. These files are mostly loaded at
initialization time (see `init.c - init_angband`_) and used to fill in the static data
arrays (see `The Static Data`_).

Pref Files
----------
TBD

Savefiles
----------

Currently, a savefile is a series of concatenated blocks. Each block has a name
describing what type it is and a version tag. The version tag allows for old
savefiles to be loaded, although the load/save code will only write new
savefiles. Numbers in savefiles are stored in little-endian byte order and
strings are stored null-terminated.

Control Flow
============

The flow of control through Angband is complicated and can be very non-obvious
due to overuse of global variables as special-behavior hooks. That said, this
section gives a high-level overview of the control flow of a game session.

Startup
-------

Execution begins in main.c, which runs frontend-independent initialization code,
then continues in the appropriate ``main-*.c`` file for the current frontend. After
the game engine is initialized, the player is loaded (or generated) and gameplay
begins.

``main.c`` and ``main-*.c``
~~~~~~~~~~~~~~~~~~~~~~~~~~~
main.c's ``main()`` is the entry point for Angband execution except on Windows,
where main-win.c's ``WinMain()`` is used, on Nintendo DS, where a special
``main()`` in main-nds.c is used, and on OS X where main-cocoa.m's ``main()``
is used. The ``main()`` function is responsible for dropping permissions if
Angband is running setuid, parsing command line arguments, then finding a
frontend to use and initializing it. Once ``main()`` finds a frontend, it sets
up signal handlers, sets up the display, and calls `init.c - init_angband`_,
which loads all the `gamedata files`_ and initializes other static data used
by the game.

init.c - ``init_angband``
~~~~~~~~~~~~~~~~~~~~~~~~~
The init_angband() function in init.c is responsible for loading and setting up
static data needed by the game engine. Inside init.c, there is a list of 'init
modules' that have startup-time static data they need to initialize, these are
registered in an array of module pointers in init.c, and init_angband() calls
their initialization hooks before doing any other work.  Finally it sets up the
RNG.

ui-init.c - ``textui_init``
~~~~~~~~~~~~~~~~~~~~~~~~~~~
The textui_init() function then loads the top-level pref file (see
`pref files`_), initializes the command queue (see `the command queue`_),
and configures subwindows.

ui-prefs.c - ``process_pref_file``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The process_pref_file() function in ui-prefs.c is responsible for loading user
pref files, which can live at multiple paths. User preference files override
default preference files. See `pref files`_ for more details.

ui-game.c - ``play_game``
~~~~~~~~~~~~~~~~~~~~~~~~~
This function calls start_game() to load a saved game if there is a valid save
(see `savefiles`_) or birth a new character if not.  It then asks for a command
from the player, and then runs the game main loop (see
`game-world.c - the game main loop`_), over and over until the character dies
or the player quits

Gameplay
--------
Once the simulation is set up, the game main loop in `ui-game.c - play_game`_
is responsible for stepping the simulation.

game-world.c - the game main loop
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The main loop of the game, run_game_loop() is repeatedly called inside
play_game(). Each iteration of the main loop is one "turn" in Angband parlance,
or one step of the simulator. During each turn:

* All monsters with more energy than the player act
* The player acts
* All other monsters act
* The UI updates
* The world acts
* End-of-turn housekeeping is done

mon-move.c - process_monsters()
*********************************

In Angband, creatures act in order of "energy", which roughly determines how
many actions they can take per step through the simulation. The
process_monsters() function in mon-move.c is responsible for walking through
the list of all monsters in the current chunk (see `the chunk`_) and having each
monster act by calling process_monster(), which implements the highest level AI
for monsters.

game-world.c - process_player()
*******************************

The process_player() function allows the player to act repeatedly until they do
something that uses energy. Commands like looking around or inscribing items do
not use energy; movement, attacking, casting spells, using items, and so on do.
The rule of thumb is that a command that does not alter game engine state does
not use energy, because it does not represent an action the character in the
simulation is doing. The guts of the process_player() function are actually
handled by process_command() in cmd-core.c, which looks up commands in the
game_cmds table in that file.

Keeping the UI up to date
*************************

Four related horribly-named functions in player-calcs.h are responsible for
keeping the UI in sync with the simulated character's state:

==================  ===============================================================
``notice_stuff()``  which deals with pack combining and dropping ignored items;
``update_stuff()``  which recalculates derived bonuses, AI data, vision, seen
                    monsters, and other things based on the flags in
                    ``player->upkeep->update``;
``redraw_stuff()``  which signals the UI to redraw changed sections of the
                    game state;
``handle_stuff()``  which calls update_stuff() and redraw_stuff() if needed.
==================  ===============================================================

These functions are called during every game loop, after the player and all
monsters have acted.

game-world.c - process_world()
******************************

The process_world() function only runs every 10 turns. It is responsible for the
day/night transition in town, restocking the stores, generating new creatures
over time, dealing poison/cut damage, applying hunger, regeneration, ticking
down timed effects, consuming light fuel, and applying a litany of spell effects
that happen 'at random' from the player's point of view.

Dungeon Generation
------------------

prepare_next_level() in generate.c controls the process of generating or loading
a level.  To signal that run_game_loop() in game-world.c should call
prepare_next_level(), game logic calls dungeon_change_level() in player-util.c
to set the necessary data in the player structure.  When a level change happens
by traversing a staircase, some other data in the player structure is set to
indicate what should be done to connect stairs.  That doesn't happen in
dungeon_change_level() and is instead set directly, currently in do_cmd_go_up()
and do_cmd_go_down() in cmd-cave.c.

With the default for non-persistent levels, loading only happens when
returning to the town or when returning from a single combat arena.  The code
and global data for handling stored levels is in gen-chunk.c.

When a new level is needed, prepare_next_level() calls cave_generate(), also in
generate.c.  That initializes a global bit of state, a dun_data structure called
dun declared in generate.h, for passing a lot of the details needed when
generating a level.  It then selects a level profile via choose_profile() in
generate.c.  The level profile controls the layout of the level.  The available
level profiles are those listed in list-dun-profiles.h and several aspects of
each profile are configured at runtime from the contents of
lib/gamedata/dungeon_profile.txt.  With a profile selected, cave_generate()
uses the profile's builder function pointer to attempt to layout the new level.
Those function pointers are initialized when list-dun-profiles.h is included
in generate.c.  The level layout functions all have names with the name of
the profile followed by *_gen*, classic_gen() for classic levels as an
example.  Those functions are defined in gen-cave.c.

Three of the level layout functions, classic_gen(), modified_gen(), and
moria_gen() follow the same basic procedure.  They divide the level into a
grid of rectangular blocks where, in general, each block can only contain
one room though a room could occupy many blocks.  They then try to randomly
place rooms in those blocks until some criteria is met.  Room selection is
configurable from lib/gamedata/dungeon_profile.txt and uses the predefined
room types listed in list-rooms.h.  When building a room, those level layout
functions use the convenience function, room_build() from gen-room.c.  That, in
turn, calls the appropriate function to build the type of room chosen.  The
names of the room building functions have *build_* followed by the name of the
room type, build_simple() for instance.  Those functions are defined in
gen-room.c.  Once the rooms are built, there's an initial pass to connect them
with corridors.  That happens in gen-cave.c's do_traditional_tunneling().
A second pass, to try and ensure connectedness though vault areas can disrupt
that, is then done with ensure_connectedness().  At that point, most other
features (mineral veins, staircases, objects, and monsters) are added.  Some
features will have already been added through some of the types of rooms.

The other layout functions are more of a grab bag.  They are all in gen-cave.c.
Many of them have portions that are caverns or labyrinths.  Those are generated
using cavern_chunk() or labyrinth_chunk(), respectively, in gen-cave.c.

Monster AI
----------

TBD


Stats
-----

The stats generation code aims to make it easy to analyze object generation,
monster generation, and other Angband processes suitable for Monte Carlo
simulation. The supplied perl script (run-stats) repeatedly invokes the 
angband executable with the stats pseudo-visual module. Each call initializes
a player, walks her down the dungeon, and, for each dungeon level between one
and ninety-nine, kills all the monsters on the generated map and dumps 
information about the monsters and objects therein. The perl script then
collects this information and outputs statistics about it as desired.

