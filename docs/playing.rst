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
key or are :ref:`otherwise disturbed <disturb-player>`. To enter a
"repeat count", type ``0``, followed by the numerical count, followed by the
command. You must type 'space' before entering certain commands. Skipping
the numerical count yields a count of 99 for the open, tunnel, disarm,
alter, close, aim a wand, zap a rod, and activate equipment commands.  The
generic use an item command also defaults to having a repeat count of 99
when it is used with a wand, staff, or rod. All other commands do not repeat
unless requested.

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

.. index::
   single: original keyset; directions
   single: roguelike keyset; directions

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

.. index::
   single: original keyset
   see: keyset; original keyset

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
``p``  (normally unused; see note)   ``P``  (unused)
``q``  Quaff a potion                ``Q``  Retire character & quit
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
``_``  (unused)                      ``^p`` Show previous messages
``+``  Alter grid                    ``^q`` (unused)
``=``  Set options                   ``^r`` Redraw the screen
``;``  Walk (with pickup)            ``^s`` Save and don't quit
``:``  Take notes                    ``^t`` (unused)
``'``  Target closest monster        ``^u`` (unused)
``"``  Enter a user pref command     ``^v`` (unused)
``,``  Stay still (with pickup)      ``^w`` (special - wizard mode)
``<``  Go up staircase (see note)    ``^x`` Save and quit
``.``  Run                           ``^y`` (unused)
``>``  Go down staircase (see note)  ``^z`` Borg commands (if available)
``\``  (special - bypass keymap)     ``~``  Check knowledge
 \`    (special - escape)            ``?``  Display help
``/``  Identify symbol
``|``  List contents of quiver
====== ============================= ====== ============================

.. index::
   single: autoexplore; original keyset

Note that the ``<``, ``>``, and ``p`` commands are affected by the
autoexplore_commands option (see
:ref:`Autoexplore Commands Option <autoexplore-commands-option>`). When that
option is off (that is the default), the commands act as described above.
When that option is on, ``<`` or ``>`` will use the staircase at the player's
location if it is the appropriate kind of staircase or will move to the
nearest known staircase of the appropriate kind if the player is not already
at that kind of staircase. ``p`` will move to the nearest unexplored location
when the autoexplore_commands option is on.

.. index::
   single: roguelike keyset
   see: keyset; roguelike keyset

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
 ``p``  (normally unused; see note)   ``P``  Browse a book
 ``q``  Quaff a potion                ``Q``  Retire character & quit
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
 ``_``  (unused)                      ``^p`` Show previous messages
 ``+``  Alter grid                    ``^q`` (unused)
 ``=``  Set options                   ``^r`` Redraw the screen
 ``;``  Walk (with pickup)            ``^s`` Save and don't quit
 ``:``  Take notes                    ``^t`` Dig a tunnel
 ``'``  Target closest monster        ``^u`` (alter - north east)
 ``"``  Enter a user pref command     ``^v`` Repeat previous command
 ``,``  Run                           ``^w`` (special - wizard mode)
 ``<``  Go up staircase (see note)    ``^x`` Save and quit
 ``.``  Stay still (with pickup)      ``^y`` (alter - north west)
 ``>``  Go down staircase (see note)  ``^z`` Borg commands (if available)
 ``\``  (special - bypass keymap)     ``~``  Check knowledge
  \`    (special - escape)            ``?``  Display help
 ``/``  Identify symbol
``TAB`` Fire default ammo at target
 ``|``  List contents of quiver
======= ============================= ====== ============================

.. index::
   single: autoexplore; roguelike keyset

Note that the ``<``, ``>``, and ``p`` commands are affected by the
autoexplore_commands option (see
:ref:`Autoexplore Commands Option <autoexplore-commands-option>`). When that
option is off (that is the default), the commands act as described above. When
that option is on, ``<`` or ``>`` will use the staircase at the player's
location if it is the appropriate kind of staircase or will move to the
nearest known staircase of the appropriate kind if the player is not already
at that kind of staircase. ``p`` will move to the nearest unexplored location
when the autoexplore_commands option is on.

.. _disturb-player:
.. index::
   single: disturb; details

Disturb
=======

For commands that repeat, like resting, tunnneling or running, certain events
will disturb the player.  That stops the command, flushes queued input from
the keyboard or mouse, and drops any unprocessed keys from a partially
processed keymap.  The events that disturb the player are:

* The command reached its goal:  the wall was tunnelled, the door or chest was opened, the specific criteria for resting was met.
* While running, following a precomputed path, resting, or executing a repeated command (either with an explicit repeat count or one that repeats automatically if no repeat count was specified), a keypress or, if the front end supports it, a mouse button event, interrupts the command by disturbing the player.  Since the keypress could come from a keymap, such commands will have to be last in the keymap (no further keystrokes in the action besides what the command would normally consume) to have the intended effect.  While resting, checking for an input event that disturbs the rest only happens every 128 turns.
* A monster's melee attack hits the player, regardless of whether any damage was done.
* A monster uses a ranged attack or spell, regardless of whether the attack hits.
* A monster that is visible and in the line of sight and either moves or performs a melee attack on the player will disturb if the :ref:`Disturb whenever viewable monster moves <disturb-near-option>` is on.  With that option on, a monster that enters the line of sight and is visible or leaves the line of sight while visible, also disturbs the player.
* A monster smashes open a door.
* The player loses hit points.
* The player makes a melee attack.
* A projection affects the player, regardless of whether it does damage.
* Changes to a timed effect, like confusion, can disturb the player.  If the change was initiated by the player by casting a spell or using an item, the change will not disturb the player with the exception for when an unidentified item is used.  For effects that have more than two gradations, like cuts, stunning, and the hunger meter, going up a grade will disturb the player and going down a grade will disturb if the effect has a message for entering the grade from a higher one.  For effects that have two gradations (off and on), going from off to on will disturb the player.  Otherwise, the change will disturb if the effect in question does not duplicate a state the player has from another source and the change is not due to decreasing the effect's duration because time passed.
* The player tries to move into impassable terrain.
* While following a precomputed path or on the second or later step of running, a visible object, including gold, or visible monster in the next grid stops running.
* While following a precomputed path or on the second or later step of running, the next grid contains a known trap and the player is not immune to traps.
* If on the second or later step of a run or following a precomputed path, the player's current grid is in an area that has had trap detection cast on it but the next grid has not had trap detection cast on it.
* While following a precomputed path, a known impassable grid that cannot be autmatically dealt with as the next step on the path disturbs the player.
* The player triggers a trap.  That disturbs even if the player passed the saving throw for the trap's effects.
* The player finds a secret door or a trap on a chest.
* The player faints because of hunger.
* The player's pack overflows.
* A curse on an equipped item triggers.
* The delayed effect of deep descent or a word of recall activates.
* The player's light source has run out of fuel or is close to running out of fuel and a new "growing faint" reminder message is displayed.
* An item that is equipped or in the pack has recharged and that item's inscription includes '!!' or the :ref:`notify on object recharge <notify-recharge-option>` is on.
* The player is on a visible stack of objects that are not all gold and a scan for autopickup happens (either by holding with autopickup, explicitly invoking autopickup, or having just moved to the grid).
* The player enters a store.
* The player enters a new level.

Special Keys
============
 
Certain special keys may be intercepted by the operating system or the host
machine, causing unexpected results. In general, these special keys are
control keys, and often, you can disable their special effects.

If you are playing on a UNIX or similar system, then 'Ctrl-c' will
interrupt Angband. The second and third interrupt will induce a warning
bell, and the fourth will induce both a warning bell and a special message,
since the fifth will either quit without saving (if Angband was compiled
without the SETGID option which puts the save files in a shared location for
all users) or kill your character (if Angband was compiled with the SETGID
option). Also, 'Ctrl-z' will suspend the game, and return you to the original
command shell, until you resume the game with the 'fg' command. The 'Ctrl-\\'
and 'Ctrl-d' and 'Ctrl-s' keys should not be intercepted.
 
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
you type any character, or until
:ref:`something significant happens <disturb-player>`, such as
being attacked. Thus, a counted command doesn't work to attack another
creature. While the command is being repeated, the number of times left to
be repeated will flash by on the line at the bottom of the screen.

To give a count to a command, type 0, the repeat count, and then the
command. If you want to give a movement command and you are using the
original command set (where the movement commands are digits), press space
after the count and you will be prompted for the command.  The open, tunnel,
disarm, alter, close, aim a wand, use a staff, zap a rod, and activate
equipment commands default to having a repeat count of 99.  The generic
use an item command also defaults to having a repeat count of 99 when it is
used with a wand, staff, rod, or equipped item.  All other commands default
to not repeating at all.
 
Counted commands are very useful for time consuming commands, as they
automatically terminate on success, or if you are attacked. You may also
terminate any counted command (or resting or running), by typing any
character. This character is ignored, but it is safest to use a 'SPACE'
or 'ESCAPE' which are always ignored as commands in case you type the
command just after the count expires.

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

.. _targeting:
.. index::
   single: targeting; details

Targeting
=========

You will use the targeting interface when you use the targeting command, ``*``,
the look command, ``l`` or ``x``, or the options in context menus to look
at a grid.  When other commands prompt for a target, they will automatically
use the current target, if there is one, when the
:ref:`Use old target by default <old-target-option>` option is on.  Otherwise,
you will be asked for a target:  ESCAPE or a mouse click with the second mouse
button breaks out of the prompt; ``5``, if there is a current target, will use
that target; ``'`` will target the nearest monster; a direction key targets
that direction rather than a specific monster or location; ``*`` or a mouse
click on the map with the first mouse button will bring up the targeting
interface to select a new target.

When the targeting interface starts, it always clears the current target.  So,
in the cases when you want to forget the current target, bring up the targeting
interface, ``*``, and then break out if it with ``q`` or ESCAPE.

The targeting interface maintains a list of locations that it thinks you will
find interesting.  When started by the targeting command, ``*``, or from a
command prompting for a target, that list only contains locations with monsters
that could be reached by a "projectable" spell, missiles, or thrown objects.
When started from the look command, the list includes locations with monsters,
regardless of whether they could be hit by a spell or missile, the player's
location, objects, traps, or interesting terrain (staircases, doors, rubble,
or stores).  The list of interesting locations can be empty if there is
nothing appropriate nearby.

The targeting interface has two modes:  one, interested targeting, restricts
the grid under the targeting cursor to be one of the interesting locations and
another, free targeting, that allows the cursor to move to any grid.  When
started without a specified location (invoked by the targeting command, the
look command, or by answering another commmand's targeting prompt with ``*``),
the targeting interface will use interested targeting if there is any
interesting locations (that is always the case when started from the look
command since the player's location is interesting) or free targeting when
there are no interesting locations.  When started with a specified location
(by answering a command's target prompt with a mouse click or when invoked
from a context menu), the targeting interface uses free targeting.  From
within the targeting interface, you can explicitly change the targeting mode
by these keys:

``o``
  Switch to free targeting.  Leave the cursor at its previous location.

``p``
  Switch to free targeting.  Move the cursor to the player's location.

``m``
  This only does something when in free targeting and there are interesting
  locations.  If that is the case, switch to interested targeting and move
  the cursor to the interesting location nearest to the cursor's location.

Some keys or mouse input can implicitly change the targeting mode:

``<``, ``>``, ``x``
  These are described further below. If the new location selected by one of
  those actions is interesting, switch to interested targeting.  If the new
  location is not interesting, switch to free targeting.

Mouse click with the first mouse button
  This will be described further below.  Shifts the location of the cursor.
  If the new location is interesting, switch to interested targeting;
  otherwise, switch to free targeting.

The targeting interface will display a description of what is at the targeting
cursor.  That will either be a very brief summary, or, for a monster or object,
a more elaborate description.  You can toggle between those by pressing ``r``
or clicking with the first mouse button on the cursor's location in the map.
When the target interface is first started or the cursor moves to a new
location, what is described will be the highest priority thing present.  A
monster has the highest priority, followed by a trap, object, and the
underlying terrain.  To cycle through the descriptions of the other things
in the grid, press ENTER.

At any point, you can break out of the targeting interface and not select
a target by pressing ``q`` or ESCAPE, or, with the mouse, use the second
mouse button and click on a position that is not the cursor's current position.

Use the following keys to operate on what is at the targeting cursor:

``t``, ``5``, ``0``, or ``.``
  In free targeting, sets the target to the location at the cursor
  and breaks out of the targeting interface.  Targeting a location is
  slightly "dangerous", as the target is maintained even if you are far
  away.  In interested targeting, does nothing besides trigger a warning
  bell when the location does not have a targetable monster.  When the
  location has a targetable monster, sets the target to that monster
  and breaks out of the targeting interface.

``g``
  Does nothing if the target interface was invoked by the targeting prompt for
  a command.  Otherwise, break out of the targeting interface and initiate
  :ref:`pathfinding <pathfinding-player>` to move the player to that location.

``k`` (original keyset) or ``^d`` (roguelike keyset)
  Does nothing when the targeting interface was invoked by the targeting command
  or another command's targeting prompt or the currently tracked object is
  not at the cursor's position.  Otherwise, bring up a menu to change the
  ignore settings for the tracked object.

Use the following keys to change the position of the targeting cursor:

direction keys
  In interested targeting, move to an interesting location that approximately
  lies in the key's direction from the current cursor location.  In free
  targeting, move (by ten grids if the key is a trigger for a running keymap;
  by one grid otherwise) the cursor in the key's direction.

space or ``+``
  Does nothing in free targeting.  In interested targeting, go to the next
  interesting location.

``-``
  Does nothing in free targeting.  In interested targeting, go to the previous
  interesting location.

``<``
  Look for the nearest, by number of turns needed to move there, down
  staircase.  If there is one, move the targeting cursor there.  As noted
  above, can change the targeting mode.

``>``
  Look for the nearest, by number of turns needed to move there, up
  staircase.  If there is one, move the targeting cursor there.  As noted
  above, can change the targeting mode.

``x``
  Look for the nearest, by number of turns needed to move there, passable
  grid that has an unknown neighbor.  If one is not found, look for the
  nearest passable grid that is next to a closed door or impassable rubble
  and that door or rubble has an unknown neighbor.  If a grid was found,
  move the targeting cursor there.  As noted above, can change the targeting
  mode.

Besides the previously mentioned ways of using the mouse, you also use the
mouse as follows from the targeting interface:

Click with first mouse button
  Moves the targeting cursor.  If the click is on the edge of the map, the
  new location is one past the edge, coerced to the bounds of the level.
  That can shift the map.  Otherwise, the new location is the click's location
  coerced to the bounds of the level.  As noted above, can change the
  targeting mode.

Click with second mouse button
  Breaks out of targeting.  If using free targeting and the targeting interface
  was invoked from the targeting command or another command's targeting prompt,
  will set the target to the cursor's location if the click is on the cursor.

Control + click with second mouse button
  In free targeting, targets the click's location and breaks out of targeting.
  In interested targeting, does nothing besides trigger a warning bell if the
  click's location does not have a targetable monster.  Otherwise, targets
  the monster and breaks out of targeting.

Alt (Option with the Mac front end) + click with second mouse button
  Does nothing if the target interface was invoked by the targeting prompt for
  a command.  Otherwise, break out of the targeting interface and initiate
  :ref:`pathfinding <pathfinding-player>` to move the player to the click's
  location.

When targeting, it can sometimes be useful to target a grid or monster that
is not the target you intend to hit.  The direct path a missile would follow
to the monster could be blocked by a wall or other obstacle.  The path to
the other location could avoid that obstacle while still hitting the intended
target.  When invoked from the targeting command or another command's targeting
prompt, the targeting interface will show the path a beam spell or missile
would follow to the targeting cursor and beyond with coloring to indicate
what is known about the path:

dark gray
  used for any grid whose terrain is unknown and the grids beyond it; because
  of the unknown terrain, what happens to a missile or spell is unclear

red
  known monster; would be hit by and block a bolt spell or missile that gets
  that far

yellow
  known and not ignored object

blue
  terrain that would block a spell or missile

white
  no known monster or object; terrain does not affect a spell or missile

Targeting for a ball spell or breath weapon is another case where you may
want to target a location other than the most dangerous monster.  Aiming
a ball spell at the center of a group of monsters can be more effective than
having the ball target a monster in that group.  Also with a ball spell,
targeting a location near where a wall bends away can be way of attacking
monsters shielded by that wall that are safe from other attacks.

.. _pathfinding-player:
.. index::
   single: pathfinding; details

Pathfinding
===========

If you use 'g' from targeting or looking to move to a location, click with
the first mouse button on the map, click with the second mouse button on the
map and select the option to pathfind, or use '<', '>', or 'p' when the
:ref:`Autoexplore Commands Option <autoexplore-commands-option>` is on, the
game computes a path to the destination and then launches you along that path.
The computation of the path looks for a path which will take the minimum amount
of turns to traverse.  In doing so, it uses your current state to compute how
many turns would be needed to open a locked door or tunnel through impassable
rubble.  It optimistically assumes that any grid with unknown terrain is
passable.  It pessimistically assumes that any locked doors have the hardest
to open locks since the game does not indicate the difficulty of the lock when
you look at the door.  The game considers paths in this order using the first
path that is feasible given your knowledge of the map:

#. Look for paths that only go through know terrain without known traps.  Skip this class of paths if the terrain in the starting grid or destination is unknown or you are immune to traps.  When not finding a path for the autoexploration commands, also skip this class if the destination contains a known trap.
#. Look for paths that only go through known terrain.  The paths can cross known traps.  Skip this class of paths if the terrain in the starting grid or destination is unknown.  When not finding a path for the autoexploration command and the previous class of paths was considered and did not reject a potentially shorter path that has a known trap, also skip this class.
#. Look for paths that can pass through either known or unknown terrain and do contain known traps.  Skip this class of paths if you are immune to traps or the destination contains a known trap and not finding a path for the autoexploration commands.
#. Look for paths that can pass through known or unknown terrain.  The paths can cross known traps.  Skip this class of paths when not finding a path for the autoexploration commands and the previous class of paths was considered and did reject a potentially shorter path that has a known trap.

Note that magic mapping and enlightment do not mark floor grids as known
terrain.  So, unless you have also also seen a floor that was revealed by
magic mapping or enlightment, pathfinding treats that floor as unknown terrain.
Paths are never allowed to cross passable, but damaging terrain, like lava.

When the game move you along the selected path, you can interrupt the process
with a keypress (space or ESCAPE are recommended as they can not be interpreted
as a command) or, when the front end supports it, pressing a mouse button.  The
process will also be interrupted by other events that
:ref:`disturb <disturb-player>`.  If following the path was interrupted and
you have not issued another command, repeating the previous command (``n`` in
the original keyset, ``^v`` in the roguelike keyset) will recompute the path
(if the path was from '<', '>', or 'p', it will find the nearest up staircase,
down staircase, or unknown location, respectively, and compute a path to it
from your current location; otherwise, it will compute a path to the original
destination from your current location) and start along that path.

Following a path can automatically open a door or dig out impassable rubble
that is in the path.  It will only do so if you know the terrain in all the
neighbors of the door or rubble.  If any neighbors have unknown terrain, you
stop at the grid before the door or rubble.

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
