********************
Changes to savefiles
********************

So you've committed some changes, but something is broken on saving/loading. You realise that something about your changes requires an extra variable to be included somewhere in the savefile. You panic, as you have no idea how to do this.

It's ok, we've all been there. This page is an attempt to help you through
this the first time (after which it will become easier).

The concept
===========

Savefiles use a block-based system. Each block of the savefile has a name and a version number. Block versions are specific to that block, so it is fine for the "inventory" block to be version 3 while the "monsters" block is version 6.

When saving, the game writes the version number of each block. When loading, the version number tells the game how to load that particular block.

When you make changes, you need to identify which block(s) need to be changed.

The files
=========

There are only three files you need to worry about:

* src/savefile.[ch]
* src/save.c
* src/load.c

The first contains two structs, called savers[] and loaders[]. (It also contains other stuff you don't need to touch.) These structures are helpfully set out in a tabular format: block name, function name, block version. The savers[] structure contains exactly one entry for each block. The loaders[] structure can contain any number of entries for each block, providing each has a different version number.

The other two files contain the functions for the loaders and savers.

What to do
==========

When you have identified which block(s) will need changing, you need to follow these steps for each affected block:

#. Edit src/savefile.c to increment the block version number by 1 in the savers[] structure. Note that this is not adding a line, but amending an existing line in the structure.
#. Edit src/save.c and edit the relevant saver function (the one whose version number you just incremented), to save the data needed to make your changes work properly. You can save extra bytes if necessary - this is the whole point of the block-based system.
#. Edit src/savefile.c again, to add a new line in the loaders[] structure, defining a new function as the loader for the new version of the block you are changing. Add it immediately below the latest version of the block you are incrementing, and make sure the version number matches the one in step 1 above. You can theoretically call the function what you like, but the convention is to use rd_blockname_X, where X is the version number.
#. Edit src/load.c and add your new function. This is usually best done by copying and pasting the previous version of the loader function for this block, and amending it to reflect the changes you made in step 2 above.  Make sure you change the function name to match the one in step 3 though.
#. Edit src/savefile.h to add your new loader function here too. It should be obvious where it goes.

When you've done that for all affected blocks, you're ready to build and
test your changes.

Worked example
==============

Let's say that we've made a change to m_ptr - we've added a new uint16_t m_ptr->wibble, which we want to save in the savefile for each monster. So the block we're going to change is the "monsters" block. Let's say that the current version of the monsters block is 6.

#. We edit src/savefile.c and amend the line ``{ "monsters", wr_monsters, 6 },`` to ``{ "monsters", wr_monsters, 7 },``.
#. We edit src/save.c and find the wr_monsters() function. We add a line, ``wr_u16b(m_ptr->wibble);``, at a sensible point in the function (i.e. inside the loop over all monsters!).
#. We go back to src/savefile.c and find the line ``{ "monsters", rd_monsters_6, 6 },`` and immediately after it we add ``{ "monsters", rd_monsters_7, 7 },``.
#. We then open src/load.c and duplicate the rd_monsters_6() function using copy & paste. We change the name of the copy to rd_monsters_7 and add a line, ``rd_u16b(m_ptr->wibble);``, at the point in the function corresponding to where we made the change in 2 above. It's important to ensure that the changes are made in the same place in the saver and loader functions, otherwise the wrong data will be loaded.
#. We then open src/savefile.h and add the rd_monsters_7() function under the other six rd_monsters functions.

A note about items
==================

The functions wr_item and rd_item_X are special cases, as they are not savefile blocks in themselves but are called by three blocks (inventory, dungeon and stores - the three blocks containing carried, floor and store objects respectively). If you make changes to the data saved and loaded about objects you need to add the new version of rd_item to the function pointer definitions in load.c so that the correct version is used by each of the three blocks. This is in addition to incrementing the versions of the inventory, dungeon and stores blocks, but you do not need to write new loader functions, because the function pointers take care of this. (You do need to write a new rd_item_X function though.)

A note on this note: items carried by monsters are actually part of the "floor" item list saved in the "dungeon" block. Each item has a field called o_ptr->held_m_idx which records which monster (if any) is carrying it. So although you might think that items need saving in four places, it's actually only three. Who knows, one day we might come down to fewer than that ...
