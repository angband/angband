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

