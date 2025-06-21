====
Borg
====

The Borg is an "Automatic Angband Player".

It was first written for about 2.8.0, separate from the game as
distributed. It was pulled into the game in around 3.3. It was removed
in 4.0 as there was too much conflict with the big changes made then.
It was reincorporated in around 4.2.3.

The name comes from "The Borg" in Star Trek which, in turn, comes from
cyborg.

The primary use of the Borg is entertainment. It is fun to watch the
Borg play the game, and it can be amusing to see how it handles
situations. It has been used to test the game and find bugs.

Running The Borg
================

It is not recommended to run the Borg on a live game, as it could
cause unexpected behavior or even crashes. It is best to run the Borg
on its own save file, or on a copy of your save file.

To run the Borg:

1. Ensure Angband is compiled with borg support
2. Start or load a game
3. Press ``^z`` (Ctrl-Z) to access the Borg command interface
4. Press ``z`` to activate the Borg
5. Watch the Borg play automatically
6. Press any key to stop the Borg when desired

Borg Command Interface
======================

The Borg command interface is only available when Angband is compiled
with borg support.

To access the Borg command interface, press ``^z`` (Ctrl-Z) during
gameplay. When you first run the command you'll be presented with a warning
message you can continue through. The most common command is ``z`` which
starts the Borg.

Pressing any key while the Borg is running will stop the Borg.

Main Commands
-------------

====== ========================================
``a``  Display avoidances
``c``  Toggle cheat flags
``C``  List nasties
``d``  Dump spell info
``f``  Toggle flags
``g``  Display grid feature
``h``  Borg_Has function
``i``  Display grid info
``k``  Display monster info
``l``  Create a snapshot log file
``m``  Money Scum
``o``  Object Flags
``p``  Borg Power
``q``  Auto stop on level
``r``  Restock Stores
``R``  Respawn Borg
``s``  Search mode
``t``  Display object info
``u``  Update the Borg
``v``  Version stamp
``w``  My Swap Weapon
``x``  Step the Borg
``y``  Last 75 steps
``z``  Activate the Borg
``?``  List Borg commands
``!``  Time
``#``  Display danger grid
``%``  Display targeting flow
``$``  Reload Borg.txt
``@``  Borg LOS
``^``  Flow Pathway
``_``  Regional Fear info
``;``  Display glyphs
``1``  Change max depth
``2``  Level prep info
``3``  Feature of grid
====== ========================================

Flag Commands
-------------

After pressing ``f`` from the main borg interface you enter flag toggle mode.

====== ========================================
``b``  Stop when alert bell rings
``c``  Self scum
``k``  Stop when the borg wins
``l``  Lunal mode
``s``  Dump savefile at each level (autosave)
``v``  Verbose mode
====== ========================================

Cheat Commands
--------------

After pressing ``c`` from the main borg interface you enter cheat toggle mode.

====== ========================================
``d``  Toggle cheat death
====== ========================================

Customizing The Borg
====================

borg.txt Configuration
----------------------

The Borg can be configured through the ``borg.txt`` file. To reload
configuration changes:

1. Press ``^z`` to access the Borg command interface
2. Press ``$`` to reload the ``borg.txt`` file

TODO: Explain borg.txt configuration options, where to put file, and
compiling yourself vs official builds.

Borg Logging
============

The Borg suppresses most messages by default. To see what the Borg is doing,
you'll want to use multi-window support to display additional information
windows.

Window Configuration
--------------------

For optimal Borg monitoring, open additional terminal windows to display:

- Equipment: See what the Borg is wearing and wielding
- Messages: View game messages and Borg status updates
- Monster Recall: See information about monsters the Borg encounters
- Inventory: Monitor what items the Borg is carrying

Set these up through the :ref:`window menu <showing-extra-info-in-subwindows>`
before activating the Borg. Borg-specific messages will appear in the
Messages window when verbose mode is enabled.

Verbose Mode
------------

Enable verbose mode to get detailed output about the Borg's decision-making
process, including calculations, target selection, danger assessment, and
action decisions.

Via Flag Command
****************

1. Press ``^z`` to access the Borg command interface
2. Press ``f`` to enter flag toggle mode
3. Press ``v`` to toggle verbose mode on/off

Via Configuration
*****************

Set ``borg_verbose = TRUE`` in the ``borg.txt`` configuration file, then
reload with ``^z`` ``$``.

Log Snapshot
------------

Create a detailed snapshot of the current game state for debugging:

1. Press ``^z`` to access the Borg command interface
2. Press ``l`` to create a snapshot log file

This generates a comprehensive ``.map`` file (e.g., ``player_name.map``) in
your Angband ``archive`` directory containing:

- ASCII dungeon map: Current level layout showing terrain, monsters (``&``),
  items, and player (``@``) position
- Recent game messages: Last actions, movements, and events
- Complete character state: Equipment, inventory, quiver, and home contents
- Borg configuration: Current swap items and borg settings
- Detailed statistics: All internal borg trait values, resistances, and
  assessments

The snapshot provides a complete picture of both the game state and the
Borg's internal knowledge at that moment, useful for understanding its
behavior or debugging issues.

Borg Screensaver
================

TODO
