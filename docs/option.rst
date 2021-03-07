===================
Option Descriptions
===================

Options are accessible through the ``=`` command, which provides an
interface to the various sets of options available to the player.

In the descriptions below, each option is listed as the textual summary
which is shown on the "options" screen, plus the internal name of the
option in brackets, followed by a textual description of the option.

Various concepts are mentioned in the descriptions below, including
"disturb", (cancel any running, resting, or repeated commands, which are in
progress), "flush" (forget any keypresses waiting in the keypress queue),
"fresh" (dump any pending output to the screen), and "sub-windows" (see
below).

.. contents:: Option pages
   :local:
   :depth: 1

User Interface Options
======================

Rogue-like commands ``rogue_like_commands``
  Selects the "roguelike" command set (see "command.txt" for info).

Use sound ``use_sound``
  Turns on sound effects, if your system supports them.

Show damage player deals to monsters ``show_damage``
  Shows the damage that the player deals to monsters for melee and ranged 
  combat in the messages.

Use old target by default ``use_old_target``
  Forces all commands which normally ask for a "direction" to use the
  current "target" if there is one. Use of this option can be dangerous if
  you target locations on the ground, unless you clear them when done.

Always pickup items ``pickup_always``
  Automatically picks up items when you walk upon them, provided it is safe
  to do so.

Always pickup items matching inventory ``pickup_inven``
  Like ``pickup_always``, but picks up an item only if it is a copy of an
  item that you already have in your inventory.

Show flavors in object descriptions ``show_flavors``
  Display "flavors" (color or variety) in object descriptions, even for
  objects whose type is known. This does not affect objects in stores.  

Highlight target with cursor ``show_target``
  Highlights the current targeted monster with a cursor.  Useful when 
  combined with "use old target by default".

Highlight player with cursor between turns ``highlight_player``
  Highlights the player with a cursor.  Useful if you have trouble finding
  the player.

Disturb whenever viewable monster moves ``disturb_near``
  Disturb the player when any viewable monster moves, whenever any monster
  becomes viewable for the first time, and also whenever any viewable
  monster becomes no longer viewable. This option ignores the existence of
  "telepathy" for the purpose of determining whether a monster is
  "viewable".

Show walls as solid blocks ``solid_walls``
  Walls are solid blocks instead of # for granite and % for veins.  Veins
  are coloured darker than granite for differentiation purposes.

Show walls with shaded backgrounds ``hybrid_walls``
  Walls appear as # and % symbols overlaid on a gray background block.  
  This overrides ``solid_walls``.

Color: Illuminate torchlight in yellow ``view_yellow_light``
  This option causes special colors to be used for "torch-lit" grids.
  Turning this option off will slightly improve game speed.

Color: Shimmer multi-colored things ``animate_flicker``
  Certain powerful monsters and items will shimmer in real time, i.e.
  between keypresses.  

Center map continuously ``center_player``
  The map always centres on the player with this option on. With it off, it
  is divided into 25 sections, with coordinates (0,0) to (4,4), and will
  show one section at a time - the display will "flip" to the next section
  when the player nears the edge.

Color: Show unique monsters in purple ``purple_uniques``
  All "unique" monsters will be shown in a light purple colour, which is
  not used for any "normal" monsters - so you can tell at a glance that
  they are unique. If you like the idea but don't like the colour, you can
  edit it via the "interact with colors" option.

Automatically clear -more- prompts ``auto_more``
  The game does not wait for a keypress when it comes to a '-more-'
  prompt, but carries on going.  

Color: Player color indicates low hit points ``hp_changes_color``
  This option makes the player ``@`` turn various shades of colour from
  white to red, depending on percentage of HP remaining.

Allow mouse clicks to move the player  ``mouse_movement``
  Clicking on the main window will be interpreted as a move command to that
  spot.

Notify on object recharge ``notify_recharge``
  This causes the game to print a message when any rechargeable object
  (i.e. a rod or activatable weapon or armour item) finishes recharging. It
  is the equivalent of inscribing '{!!}' on all such items.  

Show effective speed as multiplier ``effective_speed``
  Instead of showing absolute speed modifier (e.g. 'Slow (-2)' or 'Fast (+38)'),
  show the effective rate at which the character is moving (e.g. 'Slow (x0.8)'
  or 'Fast (x4.1)').


Birth options
=============

The birth options may only be changed when creating a character or using
the quick restart option for a dead character.  When setting the birth
options, there are handful of commands to make it easier to get to a
well-known state for all the birth options.  They are:  's' to save the
current selections so that they will be used as the starting point for
future characters, 'r' to reset the current selections to the defaults
for a new character, and 'm' to reset the current selections to the
Angband maintainer's defaults for the birth options.

Generate a new, random artifact set ``birth_randarts``
  A different set of artifacts will be created, in place of the standard
  ones. This is intended primarily for people who have played enough to
  know what most of the standard artifacts do and want some variety. The
  number, findability and power of random artifacts will all match the
  standard set - approximately.

Generate connected stairs ``birth_connect_stairs``
  With this option turned on, if you go down stairs, you start the new level
  on an up staircase, and vice versa (if you go up stairs, you start the
  next level on a down staircase).

  With this option off, you will never start on a staircase - but other
  staircases up and down elsewhere on the level will still be generated.

Force player descent (never make up stairs) ``birth_force_descend``
  Upwards staircases do not work.  All downward staircases, including the
  one in town, transport the character one level below the previous maximum
  depth.  Recalling from the dungeon works and brings the character to the
  town.  However, recalling from town brings the character one level 
  below the previous maximum depth.  The character cannot recall from quest
  levels until the quest is complete, however you will be warned before
  descending into a quest level.  Any status effects that sometimes 
  teleports the character up and sometimes teleports them down will always
  choose down.  When combined with the option for word of recall scrolls
  to have no effect, this recreates the previous "ironman" option.  

Word of Recall has no effect ``birth_no_recall``
  Word of Recall scrolls have no effect.  When combined with the option
  to force player descent, this recreates the previous "ironman" option.

Restrict creation of artifacts ``birth_no_artifacts``
  No artifacts will be created. Ever. Just *how* masochistic are you?

Stack objects on the floor ``birth_stacking``
  With this option turned on, multiple items can occupy one grid.

  With this option off, items dropped on the floor will spread out instead
  of stacking. Normal items will disappear if there is no empty grid
  within a radius of three squares.

Lose artifacts when leaving level ``birth_lose_arts``
  Normally if you leave a level with an unidentified artifact on it you may
  still find it later. With this option on, if you leave a level with an
  artifact on it's gone for the rest of the game whether you knew it was
  there or not. Note that this option has no effect on artifacts which you
  have already identified (i.e. picked up) - these will always be
  permanently lost if you leave a level without taking them with you.

Show level feelings ``birth_feelings``
  With this option turned on, the game will give you hints about what a new
  level has on it.

  With this option off, these hints will not be shown.

Increase gold drops but disable selling ``birth_no_selling``
  Shopkeepers will never pay you for items you sell, though they will still
  identify unknown items for you, and will still sell you their wares. To
  balance out income in the game, gold found in the dungeon will be
  increased if this option is on.

Start with a kit of useful gear ``birth_start_kit``
  Start with items, a useful option for new players, or ones that wish
  to descend immediately into the dungeon.  If turned off, the character
  will start with additional gold with which to purchase starting gear.

Monsters learn from their mistakes ``birth_ai_learn``
  Allow monsters to learn what spell attacks you are resistant to, and to
  use this information to choose the best attacks.  This option makes the
  game very difficult and is not recommended.

Know all runes on birth ``birth_know_runes``
  For players who don't enjoy the "identify by use" process for wearable
  items.  This option means all object properties are known at the outset, so
  artifacts and ego items will be identified on walking over them.

Know all flavors on birth ``birth_know_flavors``
  For players who don't enjoy the "identify by use" process for consumable
  items.  This option means all object flavors are known at the outset.

Persistent levels (experimental) ``birth_levels_persist``
  Each level is generated for the first time when the player enters it, and 
  from then on when the player returns the level is as they last saw it, 
  including monsters, items and traps.

To-damage is a percentage of dice (experimental) ``birth_percent_damage``
  Instead of bonuses to damage being just added on to damage dealt, each +1
  adds 5% to the value of the damage dice. This option is currently not
  very balanced.

Cheating options
================

Peek into monster creation ``cheat_hear``
  Cheaters never win. But they can peek at monster creation.

Peek into dungeon creation ``cheat_room``
  Cheaters never win. But they can peek at room creation.

Peek into something else ``cheat_xtra``
  Cheaters never win. But they can see debugging messages.

Allow player to avoid death ``cheat_live``
   Cheaters never win. But they can cheat death.

Window flags
============

Some platforms support "sub-windows", which are windows which can be used
to display useful information generally available through other means. The
best thing about these windows is that they are updated automatically
(usually) to reflect the current state of the world. The "window options"
can be used to specify what should be displayed in each window. The 
possible choices should be pretty obvious.

Display inven/equip
  Display the player inventory (and sometimes the equipment).

Display equip/inven
  Display the player equipment (and sometimes the inventory).

Display player (basic)
  Display a brief description of the character, including a breakdown of
  the current player "skills" (including attacks/shots per round).

Display player (extra)
  Display a special description of the character, including some of the
  "flags" which pertain to a character, broken down by equipment item.

Display player (compact)
  Display a brief description of the character, including a breakdown of
  the contributions of each equipment item to various resistances and
  stats.

Display map view
  Display the area around the player or around the target while targeting.
  This allows using graphical tiles in their original size.

Display messages
  Display the most recently generated "messages".

Display overhead view
  Display an overhead view of the entire dungeon level.

Display monster recall
  Display a description of the monster which has been most recently
  attacked, targeted, or examined in some way.

Display object recall
  Display a description of the most recently selected object. Currently
  this only affects spellbooks and prayerbooks. This window flag may be
  usefully combined with others, such as "monster recall".

Display monster list
  Display a list of monsters you know about and their distance from you.

Display status
  Display the current status of the player, with permanent or temporary boosts,
  resistances and status ailments (also available on the main window).

Display item list
  Display a list of items you know about and their distance from you.

Left Over Information
=====================

The ``hitpoint_warn`` value, if non-zero, is the percentage of maximal
hitpoints at which the player is warned that they may die. It is also used as
the cut-off for using the color red to display both hitpoints and mana.

The ``delay_factor`` value, if non-zero, will slow down the visual effects
used for missile, bolt, beam, and ball attacks. The actual time delay is
equal to ``delay_factor`` squared, in milliseconds.

The ``lazymove_delay`` value, if non-zero, will allow the player to move
diagonally by pressing the two appropriate arrow keys within the delay time.
This may be useful particularly when using a keyboard with no numpad.
