=====================
Exploring the Dungeon
=====================

After you have created your character, you will begin your Angband
adventure. Symbols appearing on your screen will represent the dungeon's
walls, floor, objects, features, and creatures lurking about. In order to
direct your character through their adventure, you will enter single
character commands (see 'commands.txt').

Symbols On Your Map
===================

Symbols on your map can be broken down into three categories: Features of
the dungeon such as walls, floor, doors, and traps; Objects which can be
picked up such as treasure, weapons, magical devices, etc; and creatures
which may or may not move about the dungeon, but are mostly harmful to your
character's well being.

Some symbols are used to represent more than one type of entity, and some
symbols are used to represent entities in more than one category. The "@"
symbol (by default) is used to represent the character.

It will not be necessary to remember all of the symbols and their meanings.
The "slash" command (``/``) will identify any character appearing on your
map (see 'commands.txt').

Note that you can use a "user pref file" to change any of these symbols to
something you are more comfortable with.
   


Features that do not block line of sight
----------------------------------------

===== =========================    =====  ================================== 
``.``   A floor space              ``1``    Entrance to General Store
``.``   A trap (hidden)            ``2``    Entrance to Armoury
``^``   A trap (known)             ``3``    Entrance to Weapon Smith
``;``   A glyph of warding         ``4``    Entrance to Bookseller
``'``   An open door               ``5``    Entrance to Alchemy Shop
``'``   A broken door              ``6``    Entrance to Magic Shop
``<``   A staircase up             ``7``    Entrance to the Black Market
``>``   A staircase down           ``8``    Entrance to your Home
``#``   A pool of lava
===== =========================    =====  ================================== 

Features that block line of sight
---------------------------------

===== =========================    =====  ==================================
``#``   A secret door              ``#``    A wall
``+``   A closed door              ``%``    A mineral vein
``+``   A locked door              ``*``    A mineral vein with treasure
``:``   A pile of rubble           ``:``    A pile of passable rubble
===== =========================    =====  ==================================

Objects
-------
 
=====  =============================    =====  =============================
``!``    A potion (or flask)            ``/``    A pole-arm
``?``    A scroll (or book)             ``|``    An edged weapon
``,``    A mushroom (or food)           ``\``    A hafted weapon
``-``    A wand or rod                  ``}``    A sling, bow, or x-bow
``_``    A staff                        ``{``    A shot, arrow, or bolt
``=``    A ring                         ``(``    Soft armour
``"``    An amulet                      ``[``    Hard armour
``$``    Gold or gems                   ``]``    Misc. armour
``~``    Lights, Tools, Chests, etc     ``)``    A shield
``&``    Multiple items
=====  =============================    =====  =============================
 
Monsters
--------

=====   ===================   =====  ==================================== 
``$``     Creeping Coins      ``,``    Mushroom Patch
``a``     Giant Ant           ``A``    Ainu
``b``     Giant Bat           ``B``    Bird
``c``     Giant Centipede     ``C``    Canine (Dog)
``d``     Dragon              ``D``    Ancient Dragon
``e``     Floating Eye        ``E``    Elemental
``f``     Feline (Cat)        ``F``    Dragon Fly
``g``     Golem               ``G``    Ghost
``h``     Humanoid            ``H``    Hybrid
``i``     Icky-Thing          ``I``    Insect
``j``     Jelly               ``J``    Snake
``k``     Kobold              ``K``    Killer Beetle
``l``     Tree/Ent            ``L``    Lich
``m``     Mold                ``M``    Multi-Headed Hydra
``n``     Naga                ``N``    (unused)
``o``     Orc                 ``O``    Ogre
``p``     Human "person"      ``P``    Giant "person"
``q``     Quadruped           ``Q``    Quylthulg (Pulsing Flesh Mound)
``r``     Rodent              ``R``    Reptile/Amphibian
``s``     Skeleton            ``S``    Spider/Scorpion/Tick
``t``     Townsperson         ``T``    Troll
``u``     Minor Demon         ``U``    Major Demon
``v``     Vortex              ``V``    Vampire
``w``     Worm or Worm Mass   ``W``    Wight/Wraith
``x``     (unused)            ``X``    Xorn/Xaren
``y``     Yeek                ``Y``    Yeti
``z``     Zombie/Mummy        ``Z``    Zephyr Hound
=====   ===================   =====  ====================================

The Town Level
==============

The town level is where you will begin your adventure. The town consists of
eight buildings (each with an entrance), some townspeople, and a wall which
surrounds the town and may contain streams of lava. The first time you are
in town it will be daytime, but note that the sun rises and falls (rather
instantly) as time passes.

Townspeople
===========

The town contains many different kinds of people. There are the street
urchins, young children who will mob an adventurer for money, and seem to
come out of the woodwork when excited. Blubbering idiots are a constant
annoyance, but not harmful. Public drunks wander about the town singing,
and are of no threat to anyone. Sneaky rogues hang about watching for a
likely victim to mug. And finally, no town would be complete without a swarm
of half drunk warriors, who take offense or become annoyed just for the fun
of it. (There are assumed to be other people in the town, but they are not
represented on the screen as they do not interact with the player in any
way.)

Most of the townspeople should be avoided by the largest possible distance
when you wander from store to store. Fights will break out, though, so be
prepared. Since your character grew up in this world of intrigue, no
experience is awarded for killing the town inhabitants, though you may
acquire treasure.

Town Buildings
==============

Your character will begin their adventure with some basic supplies, and some
extra gold with which to purchase more supplies at the town stores.

You may enter any open store to buy items of the appropriate type.
The price the shopkeeper requests is dependent on the price of the item.
By default stores will not buy items from the player.  If you choose to play
with selling enabled, stores have a maximum value; they will not pay more
than that for any item, regardless of how much it is actually worth.

Once inside a store, you will see the name and race of the store owner, the
name of the store, the maximum amount of cash that the store owner will pay
for any one item, and the store inventory, listed along with the prices.

You will also see an (incomplete) list of available commands. Note that
many of the commands which work in the dungeon work in the stores as well,
but some do not, especially those which involve "using" objects.

Stores do not always have everything in stock. As the game progresses, they
may get new items so check from time to time. Stores restock after 10000
game turns have passed, but the inventory will never change while you are
in town, even if you save the game and return. You must be in the dungeon
for the store to restock. Also, if you sell them an item, it may get sold
to a customer while you are adventuring, so don't always expect to be able
to get back everything you have sold. If you have a lot of spare gold, you
can purchase every item in a store, which will induce the store owner to
bring out new stock, and perhaps even retire.

Store owners will not accept known harmful or useless items. If an object is
unidentified, they will (if selling is enabled) pay you some base price for
it.  Once they have bought it they will immediately identify the object.
If it is a good object, they will add it to their inventory. If it was a bad
bargain, they simply throw the item away. You can use this feature to learn
item flavors.

The General Store (``1``)
  The General Store sells foods, some clothing, torches, oil, shovels and
  picks. All of these items and some others can be sold back to the general
  store for money. The general store restocks like every store, but the
  inventory types never change.

The Armoury (``2``)
  The Armoury is where the town's armour is fashioned. All sorts of
  protective gear may be bought and sold here. The deeper into the dungeon
  you progress the more exotic the equipment you will find stocked in the
  armoury. However, some armour types will never appear here unless you
  sell them.

The Weaponsmith's Shop (``3``)
  The Weaponsmith's Shop is where the town's weapons are fashioned. Hand
  and missile weapons may be purchased and sold here, along with arrows,
  bolts, and shots. As with the armoury, not all weapon types will be
  stocked here, unless they are sold to the shop by the player first.

The Bookseller (``4``)
  The Bookseller holds supplies of the simpler books needed by magic users,
  and will buy the more advanced books which can be found in the dungeon.

The Alchemy shop (``5``)
  The Alchemy Shop deals in all types of potions and scrolls.

The Magic User's Shop (``6``)
  The Magic User's Shop deals in all sorts of rings, wands, amulets, and
  staves.

The Black Market (``7``)
  The Black Market will sell and buy anything at extortionate prices.
  However it occasionally has **very** good items in it. With the exception
  of artifacts, every item found in the dungeon may appear in the black
  market.

Your Home (``8``)
  This is your house where you can store objects that you cannot carry on
  your travels, or will need at a later date.

Within The Dungeon
==================

Once your character is adequately supplied with food, light, armor, and
weapons, they are ready to enter the dungeon. Move on top of the ``>`` symbol
and use the "Down" command (``>``).

Your character will enter a maze of interconnecting staircases and finally
arrive somewhere on the first level of the dungeon. Each level of the
dungeon is fifty feet high (thus dungeon level "Lev 1" is often called "50
ft"), and is divided into (large) rectangular regions (several times larger
than the screen) by permanent rock. Once you leave a level by a staircase,
you will never again find your way back to that region of that level, but
there are an infinite number of other regions at that same "depth" that you
can explore later. Monsters, of course, can use the stairs, and you may
eventually encounter them again, but they will not chase you up or down
stairs.

In the dungeon, there are many things to find, but your character must
survive many horrible and challenging encounters to find the treasure lying
about and take it safely back to the town to sell.

There are two sources for light once inside the dungeon. Permanent light
which has been magically placed within rooms, and a light source carried by
the player. If neither is present, the character will be unable to see.
This will affect searching, picking locks, disarming traps, reading
scrolls, casting spells, browsing books, etc. So be very careful not to run
out of light!

A character must wield a torch or lamp in order to supply his own light. A
torch or lamp burns fuel as it is used, and once it is out of fuel, it
stops supplying light. You will be warned as the light approaches this
point. You may use the "Fuel" command (``F``) to refuel your lantern (with
flasks of oil), and it is a good idea to carry extra torches or flasks of 
oil, as appropriate. There are rumours of objects of exceptional power 
which glow with their own never-ending light.

These last two paragraphs apply to most classes, but not to necromancers.
Necromancers dislike light, and shroud themselves in darkness.  They are
usually better off not carrying a light, but also do not gain any of the
bonuses that may come from magical light sources.

Objects Found In The Dungeon
============================

The mines are full of objects just waiting to be picked up and used. How
did they get there? Well, the main source for useful items are all the
foolish adventurers that proceeded into the dungeon before you. They get
killed, and the helpful creatures scatter the various treasure throughout
the dungeon. 

Several objects may occupy a given floor location, which may or may not
also contain one creature. However, doors, rubble, traps, and staircases 
cannot coexist with items.  As below, any item may actually be a "pile" 
of up to 40 identical items. With the right choice of "options", you
may be able to "stack" several items in the same grid.

You pick up objects by moving on top of them. You can carry up to 23
different items in your backpack while wearing and wielding up to 12
others. Although you are limited to 23 different items, each item may
actually be a "pile" of up to 40 similar items. If you |``t``ake| off an
item, it will go into your backpack if there is room: if there is no room
in your backpack, it will drop onto the floor, so be careful when swapping
one wielded weapon or worn piece of armor for another when your pack is
full.

.. |``t``ake| replace:: ``t``\ake

You are, however, limited in the total amount of weight that you can carry.
If you exceed this value, you become slower, making it easier for monsters
to chase you. Note that there is no upper bound on how much you can carry,
if you do not mind being slow. Your weight "limit" is determined by your
strength.

Many objects found within the dungeon have special commands for their use.
Wands must be Aimed, staves must be Used, scrolls must be Read, and potions
must be Quaffed. You may, in general, not only use items in your pack, but
also items on the ground, if you are standing on top of them. At the
beginning of the game all items are assigned a random 'flavor'. For example
potions of 'cure light wounds' could be 'red potions'. If you have never
used, sold, or bought one of these potions, you will only see the flavor.
You can learn what type of item it is by selling it to a store, or using it
(although learning by use does not always apply to magic devices). Lastly,
items in stores that you have not yet identified the flavor of will be labeled
'{unseen}'.

Chests are complex objects, containing traps, locks, and possibly treasure
or other objects inside them once they are opened. Many of the commands
that apply to traps or doors also apply to chests and, like traps and
doors, these commands do not work if you are carrying the chest.

One item in particular will be discussed here. The scroll of "Word of
Recall" can be found within the dungeon, or bought at the alchemist in town.
All classes start with one of these scrolls in their inventory. It acts in
two manners, depending upon your current location. If read within the
dungeon, it will teleport you back to town. If read in town, it will
teleport you back down to the deepest level of the dungeon which your
character has previously been on. This makes the scroll very useful for
getting back to the deeper levels of Angband. Once the scroll has been read
it takes a while for the spell to act, so don't expect it to save you in a
crisis. During this time the word 'recall' will appear on the bottom of the
screen below the dungeon. Reading a second scroll before the first takes
effect will cancel the action.

You may "inscribe" any object with a textual inscription of your choice.
These inscriptions are not limited in length, though you may not be able to
see the whole inscription on the item. The game applies special meaning to
inscriptions containing any text of the form '@#' or '@x#' or '!x' or
'!*', see 'customize.txt'.

The game provides some "fake" inscriptions to help you keep track of your 
possessions. Weapons, armor and jewellery which have properties you don't
know about yet will get a '{??}' label.  Wands, staves and rods can get a 
'{tried}' label after use, particularly if they have an effect on a monster
and were tested in the absence of monsters.

It is rumored that rings of power and extra rare spell books may be found
deeper in the dungeon...

And lastly, a final warning: not all objects are what they seem. The line
between tasty food and a poisonous mushroom is a fine one, and sometimes a
chest full of treasure will grow teeth in its lid and bite your hand off...

Cursed Objects
==============

Some objects, often objects of great power, have been cursed. There are many
curses in the game, and they can appear on any wearable object. Curses may
have a negative (or sometimes positive) effect on an object's properties, or
cause bad things to happen to the player at random.

You can choose to wear the object in spite of its curses, or attempt to
uncurse it using magic.  A warning: failed uncursing leads to the object
becoming fragile, and a fragile object may be destroyed on future curse removal
attempts.  It is up to you to balance the risks and rewards in your use
of cursed items.

Mining
======

Some treasure within the dungeon can be found only by mining it out of the
walls. Many rich strikes exist within each level, but must be found and
mined. Quartz veins are the richest, yielding the most metals and gems, but
magma veins will have some hoards hidden within.

Mining is rather difficult without a pick or shovel. Picks and shovels have
an additional magical ability expressed as '(+#)'. The higher the number,
the better the magical digging ability of the tool. A pick or shovel also
has plusses to hit and damage, and can be used as a weapon, because, in
fact, it is one.

When a vein of quartz or magma is located, the character may wield his pick
or shovel and begin digging out a section. When that section is removed, he
can locate another section of the vein and begin the process again. Since
granite rock is much harder to dig through, it is much faster to follow the
vein exactly and dig around the granite. Eventually, it becomes easier to
simply kill monsters and discover items in the dungeon to sell, than to 
walk around digging for treasure. But, early on, mineral veins can be a
wonderful source of easy treasure.

If the character has a scroll, staff, or spell of treasure location, they can
immediately locate all strikes of treasure within a vein shown on the
screen. This makes mining much easier and more profitable.

Note that a character with high strength and/or a heavy weapon does not
need a shovel/pick to dig, but even the strongest character will benefit
from a pick if trying to dig through a granite wall.

It is sometimes possible to get a character trapped within the dungeon by
using various magical spells and items. So it can be a good idea to always
carry some kind of digging tool, even when you are not planning on
tunneling for treasure.

There are rumors of certain incredibly profitable rooms buried deep in the
dungeon and completely surrounded by permanent rock and granite walls,
requiring a digging implement or magical means to enter. The same rumors
imply that these rooms are guarded by incredibly powerful monsters, so
beware!

Traps
=====

There are many traps located in the dungeon of varying danger. These traps
are hidden from sight and are triggered only when your character walks over
them. If you have found a trap you can attempt to |``D``isarm| it, but
failure may mean activating it.  Traps can be physical dangers such as pits,
or magical runes or inscriptions which will cause an effect when triggered.
Your character may be better at disarming one of these types of traps than
the other.

.. |``D``isarm| replace:: ``D``\isarm

All characters have a chance to notice traps when they first come into view
(dependent on searching skill). Some players will also get access to magical
means of detecting all traps within a certain radius. If you cast one of these
spells, there will be a 'Dtrap' green label on the bottom of the screen, below
the dungeon map.

Some monsters have the ability to create new traps on the level that may be
hidden, even if the player is in a detected zone. The detection only finds
the traps that exist at the time of detection, it does not inform you of
new ones that have since been created.

Staircases, Secret Doors, Passages, and Rooms
=============================================

Staircases are the manner in which you get deeper or climb out of the
dungeon. The symbols for the up and down staircases are the same as the
commands to use them. A ``<`` represents an up staircase and a ``>``
represents a down staircase. You must move your character over the
staircase before you can use it.

Most levels have at least one up staircase and at least two down staircases.
You may have trouble finding some well hidden secret doors, or you may have
to dig through obstructions to get to them, but you can always find the stairs
if you look hard enough.  Stairs, like permanent rock, and shop entrances,
cannot be destroyed by any means.

Many secret doors are used within the dungeon to confuse and demoralize
adventurers foolish enough to enter, although all secret doors can be
discovered by stepping adjacent to them. Secret doors will sometimes
hide rooms or corridors, or even entire sections of that level of the
dungeon. Sometimes they simply hide small empty closets or even dead ends.
Secret doors always look like granite walls, just like traps always look
like normal floors.

Creatures in the dungeon will generally know and use these secret doors,
and can often be counted on to leave them open behind them when they pass
through.

Level and object feelings
=========================

Unless you have disabled the option to get feelings you will get a message 
upon entering a dungeon giving you a general feel of how dangerous that 
level is.

The possible messages are :

===   ========================================= 
 1    "This seems a quiet, peaceful place"
 2    "This seems a tame, sheltered place"
 3    "This place seems reasonably safe"  
 4    "This place does not seem too risky"
 5    "You feel nervous about this place"
 6    "You feel anxious about this place"
 7    "This place seems terribly dangerous"
 8    "This place seems murderous"
 9    "Omens of death haunt this place"
===   ========================================= 

This feeling depends only on the monsters present in the dungeon when you
first enter it. It will not get reduced to safer feeling as you kill 
monsters neither will it increase if new ones are summoned.
This feeling also depends on your current dungeon depth. A dungeon you
feel nervous about at 2000' is way more dangerous than a murderous one
at 50'.

Once you have explored a certain amount of the dungeon you will also
get a feeling about how good are the objects lying on the floor of the
dungeon.

The possible messages are :

===   ========================================= 
 1    "there is naught but cobwebs here."
 2    "there are only scraps of junk here."
 3    "there aren't many treasures here." 
 4    "there may not be much interesting here."
 5    "there may be something worthwhile here."
 6    "there are good treasures here."
 7    "there are very good treasures here."
 8    "there are excellent treasures here."
 9    "there are superb treasures here." 
 $    "you sense an item of wondrous power!"
===   ========================================= 

The last message indicates an artifact is present and is only possible
if the preserve option is disabled.

You may review your level feeling any time by using the ^K command.
You may also consult it by checking the LF: indicator at the bottom
left of the screen. The first number after it is the level feeling
and the second one is the object feeling. The second one will be ?
if you need to explore more before getting a feeling about the value
of the treasures present in the dungeon.

Winning The Game
================

If your character has killed Sauron (a difficult task), who lives on level
99 (4950') in the dungeon, a magical staircase will appear that will allow
you to finally reach level 100. Morgoth lurks on this level of his dungeon,
and you will not be able to go below his level until you have killed him.
Try to avoid wandering around on level 100 unless you are ready for him,
since he has a habit of coming at you across the dungeon, the Mighty Hammer
'Grond' in hand, to slay you for your impudence.

If you should actually survive the attempt of killing Morgoth, you will
receive the status of WINNER. You may continue to explore, and may even save
the game and play more later, but since you have defeated the toughest
creature alive, there is really not much point. Unless you wish to listen
to the rumors of a powerful ring buried somewhere in the dungeon, or a suit
of dragon scale mail that resists everything...

When you are ready to retire, simply kill your character (using the ``Q`` key)
to have your character entered into the high score list as a winner. Note
that until you retire, you can still be killed, so you may want to retire
before wandering into yet another horde of greater demons.

Upon Death and Dying
====================
 
If your character falls below 0 hit points, they have died and cannot be
restored. A tombstone showing information about your character will be
displayed. You are also permitted to get a record of your character, and
all your equipment (identified) either on the screen or in a file.

Your character will leave behind a reduced save file, which contains only
your option choices. It may be restored, in which case a new character is
generated exactly as if the file was not there.

There are a variety of ways to "cheat" death (including using a special
"cheating option") when it would otherwise occur. This will fully heal your
character, returning him to the town, and marking him in various ways as a
character which has cheated death. Cheating death, like using any of the
"cheating options", will prevent your character from appearing on the high
score list.

