Angband 3.1.0
=============

Angband is a graphical dungeon adventure game that uses textual characters
to represent the walls and floors of a dungeon and the inhabitants therein, 
in the vein of games like NetHack and Rogue.

Angband is currently maintained by a development "team" headed by Andrew
Sidwell.  Please see the "thanks.txt" file for a full listing of credits.

  Report bugs here:     bugs@rephial.org       
  The Angband website:  http://rephial.org/
  Angband forums:       http://angband.oook.cz/forum/

  32x32 graphics for X11 and Windows:
    http://angband.oook.cz/download/extra/graf-32x32-306.zip

  When upgrading, please read changes.txt!

  '?' in-game lets you browse the help system.

  For more information about the game, its variants, and somewhere to upload
  your characters and screenshots, please see http://angband.oook.cz/.

  If you've downloaded the source, please be aware that precompiled versions
  are available for at least Mac OS X, Windows, and RISC OS, available at the
  main site.  However, if you want to compile, try reading:
    http://rephial.org/wiki/Compiling




=== Special instructions for certain platforms ===

Make sure you keep all your savefiles in the proper place, and if you load a
savefile from the wrong place, note that the game may decide to re-save your
savefile in the proper place when you quit.


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
