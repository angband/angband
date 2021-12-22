This page is a checklist of things to do immediately before a release.  For
the broader role of release management, please see the ReleaseManager page.

Before release:

* Check it compiles on:
   * Linux with and without autoconf: ncurses, x11, sdl (ask d_m or magnate to confirm)
   * OpenBSD + FreeBSD (ask Aerdan to confirm; nudge Edd to build the OpenBSD package)
   * Windows: native, sdl (ask fizzix or Blubaron to confirm)
   * Mac OS X (Carbon and Cocoa - ask myshkin to confirm)
* Check for no warnings
* Proof the in-game help for typos and obsolete refs
* Update docs:
   * changes.txt (separate gameplay changes from bugfixes and code refactoring)
   * thanks.txt (add any new contributors since the last release)
   * consider a new xyz.txt to highlight major changes this version (see
     330.txt for example)
   * check that all docs in lib/help are up-to-date with gameplay changes
   * check copyrights and licences are correct and up-to-date
   * don't forget the top-level readme.txt!
* Check all version strings
   * configure.ac
   * src/Makefile.src
   * src/angband.man
   * src/buildid.h
* git tag -a version; git push official tag version

Release:

* Windows native (automatically built)
* OS X (automatically built)
* Debian (Magnate to build, test and push to mentors.debian.net)
* Announce on rgra, forum (not sure if all of these are still valid)
   * http://en.wikipedia.org/wiki/Angband_(video_game)
   * http://www.roguebasin.com/
   * http://www.freshmeat.net/
   * http://www.shacknews.com and its chat thread
   * http://arstechnica.com and its gaming forum
   * http://www.macupdate.com/info.php/id/15184/angband
   * http://digg.com
   * http://reddit.com
   * http://slashdot.org
   * http://kottke.org
   * http://xkcd.com and its forums
