====================
Customising the game
====================

Angband allows you to change various aspects of the game to suit your tastes.  These include:

* Options - which let you change interface or gameplay behaviour
* `Ignoring items`_ and `inscribing items`_ to change how the game treats them
* `Showing extra info in subwindows`_
* `Keymaps`_ - a way to assign commonly-used actions to specific keys
* `Visuals`_ - allowing you to change the appearance of in-game entities like objects and monsters
* `Colours`_ - allowing you to make a given color brighter, darker, or even completely different
* `Interface details`_ - depending on which interface to the game you use, these give you control over the font, window placement, and graphical tile set

Except for the options, which are linked to the save file, and interface
details, that are handled by the front end rather than the core of the game,
you can save your preferences for these into files, which are called
`user pref files`.  For the options, customize those using the ``=`` command
while playing.


User Pref Files
===============

User pref files are Angband's way of saving and loading certain settings.  They can store:

* Altered visual appearances for game entities
* Inscriptions to automatically apply to items
* Keymaps
* Altered colours
* Subwindow settings
* Colours for different types of messages
* What audio files to play for different types of messages

They are simple text files with an easy to modify format, and the game has a set of pre-existing pref files in the lib/customize/ folder.  It's recommended you don't modify these.

Several options menu (``=``) items allow you to load existing user pref files, create new user pref files, or save to a user pref file.

Where to find them
~~~~~~~~~~~~~~~~~~

On macOS, you can find them in your user directory, in ``Documents/Angband/``.

On Linux, they will be stored in ``~/.angband/Angband``.

On Windows you can find them in ``lib/user/``.

How do they get loaded?
~~~~~~~~~~~~~~~~~~~~~~~

When the game starts up, after you have loaded or created a character, some user pref files are loaded automatically.  These are the ones mentioned above in the ``lib/customize/`` folder, namely ``pref.prf`` followed by ``font.prf``.  If you have graphics turned on, then the game will also load some settings from ``lib/tiles/``.

After these are complete, the game will try to load (in order):

* ``Race.prf`` - where race is your character's race
* ``Class.prf`` - where class if your character's class
* ``Name.prf`` - where name is your character's name

So, you can save some settings - for example, keymaps - to the ``Mage.prf`` file if you only want them to be loaded for mages.

You may also enter single user pref commands directly, using the special "Enter a user pref command" command, activated by pressing ``"``.

You may have to use the redraw command (``^R``) after changing certain of the aspects of the game to allow Angband to adapt to your changes.


Ignoring items
==============

Angband allows you to ignore specific items that you don't want to see anymore. These items are marked 'ignored' and any similar items are hidden from view. The easiest way to ignore an item is with the ``k`` (or ``^D``) command; the object is dropped and then hidden from view.  When ignoring an object, you will be given a choice of ignoring just that object, or all objects like it in some way.

The entire ignoring system can also be accessed from the options menu (``=``) by choosing ``i`` for ``Item ignoring setup``.  This allows ignore settings for non-wearable items, and quality and ego ignore settings (described below) for wearable items, to be viewed or changed.

There is a quality setting for each wearable item type. Ignoring a wearable item will prompt you with a question about whether you wish to ignore all of that type of item with a certain quality setting, or of an ego type, or both.

The quality settings are:

bad
  The weapon/armor has negative AC, to-hit or to-dam.

average
  The weapon/armor has no pluses no minuses.  It is non-magical.

good
  The weapon/armor has positive AC, to-hit or to-dam. However it does not
  have any special abilities, brands, slays, stat-boosts, resistances

non-artifact
  This setting only leaves artifacts unignored.


Inscribing items
================

Inscriptions are notes you can mark on objects using the ``{`` command.  You can use this to give the game commands about the object, which are listed below. You can also set up the game to automatically inscribe certain items whenever you find them, using the object knowledge screens, accessed using ``~``.

Inscribing an item with '!!':
	This will alert you when the item has finished recharging.

Inscribing an item with '=g':
	This marks an item as 'always pick up'.  This is sometimes useful for
	picking up ammunition after a shootout.  If there is a number
	immediately after the 'g', then the amount picked up automatically
	will be limited.  If you have inscribed a spellbook with '=g4' and have
	four or more copies in your pack, you will not automatially pick up
	any more copies when you have the 'pickup if in inventory' option
	enabled.  If you have three copies in your pack with that inscription
	and happen to find a pile of two copies, you'll automatically pick up
	one so there is four in the pack.

Inscribing an item with ``!`` followed by a command letter or ``*``:
	This means "ask me before using this item".  '!w' means 'ask me before
	wielding', '!d' means 'ask me before dropping', and so on.  If you
	inscribe an item with '!*' then the game will confirm any use of an
	item.

	Say you inscribed your potion of Speed with '!q'.  This would prompt
	you when you try to drink it to see if you really mean to.  Multiple
	'!q' inscriptions will prompt multiple times.

	Similarly, using !v!k!d makes it very hard for you to accidentally
	throw, ignore or put down the item it is inscribed on.

	Some adventurers use this for Scrolls of Word of Recall so they don't
	accidentally return to the dungeon too soon.

Inscribing an item with ``@``, followed by a command letter, followed by 0-9:
	Normally when you select an item from your inventory you must enter the
	letter that corresponds to the item. Since the order of your inventory
	changes as items get added and removed, this can get annoying.  You
	can instead assign certain items numbers when using a command so that
	wherever they are in your backpack, you can use the same keypresses.
	If you have multiple items inscribed with the same thing, the game will
	use the first one.

	For example, if you inscribe a staff of Cure Light Wounds with '@u1',
	you can refer to it by pressing 1 when ``u``\sing it.  You could also
	inscribe a wand of Wonder with '@a1', and when using ``a``\, 1 would select
	that wand.

	Spellcasters should inscribe their books, so that if they lose them they
	do not cast the wrong spell.  If you are mage and the beginner's
	spellbook is the first in your inventory, casting 'maa' will cast magic
	missile. But if you lose your spellbook, casting 'maa' will cast the
	first spell in whatever new book is in the top of your inventory. This
	can be a waste in the best case scenario and exceedingly dangerous in
	the worst! By inscribing your spellbooks with '@m1', '@m2', etc., if
	you lose your first spellbook and attempt to cast magic missile by
	using 'm1a', you cannot accidentally select the wrong spellbook.

Inscribing an item with ``^``, followed by a command letter:
	When you wear an item inscribed with ``^``, the game prompts you before
	doing that action.  You might inscribe '^>' on an item if you want to
	be reminded to take it off before going down stairs.  If the item is in
	your backpack then the game won't prompt you.

	Like with ``!``, you can use ``*`` for the command letter if you want to
	game to prompt you every turn whatever you're doing.  This can get
	very annoying!


Showing extra info in subwindows
================================

In addition to the main window, you can create additional windows that have secondary information on them. You can access the subwindow menu by using ``=`` then ``w``, where you can choose what to display in which window.

You may then need to make the window visible using the "window" menu from the menu bar (if you have one in your version of the game).

There are a variety of subwindow choices and you should experiment to see which ones are the most useful for you.


Keymaps
=======

You can set up keymaps in Angband, which allow you to map a single keypress to a series of keypresses.  For example you might map the key F1 to "maa" (the keypresses to cast "Magic Missile" as a spellcaster). This can speed up access to commonly-used features.

To set up keymaps, go to the options menu (``=``) and select "Edit keymaps" (``k``).

Keymaps have two parts: the trigger key and the action.  These are written where possible just as ordinary characters.  However, if modifier keys (shift, control, etc.) are used then they are encoded as special characters within curly braces {}.

Possible modifiers are::

	K = Keypad (for numbers)
	M = Meta (Cmd-key on OS X, alt on most other platforms)
	^ = Control
	S = Shift

If the only modifier is the control key, the curly braces {} aren't included.
For example::

	{^S}& = Control-Shift-&
	^D    = Control-D

Special keys, like F1, F2, or Tab, are all written within square brackets [].
For example::

	^[F1]     = Control-F1
	{^S}[Tab] = Control-Shift-Tab

Special keys include [Escape].

The game will run keymaps in whatever keyset you use (original or roguelike). So if you write keymaps for roguelike keys and switch to original keys, they may not work as you expect!  Keymap actions aren't recursive either, so if you had a keymap whose trigger was F1, including F1 inside the action wouldn't run the keymap action again.

When you're running a keymap, you might want to automatically skip any -more- prompts.  To do this, place whatever commands you want to skip -more- prompts within between brackets: ``(`` and ``)``.

Keymaps are written in pref files as::

	A:<action>
	C:<type>:<trigger>

The action must always come first,  ```<type>``` means 'keyset type', which is either 0 for the original keyset or 1 for the roguelike keyset.  For example::

	A:maa
	C:0:[F1]

Angband uses a few built-in keymaps.  These are for the movement keys (they are mapped to ``;`` plus the number, e.g. ``5`` -> ``;5``), amongst others.  You can see the full list in pref.prf but they shouldn't impact on you in any way.

To avoid triggering a keymap for a given key, you can type the backslash (``\``) command before pressing that key.


Colours
=======

The "Interact with colors" options submenu (``=``, then ``c``) allows you to change how different colours are displayed.  Depending on what kind of computer you have, this may or may not have any effect.

The interface is quite clunky.  You can move through the colours using ``n`` for 'next colour' and ``N`` for 'previous colour'.  Then upper and lower case ``r``, ``g`` and ``b`` will let you tweak the color.  You can then save the results to user pref file.


Visuals
=======

You can change how various in-game entities are displayed using the visuals editor.  This editor is part of the knowledge menus (``~``).  When you are looking at a particular entity - for example, a monster - if you can edit its visuals, that will be mentioned in the prompt at the bottom of the screen.

If you are in graphics mode, you will be able to select a new tile for the entity.  If you are not, you will only be able to change its colours.

Once you have made edits, you can save them from the options menu (``=``).  Press ``v`` for 'save visuals' and choose what you want to save.


Interface details
=================

Some aspects of how the game is presented, notably the font, window placement
and graphical tile set, are controlled by the front end, rather than the core
of the game itself.  Each front end has its own mechanism for setting those
details and recording them between game sessions.  Below are brief descriptions
for what you can configure with the standard `X11`_, `SDL`_, `SDL2`_ and
`Mac`_ front ends.

X11
~~~

With the X11 front end, the number of windows opened is set by the '-n' option
on the command line, i.e. running ``./angband -mx11 -- -n4`` will open the
main window and subwindows one through three if the executable is in the
current working directory.  To control the font, placement, and size used for
each of the windows, set enviroment variables before running Angband.  Those
environment variables for window 'z' where 'z' is an integer between 0 (the
main window) and 7 are:

* ANGBAND_X11_FONT_z holds the name of the font to use for the window
* ANGBAND_X11_AT_X_z holds the horizontal coordinate (zero is leftmost) for the upper left corner of the window
* ANGBAND_X11_AT_Y_z holds the vertical coordinate (zero is topmost) for the upper left corner of the window
* ANGBAND_X11_COLS_z holds the number of columns to display in the window
* ANGBAND_X11_ROWS_z holds the number of rows to display in the window

SDL
~~~

With the SDL front end, the main window and any subwindows are displayed within
the application's rectangular window.  At the top of the application's window
is a status line.  Within that status line, items highlighted in yellow are
buttons that can be pressed to initiate an action.  From left to right they are:

* The application's version number - pressing it displays an information dialog about the application
* The currently selected terminal - pressing it brings up a menu for selecting the current terminal; you can also make a terminal the current one by clicking on the terminal's title bar if it is visible
* Whether or not the current terminal is visible - pressing it for any terminal that is not the main window will allow you to show or hide that terminal
* The font for the current terminal - pressing it brings up a menu to choose the font for the terminal
* Options - brings up a dialog for selecting global options including those for the graphical tile set used and whether fullscreen mode is enabled
* Quit - to save the game and exit

To move a terminal window, click on its title bar and then drag the mouse.
To resize a terminal window, position the mouse pointer over the lower right
corner.  That should cause a blue square to appear, then click and drag to
resize the terminal.

To change the graphical tile set used when displaying the game's map, press
the Options button in the status bar.  Then, in the dialog that appears, press
one of the red buttons that appear to the right of the label,
"Available Graphics:".  The last of those buttons, labeled "None", selects
text as the method for displaying the map.  Your choice for the graphical tile
set does not take effect until you press the red button labeled "OK" at the
bottom of the dialog.

When you leave the game, the current settings for the SDL interface are saved
as ``sdlinit.txt`` in the same directory as is used for preference files, see
`User Pref Files`_ for details.  Those settings will be automatically reloaded
the next time you start the SDL interface.

SDL2
~~~~

With the SDL2 front end, the application has one window that can contain the
main window and any of the subwindows.  The application may also have up to
three additional windows which can contain any of the subwindows.  A subwindow
may not appear in more than of those application windows.  Unused portions of
an application window are tiled with repetitions of the game's logo.

Each of the application windows has a menu bar along the top.  The "Menu"
entry at the left end of the menu bar has the main menu for controlling
aspects of the SDL2 interface.

Next to "Menu", are a series of one letter labels that act as toggles for the
terminal windows shown in the application window.  Click on one to toggle it
between on (drawn in white) and off (drawn in gray).  It is not possible to
toggle off the main window shown in the primary application window.

At the end of the menu bar are two toggle buttons labeled "Size" and "Move".
Each will be gray if disabled or white if enabled.  Clicking on "Size" when
it is disabled will enable it, disable "Move", turn off input to the game's
core, and cause clicks and drags within the displayed subwindows to change
the sizes for those subwindows.  Clicking on "Move" when it is disabled will
enable it, disable "Size", turn off input to the game's core, and cause clicks
and drags within the displayed subwindows to change the positions for those
subwindows.  Disable both "Move" and "Size", by clicking on one if it is
enabled, to restore passing input to the game's core.

Within "Menu", the first entries control properties each of the displayed
terminal windows within that application window.  For the main window, you
can set the font, graphical tile set, whether the window is shown with borders
or not, and whether or not the window will be shown on top of the other windows.
For subwindows, you can set the font, the purpose (which is a shortcut for
enabling the subwindow content as described in
`Showing extra info in subwindows`_), the opaqueness ("alpha") of the window,
whether the window is shown with borders or not, and whether or not the window
will be shown on top of the other windows.

Below the entries for the contained terminal windows, is an entry,
"Fullscreen" for toggling fullscreen mode for that application window.  That
entry will be gray when fullscreen mode is off and white when it is on.

In the primary application window which contains the main window, there is an
entry, "Send Keypad Modifier", after that for whether key strokes from the
numeric keypad will be sent to the game with the keypad modifier set.  That
entry will be gray when the modifier is not send and will be white when the
modifier is sent.  Sending the modifier allows some predefined keymaps to work,
for instance shift with 8 from the numeric keypad to run north, at the cost of
compatibility issues with some keyboard layouts that differ from the standard
English keyboard layout for which normal keys have equivalents on the numeric
keypad.  https://github.com/angband/angband/issues/4522 has an example of the
problems that can be avoided by not sending the keypad modifier.

Below "Send Keypad Modifier" in the primary application window's "Menu" is
"Windows", use that to bring up one of the additional application windows.

The final two entries in "Menu" are "About" for displaying an information
dialog about the game and "Quit" to save the game and exit.

When you leave the game, the current settings for the SDL interface are saved
as ``sdl2init.txt`` in the same directory as is used for preference files, see
`User Pref Files`_ for details.  Those settings will be automatically reloaded
the next time you start the SDL interface.

Mac
~~~

With the Mac-specific front end, you can use Apple's standard mechanisms to
control window placement:  click and drag on a window's title bar to move it,
click and drag on a window's edge or corner to change the window's dimensions,
and click the red button at the top left corner of a subwindow to close it.
To reopen a subwindow that you closed, use the Window menu from the Mac's
menu bar while the game is the active application and select the entry near the
bottom of that menu that corresponds to the subwindow you want to see.  For a
subwindow's entry to be enabled in the Window menu, that subwindow must be
configured to display at least one category of information:  see
`Showing extra info in subwindows`_ for details.

To change the font for a window, click on the window's title bar and select
"Edit Font" from the Settings menu in the Mac's menu bar.  That will open a
dialog which displays the family, typeface and size for the current font.
Changing the selection for any of those will change the font in the window.

Whether the game's map is displayed as text or as graphical tiles can be set
by selecting Settings from the Mac's menu bar while the game is the active
application and then choosing from one of the entries in the Graphics option.
Choosing "Classic ASCII" will display the map as text.  Any of the other options
will use some form of graphical tiles to display the map.  If you wish to
adjust how graphical tiles are scaled to match up with the currently selected
font in the main window, select 'Change Tile Set Scaling...' in the Settings
menu.

When you leave the game, the current Mac-specific settings are saved and will
be automatically reloaded when you restart.  The settings are stored in
``Library/Preferences/org.rephial.angband.plist`` within your user directory.
If you suspect those settings have been corrupted in some way or would like to
start again from the default settings, quit the game if it is running, open a
Terminal window (i.e. select 'Go->Utilities->Terminal' from the Finder's
menus), and, in that Terminal window, run this::

	defaults delete org.rephial.angband

to clear the contents of the preferences file and any cached preferences that
may be retained in memory.
