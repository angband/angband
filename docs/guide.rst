===============================
 A Players' Guide to Angband 
===============================

This guide assumes familiarity with the basic mechanics of the game. If you're
completely new to Angband, check out the user's manual and just start playing,
get into the dungeon and try, well, whatever seems to be prudent. You'll
probably die rather quickly, but the following will make much more sense to you
if you have just a little actual gameplay experience.

This guide was written for Angband 3.5.0, and is now a little out of date
(although the worst of the obsolete stuff has been cleaned out).  It is still
a handy source of hints and advice; just don't take it as an authority.

The basics
----------

As borrowed from a classic rgra post, there's not much that you actually have
to do. Your one and only mission is to slay Morgoth on dungeon level 100. In
order to get there, you need to go down a lot of stairs and kill Sauron on
dungeon level 99. That's about it - everything else is optional. Of course,
before you can kill Sauron you'll need lots of experience and good equipment,
but by the time you get that deep you'll have both. Just see to it that you
don't die along the way.

The one and most important thing you need to get in your head is that you can't
possibly kill every monster on every level. Think of the game as forays into
the dungeon from which you want to return with cool stuff; think of the
shallower levels as obstacles you need to overcome on your way deeper into the
dungeon; or whatever you like. Just never-ever think of it as a killing spree.
Until you find Sauron, your task is to survive and eventually get to level 99.

The next point is that you don't need to fight any particular monster (other
than the big two). Yes, there might be a rather impressive hoard in that
vault – but if the monsters guarding it are too many or too fierce, well, just
give it a pass. Angband offers an endless supply of monsters and treasure and
everything. There will always be another day – provided you live to see another
day. Sometimes you just have to bug out and run for your life.

So, let me recap, the vital points of Angband are:

* go down a lot of stairs
* kill Sauron (dl 99)
* kill Morgoth (dl 100)
* Quick start

From this point, the guide assumes that you are playing a fighting class
(warrior, ranger, rogue or paladin) and race (Dunedain, High-Elf, Dwarf,
Half-Orc, Half-Troll.) First, use the 'cost-based' stats selection, and create
a character with 3 or 4 blows (Don't pick a class/race combinations, like Dwarf
Ranger, that can only get 2 blows.)

First, set your stats to get maximum blows. If you have a combination with good
dexterity, select 18/10 dexterity, and then set your strength to 17 or 18. Set
your spell-casting stat to something low but usable (~12), and spend the rest
on constitution. For a beginning character, constitution is least important,
because it doesn't add significant HP until well above 18.

Example character::

  [Angband 3.5.0 Character Dump]

 Name   Anar         Age            102          Self  RB  CB  EB   Best
 Sex    Female       Height       5'11"   STR:     17  +1  +0  +0     18
 Race   High-Elf     Weight    14st 0lb   INT:     10  +3  +2  +0     15
 Class  Ranger       Turns used:          WIS:     10  -1  -2  +0      7
 Title  Runner       Game             1   DEX:     18  +3  +1  +0  18/40
 HP     14/14        Standard         0   CON:     10  +1  -1  +0     10
 SP     0/0          Resting          0

 Level                  1    Armor        [0,+2]    Saving Throw     49%
 Cur Exp                0                           Stealth         Good
 Max Exp                0    Melee        1d4,+2    Disarming        36%
 Adv Exp               23    To-hit        23,+4    Magic Devices     55
                             Blows      2.0/turn    Perception   1 in 20
 Gold                 139                           Searching        27%
 Burden          18.5 lbs    Shoot to-dam     +0    Infravision    40 ft
 Speed             Normal    To-hit        33,+4
 Max Depth           Town    Shots        1/turn


A Fighting Chance
`````````````````

For a melee character, the most important measure of power is how much damage
they can do in a single turn. Consider the above cl 1 character with 3 blows
from a Rapier (+7,+9), in a fight vs an out-of-depth Bullroarer, with no
escapes, no ranged weapons, and no armor. On the face, she has no chance.
However, if she can get in the first blow, she in fact has a 71% chance of
killing Bullroarer in a single turn, and a 95% chance of frightening him.

Further, with full buffing (!Hero, ?Blessing ?Berserk Strength), the character
has an 81% chance of killing him in a single turn. As well as improving the
to-hit probability from 91% to 95%, this corresponds to a 50% reduction in the
chance of failure in a dangerous situation. Finally, assuming 'Anar' does win
this battle, she gets 450 experience, and immediately goes jumps to cl 8.

Missile Damage
``````````````

With proper preparation even a weak character has an almost guaranteed chance
of killing Bullroarer, if you meet him at a distance (across a lighted room).
An unenchanted longbow does 7.5 damage/shot with ordinary (unenchanted) arrows.
However, flasks of oil do 7HP nominal damage when thrown, and do triple damage
(21HP) vs fire-vulnerable monsters. Bullroarer has 60 HP, so even a small stack
of oil will finish him off.

Summary
```````

* Character power is more closely associated with damage output rather than
  HP or character level.
* If you preserve your supplies, you can fight well above your weight. Early
  in the game, this means using flasks of oil against worthwhile targets;
  later in the game this means using branded or slaying "ego" ammunition.
  Good ammunition is too valuable to waste on less valuable targets (like red
  jellies, or groups of orcs.)
* Buffing, generally with !Heroism, can be very helpful to get starting
  characters out of sticky situations.
* Going deeper in the dungeon is often a more conservative (safer) strategy
  than staying at a shallow (cl < dl) depth. (cl: character level; dl: dungeon
  level)  HP and character level are easy to come by.
* Starting Equipment

Fighting Power
--------------

Buy a light weapon that gives you the maximum possible blows, with the highest
dice available. This is generally a Rapier, Main Gauche, or Dagger. If these
are not available, you may be better off quitting and restarting.

Plan to be deep
---------------

Always make sure to have a scroll of recall, even on your first trip into the
dungeon. Monsters and items at dl 5 (250') and deeper are much (~100x) more
valuable than items at dl 1. (The example above may be contrived, but it is
representative.)

To survive deep(er) you will want:

a. Escapes:

  * 3+ ?Phase Door
  * 1 ?Recall

b. Protection from secondary effects (confusion, blindness, poison)

  * 1 !CLW (for identification and blindness)
  * 1+ !CSW (for identification confusion)

c. Buffing

  * 1 !Hero (for protection from fear.)

d. Ranged attack to soften up a (single) unique

  * ~5 Flasks of Oil to kill dangerous and/or valuable monsters (throw oil
    for damage with the ``v`` throw command)
  * ~10-20 Iron shots (or arrows for Rangers) to throw at non-dangerous
    monsters with annoying side effects. (Stat drainers and acid damagers
    that are between you and the stairs down.) extra arrows for a ranger
    (shoot arrows with the ``f`` fire command.) Shots can be reused; oil can't.

e. Armor

Don't bother buying armor - it's very expensive in comparison to AC. You will
find it in the dungeon soon enough.

Your starting equipment will include more than enough food and illumination for
the first trip down.

The first trip
``````````````

It's quite possible to get to 500' (dl 10) or deeper in the first trip into the
dungeon. Plan to return when you run out of either escapes (?Phase door),
protection from side-effects (curing potions), or damage (arrows if you are a
ranger, flasks of oil if you are otherwise weak.)

Since you don't have much, don't spend it on less valuable monsters.

What to kill
````````````

* Any mobs of monsters that you can defeat (kill or frighten) in a single turn
  and either give good experience (a pack of wolves) or good drops (weaker
  orcs, novice humans.) If you are deep enough, this is likely to increase your
  character by several levels in a single battle.
* Uniques with good drops (Bullroarer, Brodda, Wormtongue)
* Easy kills that are likely to drop something worthwhile.

What to ignore
``````````````

* Monsters that will damage or destroy gear. (jellies, water hounds, etc)
* Non-valuable monsters that are likely to use up consumables (baby gold
  dragons, groups of spell-casters in line-of-sight, etc)
* Mobs that you can't dominate.
* Uniques with escorts you can't dominate
* Monsters at shallow depth. Drops for any given monster get better the deeper
  you go. Killing a novice mage at dl 1 generally gives nothing; at dl 20 he's
  likely to drop something worth hundreds of gold. Wormtongue has, on average, a
  noticeably better drop at dl 20 than dl 10.
* Things that waste effort. (Run-away breeders, low-EXP monsters with no drop.
  Just close the door and move on.)

What to avoid
`````````````

* Anything that can kill you in a single turn.

On Bad Luck
-----------

This is rule number one of Angband: don't take unnecessary risks. If you take
enough low-probability chances of death, you'll never survive to fight Sauron.
Such deaths are generally called 'stupid', but that's not always accurate.
Sometimes it's just bad luck. But given enough chances, you are guaranteed to
receive it. It's the trick to extremely fast dives: the fewer moves you make,
the less chance any one of them will be fatal, even if on average, your
individual moves are riskier than in slower play. But the strategy applies more
generally: unless you are exceedingly careful in play, messing around long
enough at any one depth guarantees that something bad will eventually happen.

Face-palm Tips
--------------

The preceding is good advice; however, it does not offer much more than
generalizations, albeit valid, for the "intermediate" (?) player. The
following is intended to state what many perceive to be blatantly obvious,
hence "you did WHAT!" face-palm deaths. This really should be cleaned up and
refactored, but placing here for now.

WARNING. I have yet to defeat Angband. This is a compilation of some of the
better tips I've learned while trying to explore the depths... (to Level ~35).
Additional advice would be greatly appreciated!

Start simple
````````````

Begin your Angband career as a warrior. Warriors are relatively simple to
begin with, and are less likely to be eaten by a pack of jackals.

Focus!
``````

Angband is a very harsh game, in that the character you've been playing for
months could be killed by a single careless action. Playing when tired or
drunk is probably a good way to leave yourself with nothing but a sad tale to
share on the forums. If you have the ability to sense monsters or traps then
use it. This is particularly important when you begin to encounter monsters
that, without the appropriate gear, will kill your character in a single move.

Use that stuff
``````````````

Angband has potions, spell books, wands, staffs, rods, activate-able items,
melee weapons, ranged weapons, and whatnot. They're meant to be used, for
crying out loud! It can take a while to get used to using all the different
types of items, but they work best when used in concert. For those able to use
magic devices, rods wands and staves can be very useful when your mana is
running low, and allow you access to spells that may not normally be available
to your class. They are also very useful for dealing with monsters that

Rangers have a bow
```````````````````

Really a subset of the previous point, but it happens so often... Don't try to
play a ranger like a warrior -- rely on the bow! Similarly for mages, don't do
a Gandalf. He may be able to draw a sword and rush headlong in to a pack or
orcs, but mages in Angband are considerably more fragile. Priests are better
equipped to engage in melee combat - with their healing abilities compensating
for their somewhat fragile nature.

Stockpile!
``````````

Players may not be inclined to carry multiples of an item, or do so in a
limited quantity, perhaps due to weight encumbrance concerns. Don't be afraid
to carry a LOT of an item, particularly the basics -- food, light,
projectiles, cure potions, "run away" scrolls, etc. Don't be afraid to MAX OUT
important items! Some monsters will steal or destroy your items, so it's worth
carrying additional quantities of key items - such as Scrolls of Recall or
important spell/prayer books. This becomes very important when you reach
dungeon levels in which monsters develop fire and acid-based attacks. Mages
and priests tend to start out with low strength, and so are very limited in
how much stuff they can carry, so consider carrying additional copies of the
spellbooks that you know you can't afford to lose during a fight.

An item you don't use is useless
````````````````````````````````

Common fallacy: you find an incredibly powerful Staff of Mighty BOOM! (3
charges), or a single Potion of become Chuck Norris, and then you keep
carrying it around and never actually use it. It could be a life insurance,
but you might still reconsider your strategy: maybe you've become too careful
lately (Angband rewards deliberate risk-taking, after all).

Identifying your items
``````````````````````

Many items found, especially early in the game, will unidentified or partially
identified. Weapons and armour can be identified by being worn and used in
combat, and can be removed if found to be of poor quality. Some characters
learn to cast spells to identify the runes on wearable items that define their
properties. 

Staves, rods and wands can often be identified by being used against monsters,
but be aware that some of these magical items can have negative effects. Only
use this approach when facing easily defeated monsters. Consuming unidentified
potions and mushrooms can be risky, so the risk-averse player may prefer to
sell them in the town. Ammunition can normally be identified by being thrown
or fired at a monster, and typically the worst thing that can happen is that
the attack does little damage to the monster. As with staves, wands and rods,
do this when facing a monster than can be easily defeated.

Scrolls can normally be identified by being read, but some scrolls have
negative effects. Your character may have a very short life if they read a
scroll that summons a horde of undead monsters. If using this approach, it's a
good idea to position your character on top of some stairs so you can quickly
escape if a mysterious scroll leaves your character surrounded by monsters.

With weapons and armour, your character will in time learn their inscribed
runes.

The dungeon is dark
```````````````````

Players will readily note that corridors are unlit; however, what may not be
blatantly obvious is that the dungeon gets darker with depth, until it's pitch
black. ALWAYS carry a light source. DON'T drop a light source for more loot!

You can RUN AWAY
````````````````

There is NO RULE that you have to clean out a dungeon -- AT ALL. See something
you can't handle? Don't be afraid to leave -- NOW. Sure you could try to avoid
it, but then again, it could be a hummerhorn. Half way through the level? You
can still LEAVE NOW. Nearly finished with the level? You can still leave NOW.
Think of it as a tactical strike, not a genocide mission.

Get away NOW!
`````````````

Sure, you should pay more attention and not get yourself in a next turn =
death situation, but this is not always avoidable and no one (presumably) is
so meticulously patient. ALWAYS carry get away items -- Phase Door, Teleport,
Recall, Teleport Level, etc. When you can, stockpile those that work NOW. If a
unique starts chasing you, you don't want to be waiting for Recall to kick in.

You should seriously consider a quick escape if dealing with a situation in
which monsters are breeding explosively. Some classes, such as mages, are more
easily able to handle these though the use of spells that affect multiple
monsters simultaneously, but even they should consider leaving if it becomes
apparent that the monsters are breeding more quickly than the player can kill
them.

Level 1 is still there
``````````````````````

Don't forget that you can always replay, and re-re-play the early levels for
ANY reason whatsoever. Recalled from a depth too deep? Dive from level 1 and
reset the depth. Want to fill your armor slots? Need a few more gold? Even if
you're on a streak diving through the dungeon, step back and reassess whether
you want to return to an earlier level -- you never know what could be lurking
around the corner.

Black Market Deals
``````````````````

Don't be afraid to buy from the "blackmarket". Sure the prices are more
expensive than the other stores, but it tends to offer a good selection of
items and it can be worth the gold versus not having it in the dungeon. You
can always get more gold, but the RNG is random. It's just another store --
don't worry about the name. The blackmarket is often a good source of potions
to increase your stats, so it's worth checking it each time you visit the town
- and try to have enough gold to purchase potions for the most important stats
for your character.

Try a Different Strategy
````````````````````````

Sure, what you're doing kinda works, but your characters keep dying off.
Playing conservatively? Try playing a character as though the "iron-man"
stairs setting is in effect. You'll probably want some ranged weapons for
this, but you may be surprised how fast you can level if you DIVE! Granted,
the lower levels are harder, but the deeper you go, the more experience per
kill you get. Still fight conservatively, but dive aggressively. Once your
character has gotten a decent complement of items and spells ... (what next?).
Don't be afraid to throw away characters -- DIVE aggressively until you get a
good feel for your character and you've leveled up.

On Depth and Item Value
```````````````````````

Whenever you kill a monster that may drop an item, a lot of randomness is
played out. No need to go into detail just here (if you really want to know,
you can look it up in the spoilers), suffice it to say that depth is the
single most important factor. You may, of course, find rather valuable and/or
useful stuff even on the very first level, but the chances are rather small.

In short: if you want to find better things, you need to go deeper.

The monster barely matters
``````````````````````````

If a monster drops an item, it can be anything. Really. You may find rare
spellbooks on illiterate Trolls, and jellies may drop artifact weapons. Some
rare monsters, like powerful dragons, will carry good or even exceptional
objects -- but still, the dungeon level is the most important factor. What was
"exceptional" on dl8 will probably be rather uninteresting on level 30.

Regarding Artifacts
```````````````````

For every single wearable item, there is a small chance that it will be an
artifact. This might seem slim, but considering the number of monsters you
will slay, it soon adds up. If you work through a large room full of orcs,
there's about a one-in-ten chance that you'll discover at least one artifact
among the carnage. Artifacts typically have impressive statistics and an
ability that can be activated.

Surviving to clvl 31 / dlvl 36
------------------------------

Additional tips learned from various failed attempts. Still have not beat
angband, but making considerable progress of late, particularly with my (now)
current character.

RUN AWAY!
`````````

Very first tip is a basic tenet repeated throughout the guide; however, it is
far too easy to become engrossed in your achievements (eg an awesome artifact,
maximizing stats, a slightly better item, gaining a second blow, getting a
speed bonus, etc) and lose it all due to something very avoidable had you been
paying diligent attention.

While playing, always make sure to check for the basics -- particularly
monsters, stairs, and traps. Don't loose a character, asserting that you can
kill a "C" easily only to discover that a "wild dog" is a far cry from a
"hellhound".

Similarly, if you just got some nice artifacts / other equipment or achieved a
significant accomplishment -- save your game and take a break. You don't have
to continue to play to enjoy your past success, and when you return you will
likely be more able to play diligently.

See a few uniques or other monsters that might possibly give you some trouble
-- maybe? Perhaps with only slight difficulty? See a vault with some cool
treasure, but a few troublesome monsters / uniques? If you're not highly
confident (perhaps even 100%) that you can easily survive the level, you can
leave on the spot. Just enter the level, immediately do your detection routine
(monsters, stairs / traps, etc) assess the immediate threat, and use the
stairs immediately if you want to. You can go up as readily as you came down,
and you can go down as readily as you came up. Further, artifacts can be
regenerated (unless you disabled that option) and as you dive, even better
items will be available. So, if you skip out on levels just because you want
to play it very conservatively, you will still find great items and better
yet, you have a higher chance of survival.

Information Awareness
`````````````````````

Angband offers a LOT of information on a LOT of different screens. If you
haven't already, try enabling multiple consoles -- ALL of them -- and then try
out different options / combinations. Being able to view a lot of information
simultaneously at a glance is a considerable benefit over having to be
diligent enough to manually check each relevant screen each time.

Also, change the text size. Sure you may be accustomed to your terminal font
and size for reading; however, you can read and make sense from context a lot
easier than you can reliably identify the glyph, color, and relative position
of every character on the screen. Angband may be a text-based game, but you
don't have to play with a tiny (or even normal size font). Using a slightly
larger font makes identifying things a LOT easier. The SDL interface is
perhaps the easiest for using multiple terminals and changing fonts.

This alone has greatly contributed to my survivability!

Start from the Beginning
````````````````````````

Trying to mature your character through the clvl / dlvl 20s can be challenging
until you can survive even lower depths. One tip here is to use Recall
exclusively to return from the dungeon and use the stairs to return back down
to the level you left.

Yes, this is more work, and as your clvl increases, the early levels become
exceptionally easy; however, it guarantees that you will always enter a level
on stairs, facilitating running away.

Item Collecting
```````````````

Its easy to stockpile, but you'll run out of space VERY quickly. Further, what
was useful may not be useful now and what's useful now may not be useful
later. It can be easy to try to grab everything in sight and then return to
town when your inventory is full; however, when youreach this point, try
continuing to dive, even with a full inventory.

As your character progresses and you continue to dive you will find better
classes of items. Don't be afraid to toss the less valuable stuff to make room,
so you can continue diving.

This is particularly important while diving. If you let a full inventory be a
limiting factor, you won't be able to get nearly as deep as if you are willing
to discard items along the way.

Take a Break
````````````

Again, this is another basic tip repeated thorough out the guide; however,
learn to stop playing. You can always save your game and resume later when you
would be in your prime and optimally ready to play as opposed to trying to
continue to play as drowsiness sets in.

This may be difficult at first, but it can help you avoid running decent
characters into the ground for avoidable mistakes...

Don't forget your Ranged Attacks!
`````````````````````````````````

Granted, as your character develops, you become more powerful at melee;
however, this does not preclude you from using ranged attacks! It is far to
easy to start playing a ranger like a warrior mage if you get a good melee
weapon, but don't forget your ranged skill! You may be "Superb" at Fighting,
but ranger's are even better at Shooting -- perhaps even "Legendary" or
better.

This can be a hard one to do diligently, especially if your melee weapon is
powerful, but try to play diligently and use ranged attacks. Try earlier
levels where survivability is much higher and try to play without melee
attacks for awhile to get used to using the bow again.

Even as your character matures, try to keep in mind your race / class core
strengths.
