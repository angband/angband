=================
Modifying Angband
=================

Angband is not just a great game in its own right, it is really easy to modify.
Much of the detail of the game is contained in text data files.  These can be
changed using nothing more than a text editor for an immediate change to how
the game works.

These data files are in lib/gamedata.

Each file has a header which describes the lines which make up entries of the
file, and for the most part this will make it clear what needs to be done to
make changes to the files.  Below is brief description of each of the files.

Those who want to change the game more than is allowed just by varying the
data files will need the source code.  Below the data file descriptions is a
brief discussion of where to start on such an endeavour.


The data files
==============

constants.txt
  This file contains game values such as carried item capacity, visual range
  and dungeon level and town dimensions.

object_base.txt
  This file contains the names and common properties of the basic object
  classes - scroll, sword, ring, and so on.  All objects have an object base.
  Each object base is assigned a 'tval' - a numeric index.  The tvals are
  defined in src/list-tvals.h.  While adding new object bases is possible,
  it is unlikely to do much without deeper changes to the game.

object.txt
  This file contains the names, properties and description of all the object
  types that appear in Angband.  New object kinds can easily be added to this
  file, or existing ones edited.  Each object defined by this file has an
  object base, and is also allocated another numeric index called an 'sval'.
  A tval-sval pair completely identifies an object - since the tval and sval
  are saved to savefiles, removing or adding objects is likely to render
  existing save files unusable.

ego_item.txt
  This file contains the names, properties and description of ego items, which
  are magically enhanced weapons and armour.  New ego items can be added or
  removed at will, although removing or changing one with an instance currently
  in the game might cause problems.

artifact.txt
  This file contains the names, properties and description of artifacts, which
  are unique items - only one of each will ever be generated.  If you are
  considering major changes, new artifacts are one of the most visible signs of
  a change of theme.  Regardless, new artifacts are easy and fun to design.

names.txt
  This file contains lists of words which are used to generate names for
  random character names, random artifacts and scrolls.  Again, in the case
  of a change of theme, this is a good way of displaying the new theme.

activation.txt
  Activations are used for artifacts and some regular objects, and could be
  used for ego items (although none currently are).  Some standard artifacts
  from artifact.txt have activations, and random artifacts may have any
  activation from this file chosen for them.  Activations can be made up of
  any effects (see list-effects.h and effect-handler-*something*.c).

flavor.txt
  Items such as potions and wands are assigned a flavor per object kind,
  different in each game.  There need to be at least as many flavors for each
  flavored object base as objects with that base.

monster_base.txt
  Monster bases are the monster equivalent of object bases - classes of monster
  like orc, troll or vampire.  This file contains the properties common across
  all monsters in each of these classes.

monster.txt
  This contains the detail of all monster races, each of which will have its
  monster base properties plus additional ones.  Some monsters are unique, and
  once killed will never reappear.

monster_spell.txt
  All the spells that can be cast by monsters (and are referred to in the
  'spells:' lines in monster.txt) are defined in this file.  As with
  activations, monster spells are built up from effects.

pain.txt
  This file contains the various messages that are given to describe how a
  monster responds to attack.

pit.txt
  Dungeon levels can contain pits - rooms full of a particular selection of
  monsters.  This file defines these selections.  They can also be used, for
  example, to generate partial or complete dungeon levels with themed monsters.

class.txt
  This file completely defines how player classes work, including all details
  of castable spells.  There are some class-specific properties hard-coded,
  which are referred to via the 'flags:' lines, and appear in
  list-player-flags.h.

p_race.txt
  This file defines all player race characteristics.  Race-specific code is
  handled as for classes.

body.txt
  Every player race has a body, which defines what equipment they can use.
  Currently there is only one body, which all races use, but this is easily
  changeable for significant effect.

history.txt
  This file is for creating the player background found on the character
  screen.  If a new race is introduced, a selection of background information
  for it will need to be added.

hints.txt
  This is simply a list of general pieces of advice that shopkeepers will give
  to their customers.

quest.txt
  This file defines the quest monsters (Sauron and Morgoth) and where they
  appear.  This currently can't easily be changed, as there are still
  hard-coded aspects of the quests.

terrain.txt
  This file defines the kind of terrain which can appear in Angband, and its
  properties.  Current terrain can be changed (with possibly large effects),
  but removing it without code changes is likely to break the game.  Adding
  new terrain will have no effect by itself, because there is no mechanism
  for it to appear.

trap.txt
  This defines all traps, door locks and runes.  Actual trap effects appear in
  list-effects.h and effect-handler-*something*.c.

room_template.txt
  This is a list of templates for interesting-shaped rooms which appear in the
  dungeon.  These can easily be changed and new ones added.

vault.txt
  Similar to room_template.txt, this handles vaults, which are very dangerous
  and lucrative rooms.

dungeon_profile.txt
  This file contains fairly technical details about the different types of
  dungeon level which can be generated.  The actual generation routines are in
  gen-cave.c; the information here consists of parameters for generating
  individual levels, and for how often given level types appear.

store.txt
  This details the shop owners and their relative generosity.

blow_effects.txt
  This defines effects to the player caused by monster attacks.  The simplest
  monster attacks just deal damage, but others can affect the player's status,
  stats or inventory.

blow_methods.txt
  This details the different ways monsters can attack (hit, claw, etc.).  It
  affects the messages the player gets, and also whether the blow can stun
  or cut the player.

brand.txt
  This details how weapon brands work.

slay.txt
  This details how weapons can be more effective against certain monsters.

curse.txt
  This file contains all the different curses that can be applied to objects.
  It includes what type of object they can be applied to, random effects they
  can cause, and how they change an object's properties.

object_property.txt
  This file gives details about what properties an object can have (apart from
  basic combat and armor class).  Every property has a code which is used
  in the game to refer to that property in some way. This means it is not
  possible to add new properties to this file and expect to have any effect,
  but it is possible to change how existing properties work.

player_timed.txt
  This file defines some of the properties of timed effects (such as haste and
  confusion) that can apply to the player.  It chiefly contains the messages
  on changes in these effects, and player attributes which prevent the effects.
  To add new timed effects or change the way existing ones operate, you will
  have to alter src/list-player-timed.h and probably other files, and
  re-compile the game.

projection.txt
  This file contains a lot of the defining information about projections -
  effects which can be produced at a distance by player or monsters, and
  affecting player, monsters, objects, and/or terrain.  In particular, this
  file defines details of the effects of elemental attacks (such as fire or
  shards) and the effectiveness of corresponding player resistance.  New
  projections have to be included in src/list-projections.h, and the code to
  implement their effects put in other source files - src/project-obj.c for
  effects on objects, and other similarly-named files.

realm.txt
  This contains a small amount of information about the two current magic
  realms.

summon.txt
  This contains definitions for the types of monsters that can be summoned.
  Adding a new summon type is not yet possible, because the summon spells are
  hard-coded in src/list-mon-spells.h.

ui_entry.txt
  Defines entries that will be displayed in the second part of the character
  sheet and in the knowledge menu's equipable comparison.  You can modify
  properties in object_property.txt and project_property.txt to bind them to
  those entries.  The intent is to make it possible to add or remove a property
  without having to update ui-player.c or ui-equipcmp.c in addition to the
  changes necessary to have that property affect core gameplay.

ui_entry_base.txt
  Provides templates for use by ui_entry.txt.

ui_entry_renderer.txt
  Defines techniques, referenced in ui_entry.txt, for rendering a property in
  the character sheet or equipable comparison.  While it is possible to add
  something that simply uses different palettes of symbols or colors than
  one of the current renderers, the basic rendering techniques are hard-coded
  in list-ui-entry-renderers.h.

Making Graphical Tilesets
=========================

You can make new graphical tilesets for Angband or customize existing ones. In
this section we'll dive into how tilesets are defined and describe how to set
one up from scratch. First, we'll enumerate the steps required and then we'll
break down each step in detail.

1. Create a directory to contain the tileset's files: (ex. ``lib/tiles/mytileset``)
2. Register the tileset in ``lib/tiles/list.txt``
3. Create an empty bitmap image large enough to hold your tileset
4. Store the empty bitmap image in your tileset folder
5. Author one or more ``.prf`` files to inform Angband how to use your tileset
6. Create a Makefile in your tileset folder

First you need to create a directory to contain your tileset's files. Put the
directory in lib/tiles and choose a name for the directory that is lower-case
and generally matches the naming convention of the other tilesets you see
there. Once the directory has been created, the next step is to decide how big
the tiles will be in pixels and then create a blank PNG image large enough to
hold all of the tiles (be sure to enable alpha transparency). As an example,
Shockbolt's tileset uses 64x64 pixel tiles. It also uses the special alpha
blending flag so it can use double-height tiles (64x128) for large or tall
monsters. Its dimensions are 8192x2048 but the tileset is not completely
full. More tiles can be added without increasing the size of the image as new
objects are added to future releases of Angband. This should be kept in mind as
packing your tileset into the smallest possible image size may not be the most
maintainable solution. Be sure to name the image file after the tile size, for
example 64x64.png. Use the base size even if you are enabling double-height
tiles.

The only file you'll need to edit outside of your tileset's directory is
lib/tiles/list.txt. list.txt contains a registry of which tilesets to load as
well as some information about the size of the tiles and any special flags to
set. The format of the file is documented in list.txt's header. Specifically,
you will be defining the name of the tileset, which directory contains the
tileset's files, how big the tiles are in pixels (i.e. 64x64), the name of the
main preference file for the tileset and some additional flags which have to do
with alpha blending. Not all tilesets need to set extra flags.

Now that the basic setup is complete you need to tell Angband how to interpret
your tileset image. You need to map each tile in your image to a specific
element in the game so that Angband knows which tiles to show for which ASCII
characters. This process can be done incrementally because Angband will
continue to show the default character symbols in-game for objects that have
not yet been mapped. This is especially helpful for verifying that your tileset
has been setup correctly before beginning to map things out in earnest. It also
means that if new objects are added to the game that you have not mapped into
your tileset, the game will still be playable with your tileset, albeit the
displayed ASCII character may appear incongruous with your styling. Mapping
tiles to game elements is done in text files called preference files which have
the extension '.prf'.

The first thing to understand about mapping game elements in preference files
is that everything that can be displayed in the game has a name, or in the case
of flavors, an ID number. The names for each type of thing can be referenced
from the data files as mentioned above. The table below is a quick reference
for where to find names of things and how to form IDs correctly to reference
them.

============= ================== ====================
Type          Data File          Example
============= ================== ====================
Terrain       terrain.txt        ``feat:open floor``
Trap          trap.txt           ``trap:pit``
Object        object.txt         ``object:light``
Monster       monster.txt        ``monster:Kobold``
Spell Effect  monster_spell.txt  ``GF:METEOR``
Player        <see below>        ``monster:<player>``
============= ================== ====================

Player pictures are referenced differently than other types of objects. They
use a special query syntax that checks to see what kind class the player is as
well as the gender in order to determine which picture to show. The query to
select which tile to show for a female elf ranger would be::

  ?:[AND [EQU $CLASS Ranger] [EQU $RACE Elf]  [EQU $GENDER Female] ]

Here, the query is checking to see if the player is a female Half-Elf and would
use the assignment on the next line of the preference file only if this is
true.

Some types of objects such as terrain can use different tiles based on their
state. In the case of terrain, the terrain can have different images for when
it is lit by a torch, or dark. these are selected by appending another colon
and a specifier to the name. For example, this would be the name of a torch-lit
up staircase::

  feat:up staircase:torch

It is possible to specify the same tile be used for all possible states of a
terrain feature by using an asterisk. This example identifies any unknown
terrain tile (a tile the player hasn't lit or otherwise seen yet)::

  feat:unknown grid:*

Given the full name of an object the last thing to do is to specify which tile
from the tileset to use. Tile locations are given in a coordinate system using
pairs of hexadecimal numbers. The coordinates start from 0x80:0x80 and
increment from there. The pairs translate directly to the top and left most
pixel of the corresponding tile from the graphics file, so the top left pixel
of the first tile on the top left of the graphics file would be specified as
0x80:0x80 (the pixel at x:0 y:0). The next tile immediately to the right of the
that one would be 0x80:0x81. The tilesheet is sliced into rows and columns
based on the tile size you specified in list.txt. So given a tile size of 64x64
pixels, the tile at 0x80:0x81 would be located in the graphics file at pixel
x:64 y:0. Remember, the coordinates in the preference files are in hexadecimal,
so the next number after 0x89 would be 0x8A. The next number after 0x8F would
be 0x90 and so on. To map an object to your tileset you will add one complete
line to the file per object. This example maps the tile at 0x81:0x81 to the
terrain feature 'quartz vein' when the quartz vein is lit by torch light::

  feat:closed door:quartz vein:torch:0x81:0x81

Before going any further, it is advisable to map a single object in your
preference file, then start the game up, select your tileset and make sure you
see your mapped tile in game. If this worked, then you are ready to design and
map the rest of your tiles. A quick example would be to map a tile for your
home in the town to the first tile position in your graphics file::

  feat:Home:*:0x80:0x80

It's possible to have more than one preference file by using a sort of include
syntax that causes other preference files referenced from your main preference
file to also be read. It is also possible to place comments in your preference
files to help you keep track of where different kinds of objects are
mapped. Any text on a line after a ``#`` symbol is ignored. Shockbolt's tiles
make great use of this and define a well organized set of mappings using three
files with comments for each logical section of objects to be mapped::

  # This is a comment
  %:other-stuff.prf  # Load another preference file

The last step to take is to make sure your tileset will be packaged with
Angband when it is compiled for distribution and that it can be installed
alongside the other tilesets. to do this you will need to add a file called
'Makefile' to your tileset directory. Copy and paste an existing Makefile from
one of the other tileset directories and update the DATA and PACKAGE lines to
match the filenames you chose for your tileset.

Once you have a working tileset and functional understanding of how tilesets
are managed and organized, it would be a good idea to study Shockbolt's tileset
and follow the examples there in order to produce a high-quality tileset that
you will be proud to share with others.

Larger changes
==============

If changing data files is not enough for you, you will need to change actual
game code and recompile it.  The first place to look is in the compiled data
files, some of which have already been mentioned:

=====================  =======================  ===========================
list-dun-profiles.h    list-mon-spells.h        list-projections.h
list-effects.h         list-mon-temp-flags.h    list-randart-properties.h
list-elements.h        list-mon-timed.h         list-rooms.h
list-equip-slots.h     list-object-flags.h      list-room-flags.h
list-history-types.h   list-object-modifiers.h  list-square-flags.h
list-ignore-types.h    list-options.h           list-stats.h
list-kind-flags.h      list-origins.h           list-terrain-flags.h
list-message.h         list-parser-errors.h     list-trap-flags.h
list-mon-message.h     list-player-flags.h      list-tvals.h
list-mon-race-flags.h  list-player-timed.h      list-ui-entry-renderers.h
=====================  =======================  ===========================

Beyond this, you will have to have some knowledge of the C programming
language, and can start making changes to the way the game runs or appears.
Many people have done this - there are over 100 variants of Angband:
http://angbandplus.github.io/AngbandPlus/
Should you get to this point, the best thing to do is to discuss your ideas on
the Angband forums at http://angband.oook.cz.  The people there are typically
keen to hear new ideas and ways to play.
