The "lib" directory contains all of Angband's special sub-directories.


=== Directory "lib/gamedata" ===

The "lib/gamedata" directory contains various special text data files.

The *.txt files are utf8 template files used to construct the data 
arrays for the game. These arrays describe the "terrain features", 
"object kinds", "artifacts", "ego-items", "monster races", and "dungeon 
vaults", "player races", "player classes", and many other things.

The utf8 template files are easier to edit than hard-coded arrays, prevent
compilation errors on some machines, and also shrink the size of the binary
executable, and also provide a user-readable spoiler file of sorts.

These files are modifiable.  It is recommended that if you do modify any of
these files, you put the modified version in your user directory (lib\user
on Windows, ~/Documents/Angband on macOS, ~/.angband/Angband on Linux/Unix),
where it will be read preferentially.  Look at the help file
lib/help/modify.txt for more details.


=== Directory "lib/customize" ===

The "lib/customize" directory contains the "pref files" - files which help
the game deal with graphics, keyboard input and messages, among other things.

In general, these files are used to "customize" aspects of the game for
a given site or a given player.


=== Directory "lib/help" ===

The "lib/help" directory contains the "online help" files.

This directory is used to search for normal "online help" files.


=== Directory "lib/screens" ===

The "lib/screens" directory contains various special text data files.

The 'news.txt' file is displayed to the user when the game starts up.  It
contains basic information such as my name and email address, and the names
of some of the people who have been responsible for previous versions of
Angband.  You may edit this file (slightly) to include local "site specific"
information such as who compiled the local executable.  You should refer the
user to a special "online help" file, if necessary, that describes any local
modifications in detail.  The first line of this file should be blank, and
only the next 21 lines should contain information.

The 'dead.txt' file is displayed to the user when the player dies.  It
contains a picture of a tombstone which is filled in with interesting
information about the dead player.  You should not edit this file.

The 'crown.txt' file is displayed to the user when the player wins.  It
contains a picture of a crown.  You should not edit this file.

These files should not be modified unless you know exactly what you are doing.


=== Directory "lib/fonts" ===

The "lib/fonts" directory contains the font files for use in the Windows and
SDL front ends.  More .fon files can be added and used if you wish.


=== Directory "lib/tiles" ===

The "lib/tiles" directory contains the .png images for the various tilesets,
as well as the file graphics.txt which the game uses to display the tiles
correctly.


=== Directory "lib/sounds" ===

The "lib/sounds" directory contains the .mp3 sound files, plus the file
sound.cfg which tells the game which sound files to use for which game events.


=== Directory "lib/icons" ===

The "lib/icons" directory contains the Angband icon files.


=== Directory "lib/user" ===

The "lib/user" directory is used to put various game-written configuration
files (such as the monster memory file, lore.txt, and a .prf file with the
subwindow configuration for the current character).  It is the destination
for character dumps, output from the death screen spoilers, and results
from the statistics front end or statistics debugging commands.  It also
contains the directories for scores, savefiles, and randart sets (all of
those may be held systemwide, as in some Linux/Unix installations), and
for user-created help files.


=== Directory "lib/user/save" ===

The "lib/user/save" directory contains "savefiles" for the players.

Each savefile is named the name of the character, or, on multi-user machines,
"UUU.NNN", where "UUU" is the player uid and "NNN" is the character name.

The savefiles should be portable between systems, assuming that the
appropriate renaming is perfomed.


=== Directory "lib/user/panic" ===

Is like "lib/user/save" but holds "savefiles" generated in repsonse to the
game receiving a fatal signal, typically because of a game crash.


=== Directory "lib/user/scores" ===

The "lib/user/scores" directory contains the "high score" files.

The "scores.raw" file contains the "high score" table, in a "semi-binary" form,
that is, all the bytes in the file are normal ascii values, but this includes
the special "nul" or "zero" byte, which is used to separate and pad records.
You should probably not attempt to modify this file with a normal text editor.
This file should be (more or less) portable between different platforms.  It
must be present (or creatable) for the game to run correctly.

On some multiuser systems there is a separate systemwide score file.


=== Direcctory "lib/user/archive" ===

Holds the randart set files when they are not in use by a character loaded
into the game.


