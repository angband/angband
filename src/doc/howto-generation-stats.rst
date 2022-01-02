**********
StatsHowTo
**********

Description of what the Monte-Carlo stats generation program does and how to run it.

Activating Stats Collection
===========================

Stats collection is currently accessible through the debug commands 'S' or 'f'
(you will need ctrl-a to get into debug mode; the executable also has to have
been compiled with debugging enabled - see https://angband.readthedocs.io/en/latest/hacking/compiling.html
for how to do that).  You will be prompted for 3 things.

#. The number of iterations (start with a small number if you're playing around with it)
#. The type of simulation. There are currently two, *Clearing* and *Diving* (more on that later)
#. Clearing is selected you will be given the option of regenerating randarts after each sim.

After these options have been selected the stats generation will run.  Depending on the number of iterations this may take a while.  If you wish to monitor the progress, it is helpful to display messages in a subwindow.  After stats collection is completed the output is written to a stats.log file in the /user folder.

How Stats Collection Works
==========================

Stats collection is a very simplistic way to get estimates for item/gold/monster frequency in Angband.  The generation (per level) goes as follows.

#. A new level is created.
#. All items on the floor are recorded and deleted.  Items from vaults are marked as coming from vaults.
#. Go through the monster list.
    #. Note whether the monster is unique, out-of-depth (up to 10 levels), or
       very out-of-depth (more than 10 levels).
    #. Go through the objects carried (mimicked objects are not counted here;
       will have been counted when getting objects from the floor) and record
       and delete them.
    #. Delete the monster.  If in a clearing simulation and the monster is
       unique, it will be marked as killed so it won't appear in a later level.

Generally the items that are tracked cover the useful items in the game.  However, there really is no limit as to what items could be tracked, and it might be worthwhile to just track everything.

Diving vs. Clearing
===================

The *Clearing* option is used to simulate a game.  Every dungeon level from 1 to 100 is cleared, and all items are recorded.  After 100 levels have been completed, artifacts are reset to unseen and uniques are revived, and the simulation repeats for the number of iterations.  This is useful for getting relative artifact frequencies (comparing randarts to standarts, or evaluating changes in artifact frequency).

*Diving* is a little more simplistic.  It clears every 5th level N times, never marking artifacts as unseen or uniques as killed.  This simulation is good for getting relative frequencies of items or estimates for how likely out of depth monsters are discovered.  It will also help with basic frequencies, and total gold values.  It should not be used for any information regarding artifacts.

Both modes will note the first depth certain items are found in an iteration
and output a summary of that, across all iterations, in stats.log.

Note on Randart Regeneration
============================

Randart regeneration was broken until 4.2.2 (it would use the previously
generated randart set as the starting point rather than the standard artifacts).
As such, those versions would have any systematic variation in artifact power
compounded by successive iterations and could crash during artifact generation
if enough iterations were performed.

Drawbacks and Problems
======================

Currently there are a very large number of problems in mimicking normal Angband play.  These are outlined below.

* The player kills all monsters, even those that would not be fightable at the current depth.
* Killing all monsters is not feasible.  Especially without generating extra monsters on the level.
* Summoners like Qs will generate no loot, even though a draconic Q will generally give you a dragon with good loot.
* There is no way of tracking the loot that comes from pits.  This is problematic, since it is a huge part of gameplay.
* There is no way of tracking frequencies of vaults/pits/labyrinths/caverns
  and other macroscopic dungeon features.  An alternative for that is the
  disconnection statistics debugging command, 'D'.  Since 4.2.3, it collects
  and reports information about level types, room types, and other details
  about the dungeon layout.
