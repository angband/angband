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

TODO

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
   files in the Lib directory

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
- Configuration requires manual INI file editing
- "Show scores" while Borg is running may cause crashes
- Cannot run the same savefile simultaneously (e.g., normal game 
  and screensaver)
- Info window sizes may increase when exiting pseudo-screensaver mode from
  options menu
