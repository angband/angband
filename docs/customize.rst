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

Except for the options, which are linked to the save file, you can save your
preferences for these into files, which are called `user pref files`.  For
the options, customize those using the ``=`` command while playing.


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
