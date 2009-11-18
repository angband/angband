The "lib" directory contains all of Angband's special sub-directories.


=== Directory "lib/apex" ===

The "lib/apex" directory contains the "high score" files.

The "scores.raw" file contains the "high score" table, in a "semi-binary" form,
that is, all the bytes in the file are normal ascii values, but this includes
the special "nul" or "zero" byte, which is used to separate and pad records.
You should probably not attempt to modify this file with a normal text editor.
This file should be (more or less) portable between different platforms.  It
must be present (or creatable) for the game to run correctly.


=== Directory "lib/edit" ===

The "lib/edit" directory contains various special ascii data files.

The *.txt files are ascii template files used to construct the data 
arrays for the game. These arrays describe the "terrain features", 
"object kinds", "artifacts", "ego-items", "monster races", and "dungeon 
vaults", "player races", "player classes", and many other things.

The ascii template files are easier to edit than hard-coded arrays, prevent
compilation errors on some machines, and also shrink the size of the binary
executable, and also provide a user-readable spoiler file of sorts.

These files should not be modified unless you know exactly what you are doing.


=== Directory "lib/file" ===

The "lib/file" directory contains various special ascii data files.

The 'news.txt' file is displayed to the user when the game starts up.  It
contains basic information such as my name and email address, and the names
of some of the people who have been responsible for previous versions of
Angband.  You may edit this file (slightly) to include local "site specific"
information such as who compiled the local executable.  You should refer the
user to a special "online help" file, if necessary, that describes any local
modifications in detail.  The first two lines of this file should be blank,
and only the next 20 lines should contain information.

The 'dead.txt' file is displayed to the user when the player dies.  It
contains a picture of a tombstone which is filled in with interesting
information about the dead player.  You should not edit this file.

The optional file 'time.txt' may be used to restrict the "times" at which
the game may be played, by providing specification of which hours of each day
of the week are legal for playing the game.  See 'files.c' for more details.
A missing file provides no restrictions, and an empty file will, by default,
forbid the playing of the game from 8am-5pm on weekdays.  This file is only
used on multi-user machines, and only if CHECK_TIME is defined, otherwise,
there are no restrictions.

These files should not be modified unless you know exactly what you are doing.


=== Directory "lib/help" ===

The "lib/help" directory contains the "online help" files.

This directory is used to search for normal "online help" files.


=== Directory "lib/info" ===

The "lib/info" directory contains the "online spoiler" files.

This directory is used to search for any "online help" file that cannot
be found in the "lib/help" directory.

This directory is empty by default. Many people use this directory for
"online spoiler files", many of which are available. Simply download 
whichever spoilers you want and place them in this directory.

Note that the default "help.hlp" file allows the "9" key to access a help
file called "spoiler.hlp", and allows the "0" key to access "user.hlp".

These special help files can thus be placed in the user's own "info"
directory to allow the on line help to access his files.


=== Directory "lib/save" ===

The "lib/save" directory contains "savefiles" for the players.

Each savefile is named "NNN" where "NNN" is the name of the character, or,
on some machines, the name of the character, or, on multi-user machines,
"UUU.NNN", where "UUU" is the player uid and "NNN" is the character name.

The savefiles should be portable between systems, assuming that the
appropriate renaming is perfomed.


=== Directory "lib/pref" ===

The "lib/user" directory contains the "user pref files", if any.

In general, these files are used to "customize" aspects of the game for
a given site or a given player.

See "src/files.c" for information on the proper "format" of these files.


=== Directory "lib/xtra" ===

The "lib/xtra" directory contains special system files, if any.

