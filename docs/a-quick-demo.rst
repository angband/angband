=====================
A quick demonstration
=====================

Angband is a very complex game, so you may want to try the following quick
demonstration. The following instructions are for demonstration purposes only,
and so they are intentionally boring.

For this demo, we will assume that you have never played Angband before,
that you have not requested any special "sub-windows", that you have not
requested any special "graphics" modes, that you have a "numeric keypad" on
your computer, and that you are using the default options, including, in
particular, the "original" command set. If any of these assumptions are
incorrect, you will need to keep in mind that this demo may not work; in
particular note that Windows users will be using graphical tiles by default,
so unless you turn off graphics map symbols below will be replaced by
little pictures.  There are many ways to view this file while playing.

Any time you see the '-more-' prompt, read the message and press space.
This takes precedence over any other instructions. At any other prompt, for
example, if you accidentally hit a key, you can normally "cancel" the
action in progress by pressing escape.

When the game starts up, depending on what platform you are using, you may
be taken directly to the character creation screen, or you may have to ask
to create a new character by using the File menu. In either case, you will
be shown the character information screen, and you will be given a series
of choices. For this demo, press ``a`` three times to elect a "human warrior"
character with the point-based stat allocation system. You will now be
presented with a description of your character. Look over the
description briefly, there is a lot of information here, and most of it
will not make any sense. Press enter four times and your character will be
placed into the "town".

You should now be looking at the basic dungeon interaction screen. To the
left is some information about your character. To the right is an overhead
view of the town. Nothing happens in Angband while the game is waiting for
you to specify a command, so take a good look at the town. You will see a
variety of symbols on the screen. Each symbol normally represents a terrain
feature, an object, or a monster. The ``@`` symbol is special, it
represents your character. You can use the ``/`` command to find out what a
given symbol represents. Press "/" then ``@`` now to verify the meaning of
the ``@`` symbol.

The solid blocks (which may be ``#`` symbols on some systems) around the
edge of the town represent the walls that surround the town. You cannot
leave the town above ground, although some games derived from Angband
(called "variants") have an overground element.

The rectangular blocks of walls with a number on the edge represent stores.
The number is the entrance to the store.  The ``.`` symbols represent the
"floor". It is currently daytime, so most of the town should consist of stores
and illuminated floor grids.  There will also be a few ``:`` symbols which
represent rubble.  Rubble comes in two varieties, "passable rubble" and just
"rubble", distinguished by color.  Both block vision, but you can move through
passable rubble.

Any "alphabetic" symbols always represent monsters, where the word
"monsters" specifies a wide variety of entities, including people, animals,
plants, etc. Only a few "races" of monsters normally appear in town, and
most of them are harmless (avoid any mercenaries or veterans if you see
them). The most common "monsters" in town are small animals (cats and dogs)
and townspeople (merchants, mercenaries, miscreants, etc).

Now use the ``l`` command to "look" around. This allows you to move the
cursor around and get a description of what is present at the cursor. The
cursor always starts on the square containing your character. In this case,
you will see a message telling you that your character is standing on a
staircase. Pressing space will cycle through the interesting locations
nearby. Press ``q`` or ESCAPE to exit the look command.

Now press ``i``, to display your character's "inventory". New characters
start out with some objects to help them survive (though there is an option
to start with more money instead). Your character will have some food, a
potion, some torches, and a scroll. Press ESCAPE to get out of the inventory
display. Press ``e`` to see what you are wearing. You will find you are
wearing armour on your body, wielding a dagger and lighting the way with a
torch. You have many other equipment slots but they are all currently empty.
Press ESCAPE to get out of the equipment display.

Press ``t`` to take something off. Note that the equipment listing is
reduced to those objects which can actually be taken off. Press ``g`` to
take off the armour, and then press ``e`` again. Note that the armour is no
longer shown in the equipment. Press ESCAPE. Press ``w`` to wield something
and observe that the inventory listing is reduced to those objects which
can actually be wielded or worn, press ``e`` to put the armour back on.

Monsters can only move after you use a command which takes "energy" from
your character. So far, you have used the ``w`` and ``t`` commands, which
take energy, and the ``e``, ``i``, ``l``, and ``/`` commands, which are
"free" commands, and so do not take any energy. In general, the only
commands which take energy are the ones which require your character to
perform some action in the world of the game, such as moving around,
attacking monsters, and interacting with objects.

If there were any monsters near your character while you were experimenting
with the ``w`` and ``t`` commands, you may have seen them "move" or even
"attack" your character. Although unlikely, it is even possible that your
character has already been killed. This is the only way to lose the game.
So if you have already lost, simply exit the game and restart this demo.

One of the most important things that your character can do is move around.
Use the numeric keys on the keypad to make your character move around. The
``4``, ``6``, ``8``, and ``2`` keys move your character west, east, north,
and south, and the ``7``, ``9``, ``1`` and ``3`` keys move your character
diagonally. When your character first moves, observe the ``>`` symbol that
is left behind. This is the "staircase" that she was standing on earlier in
the demo - it is the entrance to the dungeon.

Attempting to stay away from monsters, try and move your character towards
the entrance to the "general store", which is represented as a ``1`` on the
screen. As your character moves around, use the ``l`` command to look
around. You can press escape at any time to cancel the looking. If you die,
start over.

One of the hardest things for people to get used to, when playing games of
this nature for the first time, is that the character is not the same as
the player. The player presses keys, and looks at a computer screen, while
the character performs complex actions, and interacts with a virtual world.
The player decides what the character should do, and tells her to do it,
and the character then performs the actions. These actions may induce some
changes in the virtual world. Some of these changes may be apparent to the
character, and information about the changes is then made available to the
player by a variety of methods, including messages, character state
changes, or visual changes to the screen. Some changes may only be apparent
to the player.

There are also a whole set of things that the player can do that can not
even be described in the virtual world inhabited by the character, such as
resize windows, read online help files, modify colormaps, or change
options. Some of these things may even affect the character in abstract
ways, for example, the player can request that from now on all monsters
know exactly where the character is at all times. Likewise, there are some
things that the character does on a regular basis that the player may not
even consider, such as digesting food, or searching for traps while walking
down a hallway.

To make matters worse, as you get used to the difference between the player
and the character, it becomes so "obvious" that you start to ignore it. At
that point, you find yourself merging the player and the character in your
mind, and you find yourself saying things like "So yesterday, I was at my
friend's house, and I stayed up late playing Angband, and I was attacked by
some wild dogs, and I got killed by a demon, but I made it to the high
score list", in which the pronoun changes back and forth from the real
world to the virtual one several times in the same sentence. So, from this
point on you may have to separate the player and the character for 
yourself.

So anyway, keep walking towards the entrance to the general store until you
actually walk into it. At this point, the screen should change to the store
interaction screen. You will see the name of the shop-keeper, and the name
of the shop, and a list of objects which are available. If there are more
than twelve different objects, you can use the space or arrow keys to
scroll the list of objects. The general store is the only store with a fixed
inventory, although the amount of various items may vary. One of the items
sold here are flasks of oil. Press 'down' to highlight the line with
flasks of oil and press the ``p`` key to purchase some. If you are asked
how many you want, just hit enter. Any time you are asked a question and 
there is already something under the cursor, pressing return will accept 
that choice. Hit enter to accept the price. Many commands work inside the 
store, for example, use the ``i`` command to see your inventory, with the 
new flask of oil. Note that your inventory is always kept sorted in a 
semi-logical order, so the indexes of some of the objects may change as 
your inventory changes.

Purchase a few more flasks of oil, if possible: this time, when asked how
many you want, press ``3`` then return to buy three flasks at once. Flasks
of oil are very important for low level characters, because not only can
they be used to fuel a lantern (when you find one), but also they can be
ignited and thrown at monsters from a distance. So it is often a good idea
to have a few extra flasks of oil. Press escape to leave the store. If you
want, take time to visit the rest of the stores. One of the buildings,
marked with an ``8``, is your "home", and is not a real store. You can drop
things off at home and they will stay there until you return to pick them
up. The interface is exactly the same as a store, but there is no payment.

Now move to the staircase, represented by the ``>`` symbol, and press
``>``, to go down the stairs. At this point, you are in the dungeon. Use
the ``l`` command to look around. Note that you are standing on a staircase
leading back to town. Use the ``<`` command to take the stairs back to
town. You may find that any townspeople that were here before have
disappeared and new ones have appeared instead. Now use the ``>`` command
to go back down the stairs into the dungeon. You are now in a different
part of the dungeon than you were in before. The dungeon is so huge, once
you leave one part of the dungeon, you will never find it again.

Now look at the screen. Your character will most likely be at the end
of a corridor surrounded by eight walls and with a floor in the other
direction. If you are not at the end of a corridor, keep going back up to
the town and back down into the dungeon until you are.  You could move
down the corridor's floors, one square at time, but the game provides a way,
called "running", to move many squares until there's a choice to be made
about where to go or something interesting appears or happens. Try that now:
press ``.`` and then the direction key that would move you to the empty floor
(on most systems, you can press and hold Shift before pressing the direction
key as a shortcut for the two key sequence of ``.`` followed by a direction).

When your character stops, use ``l`` to look around to see what stopped your
character.  It could be a closed door, ``+``, which you can open by moving
into it.  It could be rubble, ``:``, blocking the path. You can clear that
by tunneling: ``T`` followed by a direction key to indicate which square to
dig out (on many systems, pressing and holding Control followed by the
direction key also works; that invokes the "alter" command which can be
used to dig, open doors, and other things).

It could be an object, represented by a punctuation symbol that does not look
like a wall, floor, closed door, or an open or broken door, ``'``.  In that
case, moving onto the object and then pressing ``g`` will pick up the item.
If the item on the floor matched something in your inventory or the object
is gold or gems, ``$``, your character will pick it up automatically.

A monster could have stopped the run.  Most monsters are represented by
letters, but some look enough like objects, that they are represented by
a punctuation symbol.  When in doubt, use ``l`` to look at something to know
what's there. If a monster stopped the run, you can attack it by moving next
to it and then moving into it. Keep moving into the monster until you kill the
monster, the monster runs away, or your character dies.  If your character
dies, start a new game. If the monster dies, it may have dropped one or more
objects. You can pick those up by moving onto the object and then press ``g``.

The run could have stopped when a corridor branched into two or more corridors
or because you have entered a room, empty space perhaps with some internal
walls. At this depth, rooms are almost always lit, and you will be able to
see parts beyond the one square radius illuminated by your torch.  Corridors
are dark, and, unless there is a lit room at the end, all you will see is
what is lit by your light source.

Now use the movement keys to explore the dungeon. As your character moves
around, you will notice that the screen keeps displaying some of the grids
that she has seen.  Think of this as a kind of "map" superimposed on the
world itself, the player can see the entire map, but the character can
only see those parts of the world which are actually nearby. If the
character gets near the edge of the "map" portion of the screen the
entire map will scroll to show a new portion of the world. If the game's
window can display 80 by 24 characters, only about ten percent of a typical
dungeon level can be seen by the player at one time.  Without moving, you can
use the ``L`` command to look at other portions of the map (it is also possible
with look command, ``l``; see the :ref:`targeting section of the manual
<targeting>` for more details on that).  Use the ``R`` key, then return, to
force your character to "rest" until she has recovered from any damage she
incurs while attacking monsters. Use the ``M`` key to see the entire dungeon
level at once, and hit escape when done. If your food rations are still at index
``a`` in your inventory, press ``E``, ``a`` to eat some food. If your oil
is still at index ``b`` in your inventory, and there is a monster nearby,
press ``v``, ``b``, ``'`` to throw a flask of oil at the nearest monster.
To drop an item from your inventory, press ``d`` plus the index of that
item. You can use the '^X' key to quit and save the game.

You now know enough to play a quick game of Angband. There is a lot more
for you to learn, including how to interpret information about your
character, how to create different kinds of characters, how to determine
which equipment to wield/wear, how to use various kinds of objects, and how
to use the more than fifty different commands available to your character.
The best resource for learning these things is the
:doc:`manual that includes this demo <index>`. The in-game help, which you
access by pressing ``?``, does have a list of the commands with the key and
very brief description for each.
