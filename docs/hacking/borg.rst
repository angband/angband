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

1. Ensure Angband is compiled with borg support (this is controlled by
   ``ALLOW_BORG``)
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
``$``  Reload borg.txt
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

The Borg's behavior is primarily configured through the ``borg.txt`` file.
This allows for extensive customization of the Borg's decision-making without
needing to recompile the game.

A sample ``borg.txt`` file is provided in the ``src/borg`` directory of the
source code. To use it, copy this file to the user preferences directory for
your operating system, and then customize it.

- Windows: Copy ``src/borg/borg.txt`` to ``lib/user/borg.txt``
- macOS/Linux: Copy ``src/borg/borg.txt`` to ``~/.angband/Angband/borg.txt``

Once copied, you can edit ``borg.txt`` to change the Borg's behavior. To apply
changes while the game is running, use the ``$`` command from the Borg command
interface (``^z``).

How you customize the Borg depends on whether you are using a pre-compiled
build or compiling from source.

Configuration Options
---------------------

The ``borg.txt`` file offers a wide range of options to customize the Borg's
behavior. Below is a summary of the key settings. For a complete list and
detailed explanations, refer to the comments within the ``borg.txt`` file
itself.

Adjusting Borg Speed
********************

When you first run the Borg it may move very slowly. This is often due to the
game's ``base delay factor``, a general setting that affects all animations. To
speed up the Borg you can decrease this value:

1. Press ``=`` to open the main options menu
2. Press ``d`` to change the ``delay factor``
3. Decrease the value

Conversely, if the Borg is moving too quickly to follow, you can increase this
value. You can also add a Borg-specific delay by setting ``borg_delay_factor``
in ``borg.txt``.

Worships
********
These settings (e.g., ``borg_worships_damage``, ``borg_worships_gold``)
influence the Borg's priorities and decision-making by assigning value to
different actions and items. For example, they can make the Borg favor
powerful weapons, seek out treasure, or prioritize speed.

Play Style
**********
- ``borg_plays_risky``: Makes the Borg dive deeper faster and be more
  aggressive in combat
- ``borg_kills_uniques``: Forces the Borg to defeat uniques before
  proceeding deeper into the dungeon

Item Management
***************
- ``borg_uses_swaps``: Allows the Borg to carry and use swap items for
  situational resistances and abilities
- ``borg_worships_gold``: Causes the Borg to return to town frequently to
  sell items for gold, especially at lower levels

Respawn and Continuous Play
***************************
- ``borg_cheat_death``: If enabled, the Borg will not die and will
  continue playing, enabling continuous play. This can be set in
  ``borg.txt`` or toggled via the Borg command interface (``^z``, then
  ``c``, then ``d``)
- ``borg_respawn_race`` and ``borg_respawn_class``: Specify the race and
  class for the next character when the Borg respawns
- ``borg_respawn_winners``: If enabled, the Borg will create a new
  character after defeating Morgoth

Dynamic Formulas
****************

The Borg can use either its internal hard-coded logic for decision-making
or a more flexible system of dynamic formulas defined in ``borg.txt``. To
enable the formula-based system, set the following in ``borg.txt``:

.. code-block:: ini

  borg_uses_dynamic_calcs = TRUE

The dynamic calculations are more customizable but may be slower and
are not always as up-to-date as the internal code logic.

Using Official Builds
---------------------

In most official builds, Borg support is already included and enabled. You just
need to copy and configure the ``borg.txt`` file in the correct location as
described above.

Compiling Yourself
------------------

When compiling from source, the Borg is enabled by default on most platforms.
For starter instructions on how to compile, see the :doc:`compiling` guide.

If you find the Borg is disabled in your build configuration, you can typically
enable it by:

- Uncommenting an ``allow_borg`` line in a configuration file (like
  ``config.h``)
- Passing a ``-DALLOW_BORG`` flag to the compiler

When compiling, you can also enable the ``SCORE_BORGS`` flag to allow Borg
characters to appear in the high score list. This is disabled by default.

Refer to the compilation instructions for your specific platform for details.
After compiling with Borg support, place your ``borg.txt`` file in the correct
location as described above.

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

The Borg can be configured to run as a Windows screensaver that
automatically plays the game in continuous play mode, automatically
restarting with new characters when the current character dies.

**WARNING:** The Angband display is not always dynamic. While modern LCD
monitors are not susceptible to burn-in, OLED displays may still experience
image retention with prolonged static content. Configure energy saving
settings to turn off your monitor after inactivity. The screensaver keeps
the processor and hard disk busy, preventing power-saving features that
depend on inactivity.

Installation
------------

1. Copy ``angband.scr`` and the included ``angband.ini`` into your Windows
   directory

2. Ensure you have the Windows version of Angband installed with all supporting
   files in the ``lib`` directory

3. Edit ``angband.ini`` with a text editor:
   
   - Set ``AngbandPath`` to point to your Angband installation directory
     (must end with a backslash ``\``)
   - Set ``SaverFile`` to the character name you want to use for the screensaver
     (a random character will be automatically created if the character doesn't
     exist)

   Example configuration::
   
       [Angband]
       AngbandPath="c:\games\angband-4.2.5\"
       SaverFile="Saver"

4. Test the screensaver in Windows Display Properties

It's recommended to create a normal character first using regular Angband,
set up your terminal windows as desired, save that file, and use that filename
as the ``SaverFile`` for your screensaver.

Technical Details
-----------------

- The screensaver is a renamed Windows Angband executable with modified
  ``main-win.c``
- Normal Borgs get highscore entries, but screensaver Borgs (continuous
  play mode) do not
- Uses low priority processing to avoid slowing down other processes

  - Can be toggled via "Options/Low priority" menu when using as normal
    executable for background Borg play
- Uses the normal Angband installation's ``angband.ini`` for screen layout,
  graphics, and sound settings
- Can be used as a normal Angband executable by renaming to ``angband.exe``

Known Limitations
-----------------

- No preview in Windows Display Properties
- Password protection not implemented
- Configuration requires manual ``ini`` file editing
- "Show scores" while Borg is running may cause crashes
- Cannot run the same savefile simultaneously (e.g., normal game 
  and screensaver)
- Info window sizes may increase when exiting pseudo-screensaver mode from
  options menu
