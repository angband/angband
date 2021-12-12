==========================
Debug Command Descriptions
==========================

Item Creation
=============

Create an object ``c``
  Provides a menu to let you create any object, and drops it on the floor.

Create an artifact ``C``
  Provides a menu to let you create any artifact, and drops it on the floor.

Create a good object ``g``
  Prompts for the number of objects to create and then creates that many
  good objects nearby.

Create a very good object ``v``
  Prompts for the number of objects to create and then creates that many
  very good ("excellent") objects nearby.

Play with an object ``o``
  Lets you modify an object by randomly rerolling it as a normal, good, or
  excellent object or lets you edit its attributes, including quantity, ego
  type, presence or absence of curses, combat values, and modifiers.  There's
  also a statistics option to evaluate how likely worse, better, or matching
  objects of the same kind would be generated.

Test kind ``V``
  Will prompt for a tval, as an integer.  For that tval, creates one object
  of each sval and drops it nearby.  There is a similar option with the 'c'
  command, but this one will generate any instant artifacts associated with
  a tval and selects the tval by number rather than name.

Detection / Information
=======================

Detect all ``d``
  Detects all traps, doors, stairs, treasure, and monsters nearby.

Magic Mapping ``m``
  Maps the nearby dungeon.

Learn about objects ``l``
  Makes you "aware" of all items with level less than or equal to 100.

Monster recall ``r``
  Gives you full monster recall on all monsters or on a chosen monster.

Wipe recall ``W``
  Resets monster recall on all monsters or on a chosen monster.

Unhide monsters ``u``
  Reveals all monsters.

Wizard-light the level ``w``
  Lights the entire level, as the Potion of Enlightenment.

Create spoilers ``"``
  Lets you create a spoiler file for objects or monsters.

Teleportation
=============

Teleport level ``j``
  Allows you to teleport to any dungeon level instantly.

Phase Door ``p``
  Teleports you up to 10 spaces away.

Teleport ``t``
  Teleports you up to 100 spaces away.

Teleport to target ``b``
  Teleports you to the targeted grid (or close to it, if it is occupied).

Character Improvement
=====================

Cure all maladies ``a``
  Removes all curses, restores all stats, xp, hp, and sp, cures all bad
  effects, and satisfies your hunger.

Advance the character ``A``
  Advances your character to level 50, maxes all stats, and gives you a
  million gold.

Edit character ``e``
  Lets you specify your base stats, xp, and gold.

Increase experience ``x``
  Prompts for an amount, up to 9999, to add to your current experience.

Rerate hitpoints ``h``
  Rerates your hitpoints.

Monsters
========

Summon monster ``n``
  Prompts you for the name or integer index of a monster, then summons that
  monster nearby.

Summon random monster ``s``
  Prompts for a number and then summons that many random monsters near you.

Zap monsters ``z``
  Prompts for a distance, up to the maximum sight range, and deletes all
  monsters within that distance.

Hit all in line of sight ``H``
  Hits all monster in the line of sight for a large, 10000, amount of damage.

.. _DebugDungeon:

Dungeon
========

Create a trap ``T``
  Prompts for the type of trap to create and places it on your square.

Perform an effect ``E``
  Prompts for an effect type and its parameters.  Then executes that effect.

Quit without saving ``X``
  Quits the game without saving (prompts first).

Query the dungeon ``q``
  Light up all the grids with a given square flag
  (see src/list-square-flags.h).

Query terrain ``F``
  Light up all the grids with a given terrain type
  (see lib/gamedata/terrain.txt).

Collect stats ``f`` or ``S``
  Collects stats on monsters and objects present on level generation.
  Requests number of runs, and whether diving or clearing levels, and
  outputs the results into the file 'stats.log' in the user directory.
  The comments in that file will be helpful for interpreting the
  results; for more in-depth information, it's best to check the
  implementation of stats_collect() in wiz-stats.c.

Collect disconnection stats ``D``
  Generates several levels to collect statistics about how often all
  down staircases are inaccessible to the player, how often the player's
  starting location isn't valid,  and how often a level has non-vault
  areas that are inaccessible to the player.  The results are written
  to the message window, and maps of the levels that are disconnected or
  have invalid starting locations are written to 'disconnect.html' in
  the user directory.  Also collects general statistics about the
  layout of all the generated levels and writes them to
  'disconnect_gstat.txt' in the user directory.  For more in-depth
  details about what's considered disconnected and what else is
  summarized about level generation, check the implementation for
  disconnect_stats() in wiz-stats.c.

Collect pit stats ``P``
  Generates several pits of the room type you specify (pit, nest, or
  other) and computes a histogram of the types of monsters involved.
  The results are written to the message window.

Nick hack ``_``
  Maps out the reachable grids (by the sound and scent algorithm) in
  successive distances from the player grid.

Push objects ``>``
  Pushes objects off the targeted grid as a way of exercising push_object().

Write a map of the current level ``M``
  Writes out a map of the current level as an HTML file.

Miscellaneous
=============

Animations demo ``G``
  Displays the graphics or characters used for animating projection effects.

Key log ``L``
  Displays the recent keystrokes entered.
