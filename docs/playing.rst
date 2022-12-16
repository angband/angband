.. _Playing the Game:

================
Playing the Game
================

Most of your interaction with Angband will take the form of "commands".
Every Angband command consists of an "underlying command" plus a variety of
optional or required arguments, such as a repeat count, a direction, or the
index of an inventory object. Commands are normally specified by typing a
series of keypresses, from which the underlying command is extracted, along
with any encoded arguments. You may choose how the standard "keyboard keys"
are mapped to the "underlying commands" by choosing one of the two standard
"keysets", the "original" keyset or the "roguelike" keyset.

The original keyset is very similar to the "underlying" command set, with a
few additions (such as the ability to use the numeric "directions" to
"walk" or the ``5`` key to "stay still"). The roguelike keyset provides
similar additions, and also allows the use of the
``h``/``j``/``k``/``l``/``y``/``u``/``b``/``n`` keys to "walk" (or, in
combination with the shift or control keys, to run or alter), which thus
requires a variety of key mappings to allow access to the underlying
commands used for walking/running/altering. In particular, the "roguelike"
keyset includes many more "capital" and "control" keys, as shown below.

Note that any keys that are not required for access to the underlying
command set may be used by the user to extend the "keyset" which is being
used, by defining new "keymaps". To avoid the use of any "keymaps", press
backslash (``\``) plus the "underlying command" key. You may enter
"control-keys" as a caret (``^``) plus the key (so ``^`` + ``p`` yields
'^p').

Some commands allow an optional "repeat count", which allows you to tell
the game that you wish to do the command multiple times, unless you press a
key or are otherwise disturbed. To enter a "repeat count", type ``0``,
followed by the numerical count, followed by the command. You must type
'space' before entering certain commands. Skipping the numerical count
yields a count of 99. An option allows certain commands (open, disarm,
alter, etc) to auto-repeat.

Some commands will prompt for extra information, such as a direction, an
inventory or equipment item, a spell, a textual inscription, the symbol of
a monster race, a sub-command, a verification, an amount of time, a
quantity, a file name, or various other things. Normally you can hit return
to choose the "default" response, or escape to cancel the command entirely.

Some commands will prompt for a spell or an inventory item. Pressing space
(or ``*``) will give you a list of choices. Pressing ``-`` (minus) selects
the item on the floor. Pressing a lowercase letter selects the given item.
Pressing a capital letter selects the given item after verification.
Pressing a numeric digit ``#`` selects the first item (if any) whose
inscription contains '@#' or '@x#', where ``x`` is the current
"underlying command". You may only specify items which are "legal" for the
command. Whenever an item inscription contains '!*' or '!x' (with ``x``
as above) you must verify its selection.

Some commands will prompt for a direction. You may enter a "compass"
direction using any of the "direction keys" shown below. Sometimes, you may
specify that you wish to use the current "target", by pressing ``t`` or
``5``, or that you wish to select a new target, by pressing ``*`` (see
"Target" below).

        Original Keyset Directions
                 =  =  =
                 7  8  9
                 4     6
                 1  2  3
                 =  =  =

        Roguelike Keyset Directions
                 =  =  =
                 y  k  u
                 h     l
                 b  j  n
                 =  =  =

Each of the standard keysets provides some short-cuts over the "underlying
commands". For example, both keysets allow you to "walk" by simply pressing
an "original" direction key (or a "roguelike" direction key if you are
using the roguelike keyset), instead of using the "walk" command plus a
direction. The roguelike keyset allows you to "run" or "alter" by simply
holding the shift or control modifier key down while pressing a "roguelike"
direction key, instead of using the "run" or "alter" command plus a
direction. Both keysets allow the use of the ``5`` key to "stand still",
which is most convenient when using the original keyset.

Original Keyset Command Summary
===============================

====== ============================= ====== ============================
``a``  Aim a wand                    ``A``  Activate an object
``b``  Browse a book                 ``B``  (unused)
``c``  Close a door                  ``C``  Display character sheet
``d``  Drop an item                  ``D``  Disarm a trap or lock a door
``e``  List equipped items           ``E``  Eat some food
``f``  Fire an item                  ``F``  Fuel your lantern/torch
``g``  Get objects on floor          ``G``  Gain new spells/prayers
``h``  Fire default ammo at target   ``H``  (unused)
``i``  List contents of pack         ``I``  Inspect an item
``j``  (unused)                      ``J``  (unused)
``k``  Ignore an item                ``K``  Toggle ignore
``l``  Look around                   ``L``  Locate player on map
``m``  Cast a spell                  ``M``  Display map of entire level
``n``  Repeat previous command       ``N``  (unused)
``o``  Open a door or chest          ``O``  (unused)
``p``  (unused)                      ``P``  (unused)
``q``  Quaff a potion                ``Q``  Kill character & quit
``r``  Read a scroll                 ``R``  Rest for a period
``s``  Steal (rogues only)           ``S``  See abilities
``t``  Take off equipment            ``T``  Dig a tunnel
``u``  Use a staff                   ``U``  Use an item
``v``  Throw an item                 ``V``  Display version info
``w``  Wear/wield equipment          ``W``  Walk into a trap
``x``  (unused)                      ``X``  (unused)
``y``  (unused)                      ``Y``  (unused)
``z``  Zap a rod                     ``Z``  (unused)
``!``  (unused)                      ``^a`` (special - debug command)
``@``  (unused)                      ``^b`` (unused)
``#``  (unused)                      ``^c`` (special - break)
``$``  (unused)                      ``^d`` (unused)
``%``  (unused)                      ``^e`` Toggle inven/equip window
``^``  (special - control key)       ``^f`` Repeat level feeling
``&``  (unused)                      ``^g`` Do autopickup
``*``  Target monster or location    ``^h`` (unused)
``(``  (unused)                      ``^i`` (special - tab)
``)``  Dump screen to a file         ``^j`` (special - linefeed)
``{``  Inscribe an object            ``^k`` (unused)
``}``  Uninscribe an object          ``^l`` Center map
``[``  Display visible monster list  ``^m`` (special - return)
``]``  Display visible object list   ``^n`` (unused)
``-``  (unused)                      ``^o`` Show previous message
``_``  Enter store                   ``^p`` Show previous messages
``+``  Alter grid                    ``^q`` (unused)
``=``  Set options                   ``^r`` Redraw the screen
``;``  Walk (with pickup)            ``^s`` Save and don't quit
``:``  Take notes                    ``^t`` (unused)
``'``  Target closest monster        ``^u`` (unused)
``"``  Enter a user pref command     ``^v`` (unused)
``,``  Stay still (with pickup)      ``^w`` (special - wizard mode)
``<``  Go up staircase               ``^x`` Save and quit
``.``  Run                           ``^y`` (unused)
``>``  Go down staircase             ``^z`` (unused)
``\``  (special - bypass keymap)     ``~``  Check knowledge
 \`    (special - escape)            ``?``  Display help
``/``  Identify symbol
``|``  List contents of quiver
====== ============================= ====== ============================

Roguelike Keyset Command Summary
================================

======= ============================= ====== ============================
 ``a``  Zap a rod (Activate)          ``A``  Activate an object
 ``b``  (walk - south west)           ``B``  (run - south west)
 ``c``  Close a door                  ``C``  Display character sheet
 ``d``  Drop an item                  ``D``  Disarm a trap or lock a door
 ``e``  List equipped items           ``E``  Eat some food
 ``f``  (unused)                      ``F``  Fuel your lantern/torch
 ``g``  Get objects on floor          ``G``  Gain new spells/prayers
 ``h``  (walk - west)                 ``H``  (run - west)
 ``i``  List contents of pack         ``I``  Inspect an item
 ``j``  (walk - south)                ``J``  (run - south)
 ``k``  (walk - north)                ``K``  (run - north)
 ``l``  (walk - east)                 ``L``  (run - east)
 ``m``  Cast a spell                  ``M``  Display map of entire level
 ``n``  (walk - south east)           ``N``  (run - south east)
 ``o``  Open a door or chest          ``O``  Toggle ignore
 ``p``  (unused)                      ``P``  Browse a book
 ``q``  Quaff a potion                ``Q``  Kill character & quit
 ``r``  Read a scroll                 ``R``  Rest for a period
 ``s``  Steal (rogues only)           ``S``  See abilities
 ``t``  Fire an item                  ``T``  Take off equipment
 ``u``  (walk - north east)           ``U``  (run - north east)
 ``v``  Throw an item                 ``V``  Display version info
 ``w``  Wear/wield equipment          ``W``  Locate player on map (Where)
 ``x``  Look around                   ``X``  Use an item
 ``y``  (walk - north west)           ``Y``  (run - north west)
 ``z``  Aim a wand (Zap)              ``Z``  Use a staff (Zap)
 ``!``  (unused)                      ``^a`` (special - debug command)
 ``@``  Center map                    ``^b`` (alter - south west)
 ``#``  (unused)                      ``^c`` (special - break)
 ``$``  (unused)                      ``^d`` Ignore an item
 ``%``  (unused)                      ``^e`` Toggle inven/equip window
 ``^``  (special - control key)       ``^f`` Repeat level feeling
 ``&``  (unused)                      ``^g`` Do autopickup
 ``*``  Target monster or location    ``^h`` (alter - west)
 ``(``  (unused)                      ``^i`` (special - tab)
 ``)``  Dump screen to a file         ``^j`` (alter - south)
 ``{``  Inscribe an object            ``^k`` (alter - north)
 ``}``  Uninscribe an object          ``^l`` (alter - east)
 ``[``  Display visible monster list  ``^m`` (special - return)
 ``]``  Display visible object list   ``^n`` (alter - south east)
 ``-``  Walk into a trap              ``^o`` Show previous message
 ``_``  Enter store                   ``^p`` Show previous messages
 ``+``  Alter grid                    ``^q`` (unused)
 ``=``  Set options                   ``^r`` Redraw the screen
 ``;``  Walk (with pickup)            ``^s`` Save and don't quit
 ``:``  Take notes                    ``^t`` Dig a tunnel
 ``'``  Target closest monster        ``^u`` (alter - north east)
 ``"``  Enter a user pref command     ``^v`` Repeat previous command
 ``,``  Run                           ``^w`` (special - wizard mode)
 ``<``  Go up staircase               ``^x`` Save and quit
 ``.``  Stay still (with pickup)      ``^y`` (alter - north west)
 ``>``  Go down staircase             ``^z`` (unused)
 ``\``  (special - bypass keymap)     ``~``  Check knowledge
  \`    (special - escape)            ``?``  Display help
 ``/``  Identify symbol
``TAB`` Fire default ammo at target
 ``|``  List contents of quiver
======= ============================= ====== ============================

Special Keys
============
 
Certain special keys may be intercepted by the operating system or the host
machine, causing unexpected results. In general, these special keys are
control keys, and often, you can disable their special effects.

If you are playing on a UNIX or similar system, then 'Ctrl-c' will
interrupt Angband. The second and third interrupt will induce a warning
bell, and the fourth will induce both a warning bell and a special message,
since the fifth will quit the game, after killing your character. Also,
'Ctrl-z' will suspend the game, and return you to the original command
shell, until you resume the game with the 'fg' command. There is now a
compilation option to force the game to prevent the "double 'ctrl-z'
escape death trick". The 'Ctrl-\\' and 'Ctrl-d' and 'Ctrl-s' keys
should not be intercepted.
 
It is often possible to specify "control-keys" without actually pressing
the control key, by typing a caret (``^``) followed by the key. This is
useful for specifying control-key commands which might be caught by the
operating system as explained above.

Pressing backslash (``\``) before a command will bypass all keymaps, and
the next keypress will be interpreted as an "underlying command" key,
unless it is a caret (``^``), in which case the keypress after that will be
turned into a control-key and interpreted as a command in the underlying
Angband keyset. The backslash key is useful for creating actions which are
not affected by any keymap definitions that may be in force, for example,
the sequence ``\`` + ``.`` + ``6`` will always mean "run east", even if the
``.`` key has been mapped to a different underlying command.

The ``0`` and ``^`` and ``\`` keys all have special meaning when entered at
the command prompt, and there is no "useful" way to specify any of them as
an "underlying command", which is okay, since they would have no effect.

For many input requests or queries, the special character 'ESCAPE' will
abort the command. The '[y/n]' prompts may be answered with ``y`` or
``n``, or 'escape'. The '-more-' message prompts may be cleared (after
reading the displayed message) by pressing 'ESCAPE', 'SPACE',
'RETURN', 'LINEFEED', or by any keypress, if the 'quick_messages'
option is turned on.
 
Command Counts
==============
 
Some commands can be executed a fixed number of times by preceding them
with a count. Counted commands will execute until the count expires, until
you type any character, or until something significant happens, such as
being attacked. Thus, a counted command doesn't work to attack another
creature. While the command is being repeated, the number of times left to
be repeated will flash by on the line at the bottom of the screen.

To give a count to a command, type 0, the repeat count, and then the
command. If you want to give a movement command and you are using the
original command set (where the movement commands are digits), press space
after the count and you will be prompted for the command.
 
Counted commands are very useful for time consuming commands, as they
automatically terminate on success, or if you are attacked. You may also
terminate any counted command (or resting or running), by typing any
character. This character is ignored, but it is safest to use a 'SPACE'
or 'ESCAPE' which are always ignored as commands in case you type the
command just after the count expires.

You can tell Angband to automatically use a repeat count of 99 with
commands you normally want to repeat (open, disarm, tunnel, bash, alter,
etc) by setting the 'always_repeat' option.
  
Selection of Objects
====================
 
Many commands will also prompt for a particular object to be used.
For example, the command to read a scroll will ask you which of the
scrolls that you are carrying that you wish to read.  In such cases, the
selection is made by typing a letter of the alphabet (or a number if choosing
from the quiver).  The prompt will indicate the possible letters/numbers,
and you will also be shown a list of the appropriate items.  Often you will
be able to press ``/`` to switch between inventory and equipment, or ``|`` to
select the quiver, or ``-`` to select the floor.  Using the right arrow also
rotates selection between equipment, inventory, quiver, floor and back to
equipment; the left arrow rotates in the opposite direction.
 
The particular object may be selected by an upper case or a lower case
letter. If lower case is used, the selection takes place immediately. If
upper case is used, then the particular option is described, and you are
given the option of confirming or retracting that choice. Upper case
selection is thus safer, but requires an extra key stroke.

Shape Changes
=============

Some classes, objects, or races may allow your character to change shape:
becoming, for instance, a fox or a wolf.  While in the alternate shape,
your character will not have access to items in the pack or quiver and
will not be able to access items on the floor except for eating or pickup.
The items your character was wearing upon changing shape will remain
equipped and continue to affect the character's statistics, resistances,
number of blows, and damage.  Your character will not be able to activate
any equipped items while in the alternate shape.  To have your character
change back to normal, cast a spell or use one of the commands, like drop,
that uses an item.
