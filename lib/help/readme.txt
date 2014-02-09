======================
README for Angband 3.4
======================

New Features in Angband 3.4.0
=============================

The new version of Angband has a lot of very interesting changes that we
hope will bring great enjoyment to new players and veterans who haven't
crawled through Morgoth's lair in many years.  

The most obvious change that you'll see when booting is that there's been
a major graphical overhaul that was spearheaded by a beautiful new set of
64x64 tiles.   This tile set can also be resized, so that you can fit a 
larger view on your monitor.  Furthermore, a new option to allow for
shorter range on spells and attacks is available for those who choose to 
play with large tiles, or on small screens.  Of course, the old ASCII view
is still available.

There are many new pits that you will find in the dungeon.  Weaker pits
are now much less likely to exist deeper in the dungeon, so no more jelly
pits on dungeon level 90.

Item detection has gotten a major overhaul.  All item detection, excepting
potions of enlightenment and the rogue spell "detect object" no longer 
indicate exactly what object is there.  Instead, detected objects are 
replaced by a red asterisk, that changes to the item type when your player
can see it.  Squelched items will still be detected as asterisks, but will
disappear when you get them in your line of sight.

There has been a huge flavor upgrade with the change to UTF-8.  Now many
names of monsters and items have the appropriate diacritical marks.  

All monsters that were in the Angel class have been replaced with the much
more thematic "Ainu".  This is more of a cosmetic change than a gameplay
change, but the three Ainu uniques are now slightly different.

The amount of items in any stack is now limited to 40.  This has the
largest effect on the quiver, but also affects inventory and shops.

All weak curses are gone.  The only curses are now heavy curses on 
artifacts, and permanent curses on the very special artifacts.  Remove
Curse scrolls and spells of all forms are gone as well.  Heavy curses can
be broken with enchantment spells or scrolls.

Torches have become weaker, giving only one light radius.  The dungeon is
a lot darker in the first few levels.

Monster summoning has been altered significantly.  Summoning spells now
have a strength that depends on the caster and the dungeon level.  Also,
summoned monsters no longer arrive with friends or escorts.

Deep descent scrolls now act on a delay and drop you 5 levels deeper.

There have been some tweaks to item allocation, item prices, monster
alertness, and combat rolls as well as the usual assortment of bugfixes.
A full list of the changes can be found in the changes.txt file.

Features in Angband 3.3.0
=============================

Here are a list of the largest changes in 3.3.0

Dungeon generation has had its first major overhaul for two decades. As
well as rooms and vaults, pits and nests you will now find special "cavern"
and "labyrinth" levels occasionally. Both of these new level types, for
different reasons, can be quite dangerous.

Level feelings have finally been completely redesigned. You will now get
two separate feelings: the moment you descend the stairs, your first
impression of the new level will give you a sense of how dangerous it is
(judging by corpses, bloodstains, etc.). Some time later, you will get a
sense of how lucrative it might be in terms of treasure/loot. Many thanks
to Jeff Greene for the foolproof mechanism for the random time delay.

Objects may now have up to three pvals, which are displayed in <angle
brackets>. So an object may provide +1 to STR and +2 to stealth at the same
time.

Macros no longer exist. Everything you could do with macros you can now do
with keymaps - and more.

You can no longer destroy items - they are squelched instead. You can
toggle the visibility of squelched items by using the 'K' command.

Certain things, like enchantment scrolls and restore stat potions, are no
longer available in town. Restore stat potions don't exist any more - but
drained stats are restored on level up (and by stat gain potions).

Resistances have been made more consistent, and a new class of
"protections" from non-damaging status effects has been created. Confusion
is no longer a breath attack, and has a protection rather than a
resistance. Protection from stunning is now separate from sound resistance.
A resistance will protect you from status effects from that element only -
so sound resistance will no longer protect you from mystics.

If you play with randarts and really like the ones in a particular game,
you can keep them for subsequent games by choosing "keep randarts" from the
death menu. Note that switching back to non-random artifacts will lose them
though.

Some items have been rebalanced: Dragon Scale armours are no longer quite
so frequently powerful, and magical effects which hinder monsters (slow,
sleep, confuse, scare) are now more useful. Elemental rings no longer boost
your melee damage. Teleport Other is now a bolt, affecting only one monster
at a time, and *Destruction* now removes artifacts as well as monsters.
Potions that cure wounds are no longer quite so powerful.

Spiking doors is now a lot more useful, and you can lock doors too (by
using the Disarm command). Monsters may take several turns to open/bash a
locked and spiked door.

Finally, mimics are a lot harder to spot ...

