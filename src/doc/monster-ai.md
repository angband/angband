# An overview of (some of) the monster intelligence code

Monster intelligence in Angband is fairly basic.  mon-move.c and mon-spell.c cover most of the AI, with the 'flow' code that keeps track of paths to the player living in cave-map.c.

Mostly the code is quite straightforward.  `mon-move.c:process_monsters()` is the starting point for monster AI, which branches into a number of other `process_monster_*()` functions depending on the right behaviour for that monster.

Each turn monsters will try the following in order:

  1. Regenerate HP and recover from timed effects
  2. Attempt to multiply
  3. Attempt a spell
  4. Try to move towards the player


# cave-map.c: flow code

Each grid on the level (in `struct square`, see cave.h) has two pieces of data stored for monster flow: its distance from the player ('noise') and the recentness of this information ('scent').

Noise is how far away the player is from a grid.  A value of 0 means that the player is on this grid, a value of 1 that the player is next door, 2 means one more away from that, etc.  You can see this visualised using the debug command '_', somewhat confusingly referred to in the code as the 'Ben hack'.

Noise is only calculated for traversable terrain.

Noise is only updated up to `z_info->max_flow_depth` (set as 32 at the moment), which means that it's not accurate after that point.  This is why we also store recentness.

Scent indicates recentness of the noise.  Because this algorithm was written in 1997 when computing resources were somewhat scarcer than they ever are now, this is encoded in a funny way.

The game keeps an internal scent counter, which is increased when flow is calculated, and when a grid's flow data is updated, its scent is set to this counter.  The counter starts out at 0 and increases by 1 each update until you reach 255 (this is because `scent` is one byte, see previous comment about 1997).  When it reaches 255, it rolls over to 128, and the game reduces the `scent` value of all grids by 128 (and any values below 128 are reset to 0), in effect preserving the last 128 updates' worth of data.

So, when updating flow information, the game marks:
  * the grid with the player on as `{ noise = 0, scent = now }`
  * adjacent grids as `{ noise = 1, scent = now }`
  * grids surrounding those as `{ noise = 2, scent = now }`,
  * and so on.

This one set of flow information is used for all monsters.  It is efficient but means that monsters that can't open or bash down doors, or otherwise deal with obstacles, will find it impossible to flow around them and find a different way to the player.

## Examples

The noise information in this situation:

```
#################
.@...............
#################
```

will be as follows: (in hexidecimal)

```
#################
10123456789abcdef
#################
```

`scent` in this case is the same for all displayed grids.

## When is scent used?

When the 'noise' value is equal, monster start tracking the 'scent' value instead.

Assuming that `MONSTER_FLOW_DEPTH` is 10, in the following scene, assuming the player has moved from the left to the right:

```
###############
..............@
###############
```

The noise values will be as follows:

```
###############
999999876543210
###############
```

And the scent values will be as follows: (where 0 = now,
                                                1 = now - 1,
                                                2 = now - 2 etc.)

```
###############
123456666666666
###############
```

When the noise value is the same, monsters will track scent instead.  This means... [===]

## When do updates happen?

Updates happen when `handle_stuff()` or `update_stuff()` are called and the `PU_UPDATE_FLOW` flag is set on the player upkeep 'update' variable*.  Sometimes `PU_UPDATE_FLOW` is paired with `PU_FORGET_FLOW`, which forces a complete forgetting of existing data before calculating new flow info.

The only time the update flag is set without the forget flag also being set is when the player moves.  Otherwise, both are set together i.e. when terrain changes from passable to impassable.

In the future it might be worth making all the grid modification functions in cave.c set the 'update flow' flag, since it would be less error-prone than the current set-up. (XXX)

* This is an obviously inappropriate place for this information, as it is not really about the player at all.  The cave struct itself should have update flags for this information. (XXX)

## Notes

Flow information is not saved in the savefile, so in theory if you quit the game and reload it when you've teleported away from a major threat, they are less likely to find you.
