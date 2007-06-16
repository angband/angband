Angband 3.0.8
=============

Bug reports to <bugs@rephial.org>
Website at     http://rephial.org/


Angband is a graphical dungeon adventure game that uses textual characters
to represent the walls and floors of a dungeon and the inhabitants therein, 
in the vein of games like NetHack and Rogue.

The game's website is http://rephial.org/, which contains various help files,
changelists, and development information.  There are also files included here
which give valuable information, like changes.txt and the help files in
lib/help/, which can be viewed in-game.

Angband is currently maintained by a development "team" headed by Andrew
Sidwell.  Please see the "thanks.txt" file for a full listing of credits.


If you've downloaded the source, please be aware that there are precompiled
versions available for at least Mac OS X, Windows, and RISC OS, available
at the main site.  However, if you want to compile, you may find the page
<http://rephial.org/wiki/Compiling> useful.


For information on variants, patches, and other assorted things, you're best
off looking at the main community site for Angband, <http://angband.oook.cz/>.
It has the Angband forum, provides access to the Angband newsgroup, and allows
you to upload character dumps and screenshots.

Bug reports should be sent to either the forum, in the "Vanilla" section, or
via email to bugs@rephial.org.



=== Special instructions for certain platforms ===

Make sure you keep all your savefiles in the proper place, and if you load a
savefile from the wrong place, note that the game may decide to re-save your
savefile in the proper place when you quit.

Some kinds of archive (namely Zip files) don't handle empty directories, so
there are "delete.me" files in those.  You may safely delete these files if
they annoy you.


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
