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
``z``  Activate the Borg (start automation)
``u``  Update the Borg
``x``  Step the Borg (single step)
``s``  Search mode
``k``  Display monster info
``t``  Display object info
``p``  Display borg power
``d``  Display borg danger
``i``  Display borg item values
``a``  Display borg artifacts
``w``  Display borg weapon information
``r``  Display borg race information
``c``  Modify cheat flags
``f``  Toggle flags (enters flag mode)
``l``  Log borg messages
``m``  Display borg messages
``n``  Display borg notes
``q``  Quit borg interface
====== ========================================

Flag Commands
-------------

After pressing ``f`` from the main borg interface you enter flag toggle mode.

====== ========================================
``a``  Toggle flag: Allow borg to play
``v``  Toggle flag: Verbose mode
``d``  Toggle flag: Debug mode
``s``  Toggle flag: Stop on stairs
``g``  Toggle flag: Graphics mode
``r``  Toggle flag: Respawn
``c``  Toggle flag: Cheat death
``n``  Toggle flag: No retreat
``t``  Toggle flag: Testable
``l``  Toggle flag: Light beam
``u``  Toggle flag: Unique tracking
``m``  Toggle flag: Munchkin mode
``p``  Toggle flag: Prep mode
====== ========================================

