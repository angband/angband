******************
A Guide to Affixes
******************

Introduction
============

This work was based on some ideas vaguely expressed in http://angband-dev.blogspot.com/2011/09/so-what-exactly-is-ego-item-anyway.html
(editor's note:  that was done in the context of the v4 branch of Angband's
development; what follows has been left as is; ego items in Angband 4
remain broadly similar to the Angband 3.3 implementation mentioned here but
there's been some renaming and restructuring of the lines in ego_item.txt).

It started off with wanting to address some pet peeves with ego item generation
(the hackish OBJECT_XTRA_TYPE\_ defines, the rigid nature of the better ego
types, etc.). It was also inspired by Eytan Zweig's item prefixes (#587), and
grew from there. The basic idea is to allow for a greater variety of magical
items and smoother progression from a dagger (1d4) (+0,+0) to a 6d5 Holy
Avenger (and so on for other equipment).

What hasn't changed
===================

All the old objects and ego items are still generatable almost exactly as they
used to look. Slay weapons, branded weapons, Defender/HA/Gondolin/Westernesse
weapons, Elvenkind armours and boots, Robes of Permanence, etc. At the moment
(subject to any rebalancing we may wish to do) they are still generated with
the same flags and on the same object types as they used to. (So only Maces
can get the Of Disruption suffix, only Scythes can get Of Slicing, only robes
can get Of Permanence etc.)

What has changed
================

I say "almost" because there are two noticeable differences. Some of the
high-end items will have all four IGNORE\_ flags where they only used to have
one or two (e.g. Gondolin). More significantly, most items will have lower
to-hit/to-dam values than under the old system - though this can now be
balanced purely through editing ego_item.txt and ego_themes.txt

The ego_item type
-----------------

Ah yes, themes. Let's get down to the code. The ego_item type still exists, and
they're still stored in the global e_info[] array, but they're not called egos
any more, they're called affixes (because you can have MAX_AFFIXES of them).
There are some (IMO) useful changes to the type:

* The C: line now takes seven parameters rather than three. As well as
  to_h/to_d/to_a, you can now modify base AC, weight, dice and sides. The
  first two of these are percentage mods (and are signed so can be negative),
  the last two are just extra dice and sides (but can also be negative). Full
  credit to Eytan Zweig for inspiring these.
* The M: line is assumed to be NO_MINIMUM:NO_MINIMUM:NO_MINIMUM if it's absent.
  So no worries about accidentally removing armour penalties.
* The X: line is gone. Rarity is no longer used, and random flags are now
  done by R: or R2: lines. R: lines specify OFT\_ flag types (sustains, high
  resists, etc. - now in src/object/list-flag-types.h), while R2: lines
  allow specification of an exact flag mask. So if you want, you can have an
  affix that adds one of SUST_STR or SUST_CON but nothing else. You cannot
  add pval flags this way though - I wrote a spec for a Z: line to do this,
  but it is difficult to box around MAX_PVALS so I have left this for another
  day.
* The T: line now deals with rarity as well as kind legality. For each
  tval/min-sval/max-sval group, you can specify an alloc_prob and a min and
  max depth. EGO_TVALS_MAX has been increased accordingly (and could go
  further). So you can have the same affix appear with different likelihood
  and at different depths for different items. Acid-resistant shields can be
  much rarer (or less rare) than acid-resistant cloaks, for example.
* The T: line also includes a "level" field, which specifies how good the
  affix is considered for this object group at these depths. So far I've
  defined four levels: "good", "great", "uber" and "artifact", but we could
  have more. The last is in case we want to have affixes only found on
  randarts (e.g. immunities). More on levels in a minute, but one point to
  note is that you can redefine the same object group here. So:
  | T:soft armour:0:99:great:10:1 to 25
  | T:soft armour:0:99:good:25:26 to 100
  would mean that this affix is considered "great" at low levels but merely
  "good" at deeper depths - for soft armour.

Ok, so those are affixes. I stripped out all the compound egos, with multiple
attributes, and boiled affixes right down, so that each only provides one or
two things. Resist affixes still give the IGNORE\_ flag, and the five x3 brands
still give both RESIST\_ and IGNORE\_ flags, but otherwise most affixes only
provide one flag. I've added the EyAngband prefixes, which modify the base
properties of armour and weapons (AC, hit/dam, dice/sides) - I left out only
the inappropriate race-specific ones (Ey has lots of weird races), and
guesstimated on some. (N.B. I was concentrating on implementation, not on
balance, so there is a ton of room for tweaking the values of any of the
Ey affixes, and any others.) I also stripped out base objects which are
obviously other base objects with affixes: Lead-Filled Mace, Mithril
Chain/Plate, Adamantite Plate, Mace of Disruption, Scythe of Slicing.
These are all now affixes (but see below for more to do on this).

N.B. There is no base item called "Blade", so the "of Chaos" affix can be
applied to the top three swords (Katana, Zweihander and Executioner IIRC).
Though there's no reason not to allow Daggers of Chaos, if we want to.

*make_object* and *make_artifact*
---------------------------------

Having improved the type and reduced the egos down to simple affixes, I set
about changing object generation. First I rewrote kind_is_good to look for
OF_GOOD, so that powerful items like DSMs are automatically considered
"good" and never "average". This doesn't actually have much impact on the
obj_alloc_great table though, since all armour and weapon types were
already in it (but see below for more thoughts on this).

Then I sorted out artifact generation, combining special and normal
artifacts. This distinction only ever existed because all normal
artifacts were only ever generated after the object kind was chosen,
and specials were attempted before. Now *make_object* tries for an
artifact first, and *make_artifact* now creates an allocation table of
all possible artifacts (i.e. not yet created, in-depth, out-of-depth
checks etc.) before choosing one. If the kind is already chosen (e.g.
from specified monster drops) then the allocation table looks a lot
smaller because it only contains artifacts legal for that kind.

Please note: artifact generation will need quite a bit of balancing,
because not only did I unify the generation functions (which will
fundamentally change how many are generated), but I also recalibrated
the alloc_prob scale to 1000 rather than 100, and reflected the fact
that these are now checked independently of base items. So Bladeturner's
old rarity accounted for the fact that its base item was very rare: now
it has to be even rarer. I implemented multiple A: lines for artifacts,
so we now have finer control over their findability (because they can
have different rarities at different depths).

Choosing and applying affixes
-----------------------------
So, if we're not an artifact we now set the number and quality of an
object's affixes in *apply_magic*. Note that the call to *apply_magic*
from *make_object* deliberately sets allow_artifacts to FALSE, because
we've already tried for an artifact there. But if it's called directly
(for a specified item), then we still get a shot at becoming an artifact.

One important change is that **all** items can now get "good" affixes.
This is because *apply_magic_weapon/armour()* have been removed, and
plusses now come from first-grade affixes (Quality, Sharp, Brutal etc.),
but you can now get minor abilities like Feather Falling on what would
previously have been just "good" items. Forced-great items automatically
get access to great affixes, and good items do so 25% of the time. There's
also a depth-dependent chance for increment, so uber affixes are (i) only
available on good or great drops and (ii) much less likely early on.

The number of affixes is a function of depth (0 to (1 + lev / 25)), plus
one or two if good, or three or four if great (minus one if we have
OF_GOOD). These numbers can be rebalanced, of course. There might not
actually be many "great" drops, so we might want to be more generous to
normal or good items, and tone down the great items to only a couple of
extra affixes. But note that "normal" drops get much more interesting
with depth, so this may not be necessary.

I haven't done a lot of renaming yet, but I've renamed *make_ego_item*
to *obj_add_affix* because it wasn't called from anywhere else except
*apply_magic*. It checks that we're not trying to add an affix to an
artifact, a themed item, or an item with MAX_AFFIXES already. It also
does that weird GREAT_EGO level boost, for a one-in-20 chance of a
potentially huge level boost (though that doesn't boost the affix level
yet - if it did, this would create interesting possibilities for randarts,
noted below). Importantly, we copy the object, so we don't have to worry
about affixes creating broken items - if that happens we just roll back
and don't add anything.

We choose which affix to apply in *obj_find_affix*, which is
*ego_find_random* renamed and rewritten to allow for the new T: lines
above. Like *make_artifact*, it builds an allocation table from the
affixes which are legal for this item at this depth and affix level.

We actually apply the affix in *ego_apply_magic* (which I didn't rename
yet 'cos it's called from a few places) - it deals with the extra stuff
outlined above (base ac / weight / dice / sides, and random flags) but
is otherwise recognisable. We now check minima both before and after
application, to ensure that a min_pval of 2 gives correct results when
applied to an existing pval of, say, -1. We also check flags at the end,
to remove contradictory elemental flags (RES_FOO and VULN_FOO etc.),
and to strip lots of mods off ammo (so that we don't have to replicate
affixes and themes for ammo). Oh, I fixed #1531 as well.

If we didn't break anything, we look to see if the object can now get a
theme.

Themes
------

Without themes, we can have very powerful items, but they're like
randarts - random collections of attributes. Themes allow us to decide,
during an item's creation, that it's going down a particular path. So I
wrote ego_themes.txt, which sets out what these themes are. At the moment
they're all recognisable, because they're the high-end/compound egos I
removed from ego_item.txt earlier on.

Themes[] are a global array like e_info[], which have N: and D: lines
exactly like ego_item.txt. They also have T: lines, but these only have
tval, svals and depths - no "level" or "commonness". So far so obvious.
*obj_find_theme* builds an allocation table of legal themes just like
*make_artifact* and *obj_find_affix*, checking depth and tval/sval.

But there the similarity ends - themes don't have an inherent commonness,
they have a number of component affixes, each of which has a weighting.
We check the object to see if it has any of these affixes already, and
record their weight. Then we multiply by the proportion of total weight
to get the actual likelihood of acquiring that theme. These weightings
were chosen very carefully, because often only one theme will be
available to an object, and we have to have an absolute percent chance
of getting it, as well as an allocation table if there are several to
choose from. The total weight of the relevant affixes on the item is
multiplied by (itself x 4 / total weight of all affixes in the theme)
to get the percent chance (in the code we use x8 and use randint0(200)
so we're using half-percent granularity).

So here's a worked example: the theme "of Resistance". It has six
constituent affixes: the four resists (each weighted 7), Reinforced
(for the to_a boost, weighted 2) and Durable (for the IGNORE flags,
weighted 4). Durable items contribute to a lot of themes, but usually
with very small probability - this is actually the largest weighting
of Durable, because it reflects the nature of the theme. So the total
weighting of all affixes in this theme is 34.

If we have only one of them, we can't get the theme. You need at least
two affixes to get any theme.

If we have two of the resists, we have a total weight of 14. The percent
chance of acquiring the theme is (14*14*4)/(34*100) = 23%.

If we have three of the resists, we have weight of 21. The percent chance
of acquiring the theme is (21*21*4)/(34*100) = 51.5%.

If we have all four resists, we have a (28*28*4)/(34*100) = 92% chance
of acquiring the theme. (If we tweaked the weightings of Durable and
Reinforced down to 2 and 1 respectively, this would be over 100%, which
is probably what we want.)

By contrast, if the two affixes we have are Reinforced and Durable, we
have weight of 6, which gives a (6*6*4)/(34*100) = 4% chance of acquiring
the theme. Both of them and one resist makes 19% - less useful than having
two of the resists.

Another example worth mentioning is lanterns of True Sight - a theme which
has only two affixes. Both have weights of 100, so if we get them, we will
automatically get this theme.

Blessed weapons have three affixes, but one of them has a weighting of zero
(of Dweomercraft, the one which provides the random ability - also on
Gondolin and \*Slay\* Evil weapons, Lothlorien bows etc.). This means it
doesn't contribute to the weighting, but it is applied in *obj_apply_theme*
after the theme is chosen. This function simply cycles through all the
affixes in the theme and applies all the ones that aren't already on the
item. Since you can specify the same affix more than once in a theme
(e.g. for extra combat bonuses, or extra random flags), we allow the
second and subsequent ones to be applied.

Note that *obj_apply_theme* doesn't actually set the o_ptr->affix for
the affixes it applies. This is deliberate: many themes have more than
MAX_AFFIXES. Also, once we acquire a theme we're unable to modify the
item further (like an artifact), so it doesn't really matter too much.
Note also that branding spells **will** (currently) work on non-themed
items, providing they have < MAX_AFFIXES. I like this, but others might
not (more below).

ID, naming and saving
---------------------

ID-by-use works reasonably well for affixes, though I had to write
*object_affix_is_known* to check from first principles whether we know
all about an affix. The IDENT\_ flags don't work because we don't know
how many affixes we're trying to know, and I decided against recording
o_ptr->known_affixes in favour of working it out on the fly.
*object_theme_is_known* is just a wrapper which makes sure that we know
all the affixes in a theme. This is pretty basic but actually seems to
work ok - both magical ID and ID-by-use seem to work ok, and the ego
knowledge menu shows affixes once they're known (it doesn't talk about
the new mods to weight/base AC/dice/sides, but otherwise works ok).

Finally, with noz's help, we sorted out the prefix and suffix names of
the object, which are the theme or the best affixes in the absence of a
theme (so you can get Emerald weapons of Gondolin, or Broken ones, etc.).
There is still some thinking to do here in relation to ID and naming,
some of which was discussed on IRC (d_m/fizzix suggested "synthetic"
affixes which change the name but no properties - this seems like a
good solution, but it would be a shame to lose all the flavour of the
affix names).

The savefile now stores the indices of the theme (in the old
o_ptr->ego->eidx slot) and the affixes. I also took a cue from Gabe and
we now record all of MAX_PVALS, MAX_AFFIXES, OF_SIZE and OF_BYTES in the
savefile, so if they change we don't have to write a new function. Oh, and
we also store o_ptr->extent, which is food/fuel/charges/gold/chest level,
fixing #1540. Ego items in old savefiles will retain all their actual
properties (flags, plusses etc.), but will lose their names. I'm happy
to write a converter to restore these names if people think it's
important, but it looks like we might be heading for major savefile
breakage for 4.0 anyway.

Next steps
==========

Code cleanup
------------
I need to get rid of remaining references to o_ptr->ego and remove it
from the object_type struct. Also renaming ego_stuff to affix_stuff
would be helpful - I've been a bit lazy about this, in case the whole
thing was rejected. I also need to write accessors or #defines for
things like AFFIX_IS_PREFIX and so on.

I'm also wondering whether it's possible/desirable to de-globalise the
themes[] and e_info[] arrays, and make them local to obj-make (or
wherever). I don't know enough about C to know how important or difficult
this would be. Similarly, there are lots of comments in the code about
making arrays read-only (e.g. #1202) - again, I'm not sure I really
understand this issue properly.

Knowledge and ID
----------------

Update: the ego knowledge menu now works properly, as does the object
knowledge menu. An item's affixes are now listed in the 'I'nspect screen
(this may or may not be desirable long-term, but is certainly useful for
testing). Flavour text is also shown for all affixes where it exists.

Rune-based ID is now working, with a separate knowledge menu for known
runes. Unknown runes will soon have random names (#1574), and both
known and unknown runes will be listed on the Inspect screen.

IMO we should no longer show the base AC or dice of an object, because
these are no longer so static - lots of the Ey prefixes change one or
the other. This fits nicely with reducing the amount of info available
and forcing people to walk over and fetch stuff. Interested in people's
thoughts on this (and see also #1551).

Naming and base items
---------------------

We need to agree a strategy for naming items with multiple affixes.
Personally I favour adopting the position that an object's displayed
name does not give you complete information about all its properties,
but others may disagree. Also, affixes can be applied more than once
(meaningless for flag affixes, but important for hit/dam/ac etc.). I
like the idea of Sharp, *Sharp* and **Sharp** or something, to denote
multiple applications of an affix. UPDATE: this is now the single most
important outstanding issue. There is consensus that it is unrealistic
to convey all information in the item's name, but no consensus on a
naming hierarchy or categorisation.

Randarts and randomness
-----------------------

My original intention was that themes were more random, i.e. that not
all affixes in a theme would be applied every time. I didn't implement
themes like this because I didn't want the outcry of "my Gondolin weapon
doesn't have RES_DARK" etc. But I still think it would be good to have
more variation. If we want to use themes to guide randart generation,
this would become more important. One way is to add a third field to
the A: lines in ego_themes.txt and specify the percent chance of adding
that affix during obj_apply_theme. We could keep the wolves at bay by
ensuring that these were all 100 for the traditional ego types.

The consideration of randarts divides into three separate issues:

* how many of the standart set to use (sub-divides into all, none or
  some (which further subdivides into a set proportion or a chosen
  proportion))
* how many randarts to generate (sub-divides into none, the same number
  as the number of unused standarts, or infinite)
* how much to randomise any standarts (straight choice between completely
  random, i.e. new base item / name / properties, or partially random,
  i.e. same base item and name, with some core attributes kept and
  others randomised)

The GREAT_EGO check could be used for generating infinite randarts
(since it could lift the affix level from "uber" to "artifact").

Object modification
-------------------

This branch opens up a lot of possibilities w.r.t. alchemy, forging etc.
(See also #1550). Nothing to worry about immediately, except whether to
retain or remove the branding spells/prayers. Arguably the prayer (for
branding melee weapons) is now actually useful where it wasn't before.
It now checks that the object doesn't already have a brand. The ammo
branding spell was already too good, and is probably even more so now
(but we could always temper it by making it reduce o_ptr->number by 50%,
or something like that).

My view remains that we should allow spells and effects to modify objects,
and just be careful to limit their power. (We could use a limit lower
than MAX_AFFIXES, for instance.)

Balancing
---------

Finally, of course, there's a ton of balancing tweaking to be done.
Some affixes are available on items which weren't before (e.g. of
Warding), and others aren't (e.g. of Dweomercraft), purely because
of what I was testing when I added them. Doing this balancing means
adjusting the stats code to record affix and theme indices (it
already records all the actual item info). I am quite happy for
people to crawl all over ego_items.txt and ego_themes.txt and adjust
all the T: lines, as I have not spent long checking what affixes
are available on which items at which depth and affix level:
mithril shields don't seem to be able to acquire any affixes at all!

A spreadsheet of the current affix distribution is at
https://docs.google.com/spreadsheet/ccc?key=0AlI-IK5uLWbEdEItWWRZY0RqSVhoeHpBWjU3OG02UHc&hl=en_US
. To see what is available on any particular type of item, scroll up
and down (and order by depth if you like). This immediately tells us
that we need more affixes on cloaks, gloves, boots and launchers. We
probably need more armour affixes in general: there are 12 AC/weight
affixes for armour (two bad), compared with 28 hit/dam/dice affixes
for weapons (which need much more careful distribution). We can
probably also converge some of the affixes: there is now no need for
special affixes for launchers or ammo (Accuracy, Power, Wounding).

There's a "proposed" spreadsheet at
https://docs.google.com/spreadsheet/ccc?key=0AlI-IK5uLWbEdFV2UzJKSjdKZmEtLWlkTXF2amd6b3c&hl=en_US
, which contains my first partial attempt at balancing the distribution.

I also think that we need to check the balance between obj_alloc
(the allocation table for all objects) and obj_alloc_great (the one
for "good" or "great" objects). Some potions/scrolls with the
OF_GOOD flag may now be too common, and some others perhaps ought to
get it (and some devices).

Finally, I haven't done much with the old "cursed" egos - they've
been re-enabled as affixes, but not split up into themes. Once we've
settled on our "new curses" system I'll come back and sort out affixes
for proper mixed-blessing items. In the meantime I will disable them
prior to any release.

Other issues
------------

A bunch of things occurred to me while doing all this stuff (I'll make
tickets post-merge)

* the slay cache can now go, as we're not constrained to a small number
  of slay combinations which are worth caching
* we could have a low-level code module for generating lookup tables
  like flag names (currently duplicated in obj-flag.c and init2.c)
  and tvals (which we could now do from object_base.txt, removing the
  need for hard-coding - we could also seek to remove tvalsval.h ...)
* affixes could change the display colour of an object (Ey has this,
  and fizzix thought of it too - #837)
* affixes could be used to generate ego jewelry, which allows
  re-thinking of what non-ego jewelry ought to be ... (it would be
  easy to regenerate the existing rings/amulets using affixes and
  themes, while enjoying the extra randomness)
* allocation of kinds could use the alloc_entry struct (presumably it
  was written before that struct?)
* items with alloc_prob 0 should not appear in knowledge menus (the old
  Bronze DSM problem, now occurring with stuff like Adamantite Plate and
  Maces of Disruption) - not sure if this is related to fizzix's bug
  report
* should maxima really be sparse? z_info->e_max is set not as the number
  of e_info entries but the index of the highest. Is this necessary?

Backporting to V 3.x
====================

There is a problem with using this code to generate only the items found
in 3.3.x's ego_item.txt. In order to get a theme, an item must already
have at least two of that theme's affixes. If we take the example of
\*slay\* dragon, none of its three affixes (+CON, RES_FEAR or KILL_DRAGON)
are ever found separately on weapons in 3.3.x. So you could never create
an item with this theme using this system.

If you got rid of themes altogether and simply turned them back into rare
affixes, you would need to make sure that an item could only have one
affix (so that it could not get both Gondolin and Defender, for example) -
thereby losing the point of affixes altogether. At that point you just
have a minor variation on the old ego system, with some parser changes.
