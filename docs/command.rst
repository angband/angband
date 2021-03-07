====================
Command Descriptions
====================

The following command descriptions are listed as the command name plus the
default key to use it. For those who prefer the original "roguelike"
keyset, the name and key of the roguelike command is also shown if it is
different. Then comes a brief description of the command, including
information about alternative methods of specifying the command in each
keyset, when needed.

Some commands use the "repeat count" to automatically repeat the command
several times, while others use the "repeat count" to specify a "quantity"
for the command, and still others use it as an "argument" of some kind.

Most commands take no "energy" to perform, while other commands only take
energy when they cause the world to change in some way. For example,
attempting to read a scroll while blind does not use any energy.

The following command is very useful for beginners,

Command lists ('Enter')
  This brings up a little window in the middle of the screen, in which you
  can select what command you would like to use by browsing.  If you wish
  to begin playing immediately, you can use this option to navigate the 
  commands and refer to this guide when you need more details about 
  specific commands.

Inventory Commands
==================

Inventory list (``i``)
  Displays a list of objects being carried but not equipped. You can carry
  up to 23 different items, not counting those in your equipment. Often,
  many identical objects can be "stacked" into a "pile" which will count as
  a single item.  Each object has a weight, and if you carry more
  objects than your strength permits, you will begin to slow down. The
  amount of weight you can still carry without being overencumbered, or the
  amount of extra weight you are currently carrying is displayed at the top
  of the screen.
 
Equipment list (``e``)
  Use this command to display a list of the objects currently being used by
  your character. The standard body (which all races currently have) has
  12 slots for equipment. Every equipment slot corresponds to a different
  location on the body, and each of which may contain only one object at
  a time, and each of which may only contain objects of the proper "type".
  For the standard body these are WEAPON (weapon), BOW (missile launcher),
  RING (ring) (two of these), AMULET (amulet), LIGHT (light source),
  BODY_ARMOR (armor), CLOAK (cloak), SHIELD (shield), HAT (helmet),
  GLOVES (gloves), BOOTS (boots). You must be wielding/wearing certain
  objects to take advantage of their special powers.

Quiver list (``|``)
  Missiles that you carry will automatically be put in your quiver.  The
  quiver has 10 slots; it also takes up inventory space, so every 40 
  missiles will reduce your number of inventory slots by 1.

Drop an item (``d``)
  This drops an item from your inventory or equipment onto the dungeon
  floor. If the floor spot you are standing on already has an object in it,
  Angband will attempt to drop the item onto an adjacent space.  Doors and
  traps are considered objects for the purpose of determining if the space 
  is occupied. This command may take a quantity, and takes some energy.

Ignore an item (``k``) or Ignore an item ('^D')
  This ignores an item in your inventory or on the dungeon floor. If the
  selected pile contains multiple objects, you may specify a quantity. When
  ignored, the game will sometimes prompt you whether to ignore only this
  item or all others like it.  If the second option is chosen, all similar
  items on the floor and in your inventory will be ignored.  To view all
  items regardless of whether they are ignored, you can use ``K`` to
  toggle the ignore setting on and off.

Wear/Wield equipment (``w``)
  To wear or wield an object in your inventory, use this command. Since
  only one object can be in each slot at a time, if you wear or wield an
  item into a slot which is already occupied, the old item will be first be
  taken off, and may in fact be dropped if there is no room for it in your
  inventory. Wielding ammunition will add it to an empty slot in your
  quiver and prompt you to replace a type of ammunition if your quiver is
  already full. This command takes some energy.

Take off equipment (``t``) or Take off equipment (``T``)
  Use this command to take off a piece of equipment and return it to your
  inventory. Occasionally, you will run into a cursed item which cannot be
  removed. These items normally penalize you in some way and cannot be
  taken off until the curse is removed. If there is no room in your
  inventory for the item, your pack will overflow and you will drop the
  item after taking it off. You may also remove ammunition from your quiver
  with this command. This command takes some energy.

Movement Commands
=================

Moving (arrow keys, number keys) or (arrow keys, number keys, 'yuhjklbn')
  This causes you to move one step in a given direction. If the square you
  wish to move into is occupied by a monster, you will attack it. If the
  square is occupied by a door or a trap you may attempt to open or disarm
  it if the appropriate option is set. Preceding this command with CTRL
  will cause you to attack in the appropriate direction, but will not move
  your character if no monster is there. These commands take some energy.

Walk (``W``)
  The walk command lets you willingly walk into a trap or a closed door,
  without trying to open or disarm it. This command may take a count,
  requires a direction, and takes some energy.

Run (``.``) or Run (``,``)
  This command will move in the given direction, following any bends in the
  corridor, until you either have to make a "choice" between two directions
  or you are disturbed. You can configure what will disturb you by setting
  the disturbance options. You may also use shift plus the "roguelike"
  direction keys (roguelike keyset), or shift plus the "original" direction
  keys on the keypad (both keysets, some machines) to run in a direction.
  This command may take an argument, requires a direction, and takes some
  energy.

Go up staircase (``<``)
  Climbs up an up staircase you are standing on. There is always at least
  one staircase going up on every level except for the town level (this
  doesn't mean it's easy to find). Going up a staircase will take you to a
  new dungeon level unless you are at 50 feet (dungeon level 1), in which
  case you will return to the town level. Note that whenever you leave a
  level (not the town), you will never find it again. This means that for
  all intents and purposes, any objects on that level are destroyed. This
  includes artifacts unless the "Create characters in preserve mode" option
  was set when your character was created, in which case the artifacts may
  show up again later. This command takes some energy.

Go down staircase (``>``)
  Descends a down staircase you are standing on. There are always at least
  one staircase going down on each level, except for the town which has
  only one, and "quest" levels, which have none until the quest monster is
  killed. Going down a staircase will take you to a new dungeon level. See
  "Go Up Staircase" for more info. This command takes some energy.

Resting Commands
================

Stay still (with pickup) (``,``) or Stay still (with pickup) (``.``)
  Stays in the same square for one move. If you normally pick up objects
  you encounter, you will pick up whatever you are standing on. You may
  also use the ``5`` key (both keysets). This command may take a count, and
  takes some energy.

Get objects (``g``)
  Pick up objects and gold on the floor beneath you. Picking up gold takes
  no time, and objects take 1/10th of a normal turn each (maximum time cost
  is a full turn). You may pick up objects until the floor is empty or your
  backpack is full.

Rest (``R``)
  Resting is better for you than repeatedly staying still, and can be told
  to automatically stop after a certain amount of time, or when various
  conditions are met. In any case, you always wake up when anything
  disturbing happens, or when you press any key. To rest, enter the Rest
  command, followed by the number of turns you want to rest, or ``*`` to
  rest until your hitpoints and mana are restored, or ``&`` to rest until
  you are fully "healed". This command may take an argument (used for the
  number of turns to rest), and takes some energy.

Alter Commands
==============

Tunnel (``T``) or Tunnel ('^T')
  Tunnelling or mining is a very useful art. There are many kinds of rock,
  with varying hardness, including permanent rock (permanent), granite
  (very hard), quartz veins (hard), magma veins (soft), and rubble (very
  soft). Quartz and Magma veins may be displayed in a special way, and may
  sometimes contain treasure, in which case they will be displayed in a
  different way. Rubble sometimes covers an object but is easy to tunnel
  through, even with your bare hands.  Tunnelling ability increases with 
  strength and weapon weight. If you have a digging tool in your pack, the
  game will automatically use this to dig. This command may take a count,
  requires a direction, and takes some energy.

Open a door or chest (``o``)
  To open an object such as a door or chest, you must use this command. If
  the object is locked, you will attempt to pick the lock based on your
  disarming ability. If you open a trapped chest without disarming the
  traps first, the trap will be set off. Opening will automatically attempt
  to pick any door locks. You may need several tries to open a door or chest.
  This command may take a count, requires a direction, and takes some energy.

Close a door (``c``)
  Non-intelligent and some other creatures cannot open doors, so shutting
  doors can be quite valuable. Furthermore, monsters cannot see you behind
  closed doors, so closing doors may allow you to buy some time without
  being attacked. Broken doors cannot be closed.  This command may take a
  count, requires a direction, and takes some energy.

Disarm a trap or chest, or lock a door (``D``)
  You can attempt to disarm traps on the floor or on chests. If you fail,
  there is a chance that you will blunder and set it off. You can only
  disarm a trap after you have found it. The command can also be used to
  lock a closed door, which will create a hindrance for monsters. Even if
  many monsters will be able to pick the lock or bash the door down, it
  will often take them some time. This command may take a count, requires
  a direction, and takes some energy.

Alter (``+``)
  This special command allows the use of a single keypress to select any of
  the "obvious" commands above (attack, tunnel, bash, open, disarm),
  and, by using keymaps, to combine this keypress with directions. In
  general, this allows the use of the "control" key plus the appropriate
  "direction" key (including the roguelike direction keys in roguelike
  mode) as a kind of generic "alter the terrain feature of an adjacent
  grid" command. This command may take a count, requires a direction, and
  takes some energy.

Steal (``s``)
  This command is only available to rogues, and allows them to try and steal
  from a monster.  Stealing works better when the player is stealthy and
  faster than the target monster, and best of all when the victim is asleep.
  A failed theft will wake the monster; if you really bungle the attempt, the
  monster may shout out in anger.  This command requires a direction and
  takes some energy.

Spell Commands
=========================

Browse a book (``b``) or Peruse a book (``P``)
  Each class has books it can read and books it cannot; except for warriors,
  who cannot read any books. When this command is used, all of the spells
  contained in the selected book are displayed, along with information such as
  their level, the amount of mana required to cast them, and whether or not
  you know the spell.

Gain new spells (``G``)
  Use this command to actually learn new spells. When you are able to learn
  new spells, the word "Study" will appear on the status line at the bottom
  of the screen. If you have a book in your possession, containing spells
  which you may learn, then you may choose to study that book. Most classes
  may actually choose which spell to study, but if you are a priest or paladin,
  your gods will choose a prayer for you. There are five books of each
  realm, but hybrid classes - paladins, rogues, rangers and blackguards - can
  only cast from two or three of these.  Higher level books are normally found
  only in the dungeon. This command takes some energy.

Cast a spell (``m`` and ``p`` in both keysets)
  To cast a spell, you must have previously learned the spell and must have
  in your inventory a book from which the spell can be read. Each spell has
  a chance of failure which starts out fairly large but decreases as you
  gain levels. If you don't have enough mana to cast a spell, you will be
  prompted for confirmation. If you decide to go ahead, the chance of
  failure is greatly increased, and you may wind up paralyzed for several
  turns. Since you must read the spell from a book, you cannot be blind or
  confused while casting, and there must be some light present. This
  command takes some energy: the higher your level, the less it takes, but
  the higher the spell level, the more it takes.


Object Manipulation Commands
============================

Eat some food (``E``)
  You must eat regularly to prevent starvation. There is a hunger meter
  at the bottom of the screen, which says "Fed" and gives a percentage in
  most circumstances. If you go hungry long enough, you will become weak,
  then start fainting, and eventually, you may well die of starvation
  (accompanied by increasingly alarming messages on your hunger meter).
  It is also possible to be "Full", which will make you move slowly; more
  slowly the fuller you get. You may use this command to eat food in your
  inventory. Note that you can sometimes find food in the dungeon, but it
  is not always wise to eat strange food. This command takes some energy.

Fuel your lantern/torch (``F``)
  If you are using a lantern and have flasks of oil in your pack, then you
  can "refuel" them with this command. Torches and Lanterns are limited
  in their maximal fuel. In general, two flasks will fully fuel a lantern.
  This command takes some energy.

Quaff a potion (``q``)
  Use this command to drink a potion. Potions affect the player in various
  ways, but the effects are not always immediately obvious. This command
  takes some energy.

Read a scroll (``r``)
  Use this command to read a scroll. Scroll spells usually have an area
  effect, except for a few cases where they act on other objects. Reading a
  scroll causes the parchment to disintegrate as the scroll takes effect.
  Most scrolls which prompt for more information can be aborted (by
  pressing escape), which will stop reading the scroll before it
  disintegrates. This command takes some energy.
 
Inscribe an object (``{``) 
  This command inscribes a string on an object. The inscription is
  displayed inside curly braces after the object description. The
  inscription is limited to the particular object (or pile) and is not
  automatically transferred to all similar objects. Under certain
  circumstances, Angband will display "fake" inscriptions on certain
  objects ('tried', 'empty') when appropriate. These "fake" inscriptions
  remain all the time, even if the player chooses to add a "real" inscription
  on top of it later.

  In addition, Angband will place the inscription '??' on an object for you
  if the object has a property (or "rune") that you have not learned yet.
  This inscription will remain until you know all the runes on the object.

  An item labeled as '{empty}' was found to be out of charges, and an
  item labeled as '{tried}' is a "flavored" item which the character has
  used, but whose effects are unknown. Certain inscriptions have a meaning
  to the game, see '@#', '@x#', '!*', and '!x', in the section on
  inventory object selection.

Uninscribe an object (``}``)
  This command removes the inscription on an object. This command will have
  no effect on "fake" inscriptions added by the game itself.
  
Toggle ignore (``K``) or Toggle ignore (``O``)
  This command will toggle ignore settings.  If on, all ignored items 
  will be hidden from view.  If off, all items will be shown regardless
  of their ignore setting.  See the customize section for more info.

Magical Object Commands
=======================

Activate an object (``A``)
  You have heard rumors of special weapons and armor deep in the Pits,
  items that can let you breathe fire like a dragon or light rooms with
  just a thought. Should you ever be lucky enough to find such an item,
  this command will let you activate its special ability. Special abilities
  can only be used if you are wearing or wielding the item. This command
  takes some energy.

Aim a wand (``a``) or Zap a wand (``z``)
  Wands must be aimed in a direction to be used. Wands are magical devices,
  and therefore there is a chance you will not be able to figure out how to
  use them if you aren't good with magical devices. They will fire a shot
  that affects the first object or creature encountered or fire a beam that
  affects anything in a given direction, depending on the wand. An
  obstruction such as a door or wall will generally stop the effects from
  traveling any farther. This command requires a direction and can use a
  target. This command takes some energy.

Use a staff (``u``) or Zap a staff (``Z``)
  This command will use a staff. A staff is normally very similar to a
  scroll, in that they normally either have an area effect or affect a
  specific object. Staves are magical devices, and there is a chance you
  will not be able to figure out how to use them. This command takes some
  energy.
 
Zap a rod (``z``) or Activate a rod (``a``)
  Rods are extremely powerful magical items, which cannot be burnt or
  shattered, and which can have either staff-like or wand-like effects, but
  unlike staves and wands, they don't have charges. Instead, they draw on
  the ambient magical energy to recharge themselves, and therefore can only
  be activated once every few turns. The recharging time varies depending
  on the type of rod. This command may require a direction (depending on
  the type of rod, and whether you are aware of its type) and can use a
  target. This command takes some energy.

Throwing and Missile Weapons
============================

Fire an item (``f``) or Fire an item (``t``)
  This command will allow you to fire a missile from either your quiver or
  your inventory provided it is the appropriate ammunition for the current
  missile weapon you have equipped. You may not fire an item without a
  missile weapon equipped. Fired ammunition has a chance of breaking.
  This command takes some energy.

Fire default ammo at nearest (``h``) or ('TAB')
  If you have a missile weapon equipped and the appropriate ammunition in
  your quiver, you can use this command to fire at the nearest visible
  enemy. This command will cancel itself if you lack a launcher, ammunition
  or a visible target that is in range. The first ammunition of the correct
  type found in the quiver is used. This command takes some energy.

Throw an item (``v``)
  You may throw any object carried by your character. Depending on the
  weight, it may travel across the room or drop down beside you. Only one
  object from a pile will be thrown at a time. Note that throwing an object
  will often cause it to break, so be careful! If you throw something at a
  creature, your chances of hitting it are determined by your plusses to
  hit, your ability at throwing, and the object's plusses to hit. Some
  weapons are especially designed for throwing.  Once the
  creature is hit, the object may or may not do any damage to it. 
  Note that flasks of oil will do some fire damage to a monster on impact. 
  If you are wielding a missile launcher compatible with the object you are
  throwing, then you automatically use the launcher to fire the missile
  with much higher range, accuracy, and damage, than you would get by just
  throwing the missile. Throw, like fire, requires a direction. Targeting
  mode (see the next command) can be invoked with ``*`` at the 
  'Direction?' prompt. This command takes some energy.

Targeting Mode (``*``)
  This will allow you to aim your ranged attacks at a specific monster or
  grid, so that you can point directly towards that monster or grid (even
  if this is not a "compass" direction) when you are asked for a direction.
  You can set a target using this command, or you can set a new target at
  the "Direction?" prompt when appropriate. At the targeting prompt, you
  have many options. First of all, targeting mode starts targeting nearby
  monsters which can be reached by "projectable" spells and thrown objects.
  In this mode, you can press ``t`` (or ``5`` or ``.``) to select the
  current monster, space to advance to the next monster, ``-`` to back up to
  the previous monster, direction keys to advance to a monster more or less
  in that direction, ``r`` to "recall" the current monster, ``q`` to exit
  targeting mode, and ``p`` (or ``o``) to stop targeting monsters and
  enter the mode for targeting a location on the floor or in a wall. Note
  that if there are no nearby monsters, you will automatically enter this
  mode. Note that hitting ``o`` is just like ``p``, except that the
  location cursor starts on the last examined monster instead of on the
  player. In this mode, you use the "direction" keys to move around, and
  the ``q`` key to quit, and the ``t`` (or ``5`` or ``.``) key to target
  the cursor location. Note that targeting a location is slightly
  "dangerous", as the target is maintained even if you are far away. To
  cancel an old target, simply hit ``*`` and then 'ESCAPE' (or ``q``).
  Note that when you cast a spell or throw an object at the target
  location, the path chosen is the "optimal" path towards that location,
  which may or may not be the path you want. Sometimes, by clever choice of
  a location on the floor for your target, you may be able to convince a
  thrown object or cast spell to squeeze through a hole or corridor that is
  blocking direct access to a different grid. Launching a ball spell or
  breath weapon at a location in the middle of a group of monsters can
  often improve the effects of that attack, since ball attacks are not
  stopped by interposed monsters if the ball is launched at a target.

Looking Commands
================

Full screen map (``M``)
  This command will show a map of the entire dungeon, reduced by a factor
  of nine, on the screen. Only the major dungeon features will be visible
  because of the scale, so even some important objects may not show up on
  the map. This is particularly useful in locating where the stairs are
  relative to your current position, or for identifying unexplored areas of
  the dungeon.

Locate player on map (``L``) or Where is the player (``W``)
  This command lets you scroll your map around, looking at all sectors of
  the current dungeon level, until you press escape, at which point the map
  will be re-centered on the player if necessary. To scroll the map around,
  simply press any of the "direction" keys. The top line will display the
  sector location, and the offset from your current sector.

Look around (``l``) or Examine things (``x``)
  This command is used to look around at nearby monsters (to determine 
  their type and health) and objects (to determine their type). It is also 
  used to find out if a monster is currently inside a wall, and what is 
  under the player. When you are looking at something, you may hit space 
  for more details, or to advance to the next interesting monster or 
  object, or minus (``-``) to go back to the previous monster or object, 
  or a direction key to advance to the nearest interesting monster or 
  object (if any) in that general direction, or ``r`` to recall 
  information about the current monster race, or ``q`` or escape to stop 
  looking around. You always start out looking at "yourself". 

Inspect an item (``I``)
  This command lets you inspect an item. This will tell you things about
  the special powers of the object, as well as attack information for
  weapons. It will also tell you what resistances or abilities you have
  noticed for the item and if you have not yet completely identified all
  properties.
        
List visible monsters (``[``)
  This command lists all monsters that are visible to you, telling you how
  many there are of each kind. It also tells you whether they are asleep,
  and where they are (relative to you).

List visible items (``]``)
  This command lists all items that are visible to you, telling you how of
  each there are and where they are on the level relative to your current
  location.

Message Commands
================

Repeat level feeling ('^F')
  Repeats the feeling about the monsters in the dungeon level that you got
  when you first entered the level.  If you have explored enough of the 
  level, you will also get a feeling about how good the treasures are.

View previous messages ('^P')
  This command shows you all the recent messages. You can scroll through
  them, or exit with ESCAPE.

Take notes (``:``)
  This command allows you to take notes, which will then appear in your
  message list and your character history (prefixed with "Note:").

Game Status Commands
====================

Character Description (``C``)
  Brings up a full description of your character, including your skill
  levels, your current and potential stats, and various other information.
  From this screen, you can change your name or use the file character
  description command to save your character status to a file. That command
  saves additional information, including your background, your inventory,
  and the contents of your house.

Check knowledge (``~``)
  This command allows you to ask about the knowledge possessed by your
  character. Currently, this includes being able to list all known
  "artifacts", "uniques", and "objects". Normally, once an artifact is
  "generated", and "lost", it can never again be found, and will become
  "known" to the player. With "preserve" mode, an artifact can never be
  "lost" until it is "known" to the player. In either case, any "known"
  artifacts not in the possession of the player will never again be
  "generated". The "uniques" are special "unique" monsters which can only
  be killed once per game. Certain "objects" come in "flavors", and you 
  must determine the effect of each "flavor" once for each such type of 
  object, and this command will allow you to display all "known" flavors.
  Inscribing items in this list will cause you to similarly inscribe all
  similar items you find from this point on.

Saving and Exiting Commands
===========================

Save and Quit ('Ctrl-X')
  To save your game so that you can return to it later, use this command.
  Save files will also be generated (hopefully) if the game crashes due to
  a system error. After you die, you can use your savefile to play again
  with the same options and such.

Save ('Ctrl-S')
  This command saves the game but doesn't exit Angband. Use this frequently
  if you are paranoid about having your computer crash (or your power go
  out) while you are playing.

Quit (``Q``)
  Kills your character and exits Angband. You will be prompted to make sure
  you really want to do this, and then asked to verify that choice. Note
  that dead characters are dead forever.

User Pref File Commands
=======================

Interact with options (``=``)
  Allow you to interact with options. Note that using the "cheat" options
  may mark your savefile as unsuitable for the high score list. The
  "window" options allow you to specify what should be drawn in any of the
  special sub-windows (not available on all platforms). See the help files
  'customize.txt' and 'options.txt' for more info. You can also interact
  with keymaps under this menu.

Interact with keymaps - option submenu
  Allow you to interact with keymaps. You may load or save keymaps from
  user pref files, or define keymaps. You must define a "current action",
  shown at the bottom of the screen, before you attempt to use any of the
  "create macro" commands, which use that "current action" as their action.
 
Interact with visuals - option submenu
  Allow you to interact with visuals. You may load or save visuals from
  user pref files, or modify the attr/char mappings for the monsters,
  objects, and terrain features. You must use the "redraw" command ('^R')
  to redraw the map after changing attr/char mappings. NOTE: It is
  generally easier to modify visuals via the "knowledge" menus.

Interact with colors - option submenu
  Allow the user to interact with colors. This command only works on some
  systems. NOTE: It is commonly used to brighten the 'Light Dark' color
  (eg. Cave Spiders) on displays with bad alpha settings.

Help Commands
=============

Help (``?``)
  Brings up the Angband on-line help system. Note that the help files are
  just text files in a particular format, and that other help files may be
  available on the Net. In particular, there are a variety of spoiler files
  which do not come with the standard distribution. Check the place you got
  Angband from or ask on the Angband forums, angband.oook.cz , about them.

Identify Symbol (``/``)
  Use this command to find out what a character stands for. For instance,
  by pressing '/.', you can find out that the ``.`` symbol stands for a
  floor spot. When used with a symbol that represents creatures, the this
  command will tell you only what class of creature the symbol stands for,
  not give you specific information about a creature you can see. To get
  that, use the Look command.

  There are three special symbols you can use with the Identify Symbol
  command to access specific parts of your monster memory. Typing
  'Ctrl-A' when asked for a symbol will recall details about all
  monsters, typing 'Ctrl-U' will recall details about all unique
  monsters, and typing 'Ctrl-N' will recall details about all non-unique
  monsters.

  If the character stands for a creature, you are asked if you want to
  recall details. If you answer yes, information about the creatures you
  have encountered with that symbol is shown in the Recall window if
  available, or on the screen if not. You can also answer ``k`` to see the
  list sorted by number of kills, or ``p`` to see the list sorted by 
  dungeon level the monster is normally found on. Pressing 'ESCAPE' at 
  any point will exit this command.

Game Version (``V``)
  This command will tell you what version of Angband you are using. For
  more information, see the 'version.txt' help file.

Extra Commands
==============

Toggle Choice Window ('^E')
  Toggles the display in any sub-windows (if available) which are
  displaying your inventory or equipment.

Redraw Screen ('^R')
  This command adapts to various changes in global options, and redraws all
  of the windows. It is normally only necessary in abnormal situations,
  such as after changing the visual attr/char mappings, or enabling
  "graphics" mode.

Save screen dump (|``)``|)
  This command dumps a "snap-shot" of the current screen to a file,
  including encoded color information. The command has two variants:

  - html, suitable for viewing in a web browser.
  - forum embedded html for vBulletin, suitable for pasting in
    web forums like http://angband.oook.cz/forums.
	
Special Keys
============
 
Certain special keys may be intercepted by the operating system or the host
machine, causing unexpected results. In general, these special keys are
control keys, and often, you can disable their special effects.

If you are playing on a UNIX or similar system, then Ctrl-C will interrupt
Angband. The second and third interrupt will induce a warning bell, and the
fourth will induce both a warning bell and a special message, since the
fifth will quit the game, after killing your character. Also, 'Ctrl-Z'
will suspend the game, and return you to the original command shell, until
you resume the game with the 'fg' command. There is now a compilation
option to force the game to prevent the "double 'ctrl-z' escape death
trick". The 'Ctrl-\\' and 'Ctrl-D' and 'Ctrl-S' keys should not be
intercepted.

It is often possible to specify "control-keys" without actually pressing
the control key, by typing a caret (``^``) followed by the key. This is
useful for specifying control-key commands which might be caught by the
operating system as explained above.

Pressing backslash (``\``) before a command will bypass all keymaps, and
the next keypress will be interpreted as an "underlying command" key,
unless it is a caret (``^``), in which case the keypress after that will be
turned into a control-key and interpreted as a command in the underlying
angband keyset. For example, the sequence ``\`` + ``.`` + ``6`` will always
mean "run east", even if the ``.`` key has been mapped to a different
underlying command.

The ``0`` and ``^`` and ``\`` keys all have special meaning when entered at
the command prompt, and there is no "useful" way to specify any of them as
an "underlying command", which is okay, since they would have no effect.

For many input requests or queries, the special character ESCAPE will abort
the command. The '[y/n]' prompts may be answered with ``y`` or ``n``, or
'ESCAPE'. The '-more-' message prompts may be cleared (after reading
the displayed message) by pressing 'ESCAPE', 'SPACE', 'RETURN',
'LINEFEED', or by any keypress, if the "quick_messages" option is turned
on.
 
Command Counts
==============

Some commands can be executed a fixed number of times by preceding them
with a count. Counted commands will execute until the count expires, until
you type any character, or until something significant happens, such as
being attacked. Thus, a counted command doesn't work to attack another
creature. While the command is being repeated, the number of times left to
be repeated will flash by on the line at the bottom of the screen.

To give a count to a command, type ``0``, the repeat count, and then the
command. If you want to give a movement command and you are using the
original command set (where the movement commands are digits), press space
after the count and you will be prompted for the command.

Counted commands are very useful for time consuming commands, as they
automatically terminate on success, or if you are attacked. You may also
terminate any counted command (or resting or running), by typing any
character. This character is ignored, but it is safest to use a 'SPACE' 
or 'ESCAPE' which are always ignored as commands in case you type the
command just after the count expires.	

.. |``)``| replace:: ``)``

