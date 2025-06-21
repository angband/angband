====
Borg
====

The Borg is an "Automatic Angband Player".

It was first written for about 2.8.0, separate from the game as
distributed. It was pulled into the game in around 3.3. It was removed
in 4.0 as there was too much conflict with the big changes made then.
It was reincorporated in around 4.2.3.

Running The Borg
================

It is not recommended to run the Borg on a live game, as it could
cause unexpected behavior or even crashes. It is best to run the Borg
on its own save file, or on a copy of your save file.

Historical information on how to run the Borg can be found in
``borgread.txt``.

Borg Command Interface
======================

The Borg command interface is only available when Angband is compiled
with borg support.

To access the Borg command interface, press ``^z`` (Ctrl-Z) during normal
gameplay. When you first run the command you'll be presented with a warning
message you can continue through. The most common command is ``z`` which
starts the Borg.

Pressing ``^z`` while the Borg is running will stop the Borg.

Main Commands
-------------

====== ========================================
``$``  Reload Borg.txt
``z``  Activate the Borg
``u``  Update the Borg
``x``  Step the Borg
``f``  Toggle flags
``c``  Toggle cheat flags
``s``  Search mode
``g``  Display grid feature
``i``  Display grid info
``a``  Display avoidances
``k``  Display monster info
``t``  Display object info
``%``  Display targeting flow
``#``  Display danger grid
``_``  Regional Fear info
``p``  Borg Power
``!``  Time
``@``  Borg LOS
``w``  My Swap Weapon
``q``  Auto stop on level
``v``  Version stamp
``d``  Dump spell info
``h``  Borg_Has function
``y``  Last 75 steps
``m``  Money Scum
``^``  Flow Pathway
``R``  Respawn Borg
``o``  Object Flags
``r``  Restock Stores
``l``  Create a snapshot log file
``;``  Display glyphs
``1``  Change max depth
``2``  Level prep info
``3``  Feature of grid
``C``  List nasties
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

