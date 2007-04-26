
                     _                _                     _
                    / \   _ __   __ _| |__   __ _ _ __   __| |
                   / _ \ | '_ \ / _` | '_ \ / _` | '_ \ / _` |
                  / ___ \| | | | (_| | |_) | (_| | | | | (_| |
                 /_/   \_\_| |_|\__, |_.__/ \__,_|_| |_|\__,_|
                                |___/

                 Version 3.0.8 by Andrew Sidwell and Pete Mack

           Send comments, bug reports and patches to the newsgroup:
      <news:rec.games.roguelike.angband> or at <http://angband.oook.cz/>.

           Based on Moria:   Copyright (c) 1985 Robert Alan Koeneke
               and Umoria:   Copyright (c) 1989 James E. Wilson
      Angband 2.0 - 2.6.2:   Alex Cutler, Andy Astrand, Sean Marsh,
                             Geoff Hill, Charles Teague, Charles Swiger
            2.7.0 - 2.8.5:   Ben Harrison
            2.9.0 - 3.0.6:   Robert Ruehlmann


Angband is a graphical dungeon adventure game that uses textual characters
to represent the walls and floors of a dungeon and the inhabitants therein, 
in the vein of games like NetHack and Rogue.

The current main website is http://angband.rogueforge.net/, which contains
various help files, changelists, and development information.  There also
are various files included here which give valuable information, like
changes.txt and the help files in lib/help/, which can be viewed in-game.

If you want compile the game, please check:
  <http://ajps.mine.nu/angband/wiki/BuildSystem>


For information on variants, patches, and other assorted things, you're best
off looking at both the main website and http://angband.oook.cz/, a fansite
which provides access to the Angband newsgroups, a place to upload character
dumps, and various spoilers.

Bug reports should go to <http://angband.rogueforge.net/trac/newticket>.
Please include your email address (even with a NOSPAM in there) so we can ask
for more details if necessary.



=== Getting a working copy ===

This version of Angband has support for Windows, Mac OS X, various Unixes
(both console and X11 versions), RISC OS, and DOS.  If you're interested in
writing support for a new version, then please do!  It will gladly be
incorporated into the official sources.

If you've downloaded the source, please be aware that there are precompiled
versions available for at least Mac OS X, Windows, and RISC OS, available
at the main site.  See compile.txt for details on compiling if you need to.




=== Special instructions for certain platforms ===

Make sure you keep all your savefiles in the proper place,
and if you load a savefile from the wrong place, note that the game
may decide to re-save your savefile in the proper place when you quit.

Some archive generation programs refuse to handle empty directories,
so special "fake" files with names like "DELETEME.TXT" may have been
placed into certain directories to avoid this problem.  You may safely
delete these files if you so desire.


=== Upgrading from older versions (and/or other platforms) ===

If you have been using an older version of Angband (and/or playing on a
different platform), you can move to the current Angband, bringing your
old savefiles, high score list, and other files with you.

To use an old savefile, simply copy it into the "lib/save" directory, changing
the name of the savefile (if necessary) to satisfy the requirements of the
platform you are using.  On multi-user systems, the savefile should be named
"UUU.NNN", where "UUU" is the userid of the player (on multi-user systems),
and "NNN" is the name of the "character" in the savefile.

Angband uses a platform independant file format for the binary file
used to store the high score list.  This file is named "scores.raw".  To
use an old high score list, simply copy it into the "lib/apex" directory.

Angband uses a set of ascii "user pref files" which are kept in the
"lib/pref" directory.  Most of these files can only be used on a small set
of platforms, and may need slight modifications when imported from older
versions.

If you're interested in what else lies in lib/, then please read
"lib/readme.txt".


=== NO WARRANTY ===

    BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

    IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.


--- Ben Harrison, Robert Ruehlmann, and Andrew Sidwell ---
