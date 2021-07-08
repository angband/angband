==================
Attacking monsters
==================

Attacking and Being Attacked
============================

Attacking is simple in Angband. If you move into a creature, you attack it.
You can attack from a distance by firing a missile or by magical means
(such as aiming a wand). Creatures attack in the same way. If they move
into you, they attack you. Some creatures can also cast spells from a
distance, and others can use various breath weapons (such as fire) on you
from a distance.

Creatures in walls can not be attacked by wands or other magic attacks
normally stopped by walls, nor can they be shot at with bows and arrows.
Tunnelling into the wall (using the "tunnel" or "alter" command) will allow
you to attack any creature in the wall with your main weapon. This applies
to creatures which "pass through" walls: if they "bore through" walls, the
wall is no longer there, and the creature can be targeted normally.

If you are wielding a weapon, the damage for the weapon is used when you
hit a creature. Otherwise you get a single punch which does minimal damage.

You may ``w``\ield one weapon for melee combat, and also one missile
launcher (bow, crossbow or sling). You may also wear one amulet (around the
one and only neck of the character), two rings (on the two "ring" fingers,
i.e. the third finger of each hand: a magic ring does not function when
worn on any other finger, nor may two be worn on the same finger), one
light source, and a full set of armor - body armor, shield, helmet, gloves,
boots and a cloak. Any or all of these items may provide powers to the
character in terms of bonuses to-hit, to-damage, to-armor class, or to
other stats.

Firing a missile (while wielding the appropriate launcher) is the only way
to get the "full" power out of the missile. You may of course throw an
arrow at a monster without shooting it, but you will find the effects will
not be what you had hoped.

Hits and misses are determined by ability to hit versus armor class. A hit
is a strike that does some damage; a miss may in fact reach a target, but
fails to do any damage. Higher armor classes make it harder to do damage,
and so lead to more misses. Characters with higher armor classes also
receive a damage reduction. This is not true for monsters, whose AC only
affects the character's difficulty to hit them.

If you wish to see how much damage your weapon will do, you can
``I``\nspect it. You will find the number of blows and how much damage you
would do per round, including information on whether your weapon damages
other types of monsters differently.

Monster Memories
================

There are hundreds of different creatures in the pits of Angband, many of
which have the same letter symbol and color on the screen. The exact
species of a creature can be discovered by |``l``ooking| at it. It is also
very difficult to keep track of the capabilities of various creatures.
Luckily, Angband automatically keeps track of your experiences with a
particular creature. This feature is called the monster memory. Your
monster memory recalls the particular attacks of each creature (whether or
not technically a monster) which you have suffered, as well as recalling if
you have observed them to multiply or move erratically, or drop treasure,
etc. Otherwise you would simply have to take notes, which is an unnecessary
bother.

.. |``l``ooking| replace:: ``l``\ooking

If you have killed enough of a particular creature, or suffered enough
attacks, recalling the monster memory may also provide you with information
not otherwise available, such as an armor class, hit dice, spell types,
frequency of spell casting, or the amount of damage for breaths or spells.
These attacks will be color coded to inform you of whether or not you
currently resist a specific attack. Red or orange means you do not resist
it, yellow means you partially resist it, and green means you resist it or
are immune. If you attack a monster with specific elemental attacks you will
learn if the monster resists that element or if they are immune. There are
other magical means to learn about monster's abilities that don't require
you to actually experience the attacks.

This memory can be used by all your characters; it is stored in a file
called 'lore.txt' in your user directory (~/.angband/Angband in Linux,
lib/user in Windows, Documents/Angband in macOS).

Your Weapon
===========

Carrying a weapon in your backpack does you no good. You must wield a
weapon before it can be used in a fight. A secondary weapon can be kept by
keeping it in the backpack, and switching it with the primary weapon when
needed. This is most often used when switching between two weapons, each of
which provides a rare power that the character needs at two separate times.
Note that a digging tool need only be carried in your pack, as when you try
to dig your best digging tool will automatically be used.

Weapons have two main magical characteristics, their enchanted ability to
hit and their enchanted ability to do damage, expressed as '(+#,+#)'. A
normal weapon would be '(+0,+0)'. Many weapons in Angband have bonuses to
hit and/or to damage.

Angband assumes that your youth in the rough environment near the dungeons
has taught you the relative merits of different weapons, and displays as
part of their description the damage dice which define their capabilities.
Any damage enchantment is added to the dice roll for that weapon. The dice
used for a given weapon is displayed as 'XdY'. The number ``X`` indicates
how many dice to roll, and number ``Y`` indicates how many sides they have.
A '2d6' weapon will thus give damage from 2 to 12, plus any damage bonus.
The weight of a weapon is also a consideration. Heavy weapons may hit
harder, but they are also harder to use. Depending on your strength,
dexterity, character class, and weapon weight, you may get attacks more
quickly: high dexterity and strength and low weapon weight are the main
factors. Warriors may get up to a maximum of 6 attacks per round: pure
spellcasters are limited to only 4: other classes may get up to 5. Your attacks
per round with a weapon are displayed as a decimal, e.g. 2.3 or 3.4 etc.
The fractions take the form of unused energy which is carried over to your
next turn.

Missile weapons, such as bows, have their characteristics added to those of
the missile used, if the proper weapon/missile combination is used, and
then the launcher multiplier is applied to the total damage, making missile
weapons very powerful given the proper missiles, especially if they are
enchanted. Like weapons, |``I``nspecting| ammunition will tell you how much
damage you will do with your current missile launcher.

.. |``I``nspecting| replace:: ``I``\nspecting

Finally, some rare weapons have special abilities. These are called ego
weapons, and are feared by great and meek. An ego weapon must be wielded to
receive the benefit of its abilities. It should be noted that some of these
items are considerably more powerful than others, and generally the most
powerful items are the rarest. Some items will have an obvious effect, 
like an increase in infravision, or extra strength. These effects will be 
noticed as soon as you wield the item. Other effects, like most 
resistances, will need to be learned. You can learn them by either 
suffering an appropriate attack, or by using magical means of 
identification.

Some of the more common ego weapons are described at the end of this file.

Your Armor Class
================

Your armor class (or AC) is a number that describes the amount and the
quality of armor being worn. Armor class will generally run from about 0 to
200, though exceptionally good armor can improve even on the latter figure.

The higher your armor class, the more protective it is. A negative armor
class would actually help get you hit. Armor protects you in three manners.
First, it makes you harder to be hit for damage. A hit for no damage counts
as a miss, and is described as a miss. Second, good armor will absorb
some of the damage that your character would have taken from normal
attacks. Third, acid damage is reduced by wearing body armor (but the
armor may be damaged instead). It is obvious that a high armor class is
vital for surviving the deeper levels of Angband.

Armor class values are always displayed between a set of square brackets,
as '[#]' or '[#,+#]'. The first value is the base armor class of the
armor. The second number is the magical bonus of the item, which is only
displayed if known, and will always have a sign preceding the value. These
plusses can be determined by wielding the armor in combat and being hit.
Note that a few rings, amulets, and weapons also have the '[+#]'
notation, indicating that they provide an armor bonus. Many pieces of heavy
body armor will also have a '(-#)' (in normal brackets) before the
'[#,+#]', which indicates that the weight of the armor decreases your
chances of hitting monsters. This can range from nonexistent for very light
armor to '(-8)' for the heaviest armor!

Monster status effects
======================

You will find some spells and items which can affect monsters in ways which
do not involve directly dealing them damage.  These are 'status effects'.
They are listed with their effects below.  These status effects will either
work on a monster type or they won't; some monsters resist particular effects
but not all do.

Hold Monster:
  Paralyses a monster until you hit them
  Increases chance of player getting a critical hit
  Normal duration 3-8 turns

Stun Monster:
  Reduces the monster's melee accuracy and damage by 25%
  1 in 10 chance that the monster will miss the turn
  Increases chance of player getting a critical hit 
  Normal duration 5-10 turns

Confuse Monster:
  Monster spells fail 50% more often
  Monster at least 40% more likely to miss target with spells/ranged attacks
  Monster ball & bolt spells sometimes go in the wrong direction
  30% chance of erratic movement, more when more confused
  Increases chance of player getting a critical hit
  Normal duration 5-10 turns

Slow Monster:
  -2 speed, more if more slowed
  Normal duration 10 or more turns

Sleep Monster:
  Puts monsters to sleep, but they can wake up again quite easily

Scare Monster:
  Monster will run away
  Monster spells fail 20% more often

Disenchant Monster:
  Monster spells fail 50% more often
  Normal duration 5-10 turns


Non-melee attacks and resistances
=================================

The player may at some time gain access to non-melee attacks, and many
monsters also have them. Perhaps the most famous of this type of attack is
dragon breath, but monsters may also cast spells at the player, and vice
versa. This damage generally is not affected by armor class, and does not
need a hit roll to hit the player or monster being aimed at.

Some attacks are purely magical: attack spells which blind, confuse, slow,
scare or paralyze the target. These attacks are resisted by monsters of
higher level (native to deeper dungeon depths) and characters with a high
saving throw - saving throws being dependent on class, level and wisdom.
There are also available resistances to fear, blindness, confusion and 
stunning, and the power of "free action" prevents magical paralysis and
most slowing attacks (the player may still be paralyzed by being "knocked
out" in melee or by a stunning attack, but this is very rare and can be
prevented with protection from stunning.) There are monsters that can
cause status effects such as blindness, paralysis or confusion through
their melee attack.  Since this is a physical effect and not a mental one,
the player will not get a saving throw.  However, having resistance to
that effect will prevent the negative status in all cases. It should
also be noticed that most unique monsters automatically pass their saving
throws, and some monsters are naturally resistant to confusion, fear and
sleep. Some monsters may have spells that 'cause wounds' that can be 
deadly if successful but do no damage if the saving throw is passed.
Some melee attacks by monsters may drain a stat, as can some traps: this is
prevented by having that stat sustained. Drained stats are temporary and
can be restored on gaining a new character level or consuming rare items
found in the dungeon.

Some monsters may cast spells that teleport the player character. There is
no saving throw, except to those that would actually teleport him up or
down one dungeon level. Having resistance to nexus will also prevent being
level-teleported, but will not help against normal teleportation spell
attacks. The player may teleport monsters in the same way, with a spell,
wand or rod. No monsters, even Morgoth himself, can resist this 
teleportation.  Yet...

Other attacks are usually element-based, including the aforementioned
example of dragon breath. Many monsters can breathe various attacks or cast
bolt or ball spells, and the player may also have access to bolt and ball
spells (or breathe like a dragon, in some rare circumstances). The player,
and the monsters, may be resistant to these forms of attack: resistance is
handled in different ways for the player and the monster, and for different
attack forms.

Bolt spells will hit the first monster (or the player) in the line of fire:
ball spells may centre on a target which may be hiding behind
other targets. Ball spells and breath weapons affect an area: other
monsters caught in the blast take reduced damage depending on their distance
from the centre of the blast. Breath weapons are proportional to a
fraction of the monster's current hit points and drop off in power with
distance from the monster, with a maximum cap on the
damage (which is higher for the most common of such attacks, owing to the
fact that the resistances are also easier to find). Bolt and ball spell
damage is calculated differently - often (but not always) relative to
character or monster level.

In the case of fire, cold, lightning, acid and poison, if the monster has
resistance to a player attack of this kind it will take almost no damage.
If the player has one or more permanent sources of resistance, they will take
1/3 of the damage they would normally take: if the player has a temporary
source of resistance (whether from potion, spell or item activation), this
will also reduce the damage to 1/3 of its normal level, allowing the
character to take only 1/9 damage if they have both permanent and temporary
resistance. Having more than one source of permanent resistance confers no
extra bonus, and using more than one source of temporary resistance
increases only the duration of the resistance: in both cases, either the
resistance is present or it is not. But one permanent resistance and one
temporary resistance are both effective simultaneously.

Elemental attacks also have a chance to damage wielded equipment or destroy
items in the character's inventory. Fire attacks destroy scrolls, staves,
magic books and arrows. Acid attacks destroy scrolls, staves, arrows, bolts
and can damage armor. Electricity attacks can destroy wands, rods, rings
and amulets. Cold attacks can destroy potions. Items in your inventory get
a saving throw, and they are unharmed if they pass it. Having resistance to
the element will make an item less likely to be destroyed. Items on the
floor that get caught in an elemental ball or breath are automatically
destroyed without a saving throw. Weapons, armor and chests can also be
destroyed if they are lying on the floor, but cannot be harmed if they are
in your pack.

The character may also gain immunity to fire, cold, lightning and acid if
he is fortunate to find any of the few artifacts that provide these
immunities: immunity means that no damage is taken, and the character's
equipment is also totally protected. Immunities are EXTREMELY rare.

Another attack that the player will come into contact with all too often is
the soul-chilling nature of the undead, which can drain the character's
life experience. Some monsters have a life-draining melee attack, others
may cast ball or bolt spells or, in extreme cases, breathe the very force
of the netherworld (shortened by the game to "nether".) There are two
powers which are of assistance in this case: that of "hold life" will
prevent 90% of all experience drains, and in the other 10% of cases, the
amount of experience lost will be reduced by 90%. That of "resistance to
nether forces" will provide resistance to nether bolts, balls and breaths,
reducing the damage and preventing any experience drains from these
attacks, but has no effect on melee "hits to drain experience". Monsters
caught in the blast from a nether ball or breath will take damage
proportional to distance from the centre of the attack, except for undead
who are totally immune. The player may find wands or rods of Drain Life,
which similarly are ineffective on those undead creatures which have no
life to drain: however, the real player equivalent attack spell is the
priest/paladin spell of "Orb of Draining", a ball spell which does damage
to all monsters, double damage to evil monsters, and is resisted by none.

Other attack forms are rarer, but may include: disenchantment (both in
melee or by a monster breath), chaos (breath or melee, which if unresisted
will cause the player to hallucinate and be confused, and may drain life
experience), nexus (which may teleport the player to the monster, away from
the monster, up or down a level, or swap over two of the player's
"internal" stats), light and darkness (which will blind a character unless
they have protection from blindness or resistance to light or dark), sound
(which will stun a character without sound resistance or protection from
stunning), crystal shards (which will cut a non-resistant character),
inertia (which will slow a character regardless of free action), gravity
(which will blink a character, also stunning and slowing), force (which
will stun the character), plasma (which will stun), time (which may
drain experience regardless of hold life, or drain stats regardless of
sustains), water bolts and balls (which may confuse and stun, and do
considerable damage from high-level monsters), ice bolts (which may
cut and stun, and damage potions), and mana bolts and balls (the latter
usually known as Mana Storms.) Magic missiles are included in the "mana"
category, whether cast by the monster or the player.

In addition items on the ground are especially vulnerable to elemental 
effects.  Potions on the ground will always be destroyed by cold, shards,
sound and force.  Scrolls, staves, books, and non-metal gear will always
get destroyed by fire or plasma.  Scrolls, staves, and all non-mithril gear
will be destroyed by acid.  Rings, amulets, wands and rods will be
destroyed by lightning and plasma.  And finally nearly everything will be
destroyed by a mana storm if left on the ground. 

Some attacks may stun or cut the player. These can either be spells or
breath attacks (sound, water balls) or from melee. A stunned character
receives a penalty to hit and is much more likely to fail a spell or
activation. If a character gets very stunned, they may be knocked out and
at the mercy of the enemies. A cut character will slowly lose life until
healed either by potions, spells or natural regeneration. Both stunning and
cut status are displayed at the bottom of the screen.

There are resistances available to chaos, disenchantment, confusion, nexus,
sound, shards, light and darkness: all of these will reduce the damage and
prevent side-effects other than physical damage. With these resistances, as
with nether resistance, damage is a random fraction between 1/2 and 2/3.

It should be noted that not all of these are actually vital to completing
the game: indeed, of the above list, only fire, cold, acid, lightning,
poison and confusion resists are regarded as truly vital, with blindness,
chaos and nether the next most desirable. Some attack forms are not
resistible, but thankfully these are rare: resist shards will prevent all
other magical attacks which cut (namely ice bolts), and confusion resistance 
will prevent confusion by a water bolt or ball, but there is no resistance 
to the physical damage caused by these following attacks: inertia, force, 
gravity, plasma, time, ice, water, mana. There is no resistance to any of 
the side-effects of a time attack, or indeed to anything but the stunning 
effects of a gravity attack.

A note on speed
===============

Monsters which do not move at normal speed generally move "slowly" (-10 to
speed), "fairly quickly" (+5), "quickly" (+10), "very quickly" (+20) or
"incredibly quickly" (+30). (It will surprise nobody that Morgoth is one of
the few monsters in the last category.) This is further adjusted by the fact
that any non-unique monster may have a random adjustment from (-2) to (+2)
to its own speed.

Generally, (+10) is exactly double normal speed, and (-10) exactly half.
(+20) is about three times normal speed, but after that there is less
noticeable improvement as speed goes higher - for instance, (+30) is not
quite four times normal speed, and higher values than this are largely
irrelevant. The player may find items which can be worn or wielded that
provide speed bonuses: these may include boots of speed, rings of speed and
a few very rare artifacts. Boots will provide a random 1d10 to speed: rings
of speed may be bigger than that - generally the best that the player will
get is two just over (+10), but individual rings of up to (+23) speed have
been known.

Separate from the question of permanent speed (as determined by the
player's speed items and the monster's natural speed) is that of temporary
speed. The player may cast a spell of haste-self, or use a potion, staff or
rod of speed or use an artifact activation to speed him temporarily: or a
monster may cast a haste-self spell, or be affected by another monster
"shrieking for help" or the player reading a scroll of aggravate monster.
In all cases, (+10) speed is added temporarily to the affected monster or
player. Using two or more sources of temporary speed is cumulative only in
duration - one cannot get from normal speed to (+20) using a potion and a
spell of speed. Spells of temporary slowing (including monsters breathing
inertia or gravity) are handled the same way, with exactly (-10) being
subtracted from the player or monster's speed temporarily, for the duration
of the spell or breath's effect.

Ego weapons and armor
=====================

Some of the ego weapons that you might find in the dungeon are listed
below. This will give you a small taste of the items that can be found.
However if you wish to discover these items on your own, you may not wish
to continue. Ego weapons are denoted by the following "names":

Ego Melee Weapons:
------------------
(Defender)
  A magical weapon that actually helps the wielder defend himself, thus
  increasing his/her armor class, and protecting him/her against damage
  from fire, cold, acid, lightning, and falls. This weapon also will
  increase your stealth, let you see invisible creatures, protect you from
  paralyzation and some slowing attacks, and help you regenerate hit points
  and mana faster. As a result of the regeneration ability, you will use up
  food somewhat faster than normal while wielding such a weapon. These
  powerful weapons also will sustain one stat, though this stat will vary
  from weapon to weapon.

(Holy Avenger)
  A Holy Avenger is often one of the most powerful weapons. A Holy Avenger
  will increase your wisdom and your armour class. This weapon will do
  extra damage when used against evil, demonic and undead creatures, and
  will also give you the ability to see invisible creatures. These weapons
  are basically extremely powerful versions of Blessed Blades and can be
  wielded by priests with no penalty. These weapons, like (Defender)
  weapons, also will sustain one random stat.

(Blessed)
  A blessed blade will increase your wisdom. If you are a priest, wielding
  a non-blessed sword or polearm causes a small penalty while attacking and
  may infuriate your god, decreasing the chances that she will accept your
  prayers: a blessed blade may be wielded without this penalty. Blessed
  blades also have one extra, random, power.

Weapon of Westernesse
  A Weapon of Westernesse is one of the more powerful weapons. It does
  extra damage against orcs, trolls, and giants, while increasing your
  strength, dexterity, and constitution. It also lets you see invisible
  creatures and protects from paralyzation and some slowing attacks. These
  blades were made by the Dunedain.

Weapon of Extra Attacks
  A weapon of extra attacks will allow the wielder to deliver extra attacks
  during each round.

Elemental Branded Weapons
  Each of the five elemental attacks has a corresponding weapon which will
  do treble its base damage to creatures not resistant to that element. (It
  should be noted that the magical damage bonus is not affected by this: a
  weapon of Flame '(2d6) (+5,+6)' does 6d6+6 damage per hit, not 6d6+18,
  against creatures which are not fire-resistant.) There are weapons of
  Flame, Frost, Lightning, Acid and Poison brands.

Weapons of Slaying enemies
  These weapons do extra damage against creatures of a vulnerable type.
  Weapons of Slay Evil and Slay Animal do double the base damage, while
  weapons of Slay Orc, Troll, Giant, Dragon, Demon and Undead do triple the
  base damage. As with elemental branded weapons, the magical damage bonus
  is not affected.

Weapons of |*Slay*ing| enemies
  These weapons, in addition to doing extra damage to your enemies, have
  extra powers as well. In each case, one stat is increased. Weapons of
  |*Slay*| Dragon, Demon or Undead are also more powerful against their
  opponents, doing five times their base damage rather than the normal
  three.

Shovels and Picks of Digging
  These powerful diggers will dig through granite as if it were mere wood,
  and mineral veins as if they were butter. Permanent rock is still an
  impassable obstacle.

Ego Missile Launchers and Ammo:
-------------------------------
Launchers of Accuracy
  These launchers have an unnaturally high to-hit number, making them
  extremely accurate.

Launchers of Power
  These launchers do an unnaturally high amount of damage due to their high
  to-dam number.

Launchers of Extra Shots
  These launchers allow the wielder to shoot more times per round than
  normal.

Launchers of Extra Might
  These launchers have a higher base damage than normally made launchers of
  their type. For instance, a 'Long Bow of Extra Might (x3)(+X,+Y)(+1)'
  is really a Long Bow '(x4)(+X,+Y)' where '(+X,+Y)' is the standard
  to-hit and to-dam. As the damage multiplier with the bow affects
  **everything** the base arrow damage, the magical damage bonus on both
  the bow and the arrow, and any bonuses for slaying or elemental-branded
  arrows - this makes it a powerful weapon.

Ammo of Wounding
  This ammunition - whether it be pebbles, iron shots, arrows, bolts,
  seeker arrows or seeker bolts - has big bonuses to-hit and to-damage.

Ammo of Elemental Brands, and Ammo of Slaying enemies
  This works in the same way as melee weapons of the same type: double
  damage for slay evil and slay animal, triple damage for all other slays
  and for all elemental brands. Unlike melee weapons, the slays and
  elemental brands **do** affect the magical damage bonus for ammo.

These are the most common types of ego-weapon: note that they are not the 
ONLY ego-items available in the dungeon, there may be more.

Apart from these there are some very rare and well made weapons in the
dungeon with not necessarily any special abilities. These include Blades
of Chaos, Maces of Disruption, and Scythes of Slicing.  They can also be
ego weapons like the ones above.  For example, a Blade of Chaos (Holy
Avenger) is much more powerful than many artifact weapons!

Some pieces of armor will possess special abilities denoted by the following
names:

Ego Armors and Shields:
-----------------------
of Resist Acid, Lightning, Fire or Cold
  A character wearing armor or a shield with one such resistance will take
  only 1/3 of normal damage from attacks involving the relevant element of
  acid, lightning, fire or cold. Note that multiple permanent sources of
  resistance are NOT cumulative: wearing two is no better than wearing one.
  However, armor which provides resistance to acid cannot itself be damaged
  by acid, and this is a good reason to wear more than one such piece of
  armor.
 
of Resistance
  A character wearing armor with this ability will have resistance to Acid,
  Cold, Fire, and Lightning as explained in each part above.

Armor of Elvenkind
  This is the same as Resistance armor, only generally better enchanted. It
  will make you more stealthy. This armor also possesses an extra
  resistance, at random from the following list: poison, light, dark,
  nexus, nether, chaos, disenchantment, sound, and shards.

Robes of Permanence
  These robes are designed especially for wizards. Just like Elvenkind
  armor, they provide resistance to fire, cold, acid, and electricity and
  cannot be damaged by acid. They sustain all of your stats and protect you
  from a good deal of all experience draining. Also like Elvenkind armor,
  they have one random resistance.

Dragon Scale Mails
  These extremely rare pieces of armour come in many different colors, each
  protecting you against the relevant dragons. Naturally they are all
  resistant to acid damage. They also occasionally allow you to breathe as
  a dragon would.  Dragon Scale Mails can also have egos as well.

Ego Helms:
----------
Stat Boosting Helms
  There are magical helms found in the dungeon that have the ability to
  boost the wearer's intelligence or wisdom. In addition to boosting the
  relevant stat these helms will also prevent that stat from being drained.

Crown of the Magi
  This is the great crown of the wizards. The wearer will have an increased
  (and sustained) intelligence, and will also be given resistance against
  fire, frost, acid, and lightning. These valuable helms also have an
  additional random power.

Crown of Might
  This is the crown of the warriors. The wearer will have an increased and
  sustained strength, dexterity, and constitution, and will also be immune
  to any foe's attempt to slow or paralyze him or her.

Crown of Lordliness
  This is the great crown of the priests. The wearer will have an increased
  and sustained wisdom.

Helm/Crown of Seeing
  This is the great helmet or crown of the rogues. The wearer will be able
  to see invisible creatures, and will have an increased ability to locate
  traps. It is also rumored that the wearer of such a helm will not be able
  to be blinded.

Helm of Infravision
  This helmet allows the character to see monsters even in total darkness,
  with the ability to see heat. Note that spellbooks are the same
  temperature as the surroundings, and so cannot be read unless some real
  light is present. (Some monsters which are invisible to normal vision can
  be seen under infravision.)

Helm of Light
  In addition to providing a permanent light source for the wearer, this
  helm also provides resistance against light-based attacks.

Helm/Crown of Telepathy
  This helm or crown grants the wearer the power of telepathy.

Helm of Regeneration
  This helm will help you regenerate hit points and mana more quickly than
  normal, allowing you to fight longer before needing to rest. You will use
  food faster than normal while wearing this helm because of the
  regenerative effects.

 
Ego Cloaks:
-----------
Cloak of Protection
  This finely made cloak will come with an unnaturally high enchantment and
  is not affected by elemental based attacks.

Cloak of Stealth
  This cloak will increase the wearer's stealth, making the wearer less
  likely to wake up sleeping monsters.

Cloak of Aman
  These exceptionally rare cloaks provide great stealth, have a very high
  enchantment, and one random resistance.

Ego Gloves:
-----------
Gloves of Free Action
  The wearer of these gloves will find himself resistant to paralyzing
  attacks as well as some slowing attacks. Because of the special nature of
  these gloves, magic users may wear these gloves without incurring a mana
  penalty.

Gloves of Slaying
  These gloves will increase the wearer's fighting ability by boosting the
  wearer's to-hit and to-dam values.

Gloves of Agility
  These gloves will increase the wearer's dexterity. Because of the special
  nature of these gloves, magic users may wear these gloves without
  incurring a mana penalty.

Gauntlets of Power
  These spiked gauntlets will boost the wearer's strength as well as the
  wearer's to-hit and to-dam numbers.

Ego Boots:
----------
Boots of Slow Descent
  These boots protect the wearer from the effects of small falls.

Boots of Stealth
  These boots increase the wearer's stealth, like a Cloak of Stealth.

Boots of Free Action
  The wearer of these boots will find himself resistant to paralyzing
  attacks as well as some slowing attacks.

Boots of Speed
  The wearer of these boots will become unnaturally fast.

Once again, these are not necessarily the ONLY ego-items in the dungeon, 
only the most common.

Apart from these there are some very rare and well-made armours in the
dungeon with not necessarily any special abilities. These include Shields
of Deflection, Adamantite Plate Mail, Mithril Plate Mail, Mithril Chain
Mail, and Elven Cloaks. The first four cannot be damaged by acid because of
the quality metals they contain.

There are rumors of unique "artifact" items in the dungeon - weapons and
armor of all types. Many of these are more powerful than even the greatest
ego-items: some are weak and have little more than a name to recommend
them.

.. |*Slay*| unicode:: *Slay*
.. |*Slay*ing| unicode:: *Slay*ing
