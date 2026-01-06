This page is a checklist of things to do immediately before a release.  For
the broader role of release management, please see the ReleaseManager page.

Before release:

* Check that the dependencies in src/Makefile.inc are up to date;  on Linux/Unix
  run "cd src; make -f Makefile.std depgen" and then merge the generated
  src/Makefile.new into src/Makefile.inc
* Check it compiles on:
   * Linux with and without autoconf: ncurses, x11, sdl, sdl2
      * .github/wprkflows/linux.yaml running on GitHub's ubuntu-latest runner
         does that with CMake + Ninja.  Also on ubuntu-latest, it checks a
         configure-based build of stats front end + SDL sound and compilation
         with src/Makefile.std of ncurses + X11 + SDL and ncurses + SDL2.
         There is no automated checking for other distributions.
      * backwardsEric can manually check for whatever version of Debian
         he has in a Parallels virtual machine:  currently that is
         Debian 12 i686.
      * perhaps outdated:  ask d_m or magnate to confirm
   * OpenBSD + FreeBSD
      * backwardsEric can manually check for whatever version of OpenBSD
         he has in a Parallels virtual machine:  currently that OpenBSD 7.8
         amd64.
      * perhaps outdated:  Aerdan to confirm; nudge Edd to build the OpenBSD
         package
   * Windows: native, sdl, sdl2
      * .github/workflows/msys2.yaml checks compiling the Windows and SDL2
        front ends on MSYS2.  .github/workflows/nmake.yaml checks compiling
        the Windows front end with src/Makefile.nmake.
        .github/workflows/msbuild.yaml checks compiling the Windows front end
        using msbuild and the project files in  src/win/vs2019.  All use the
        windows-latest runner.
      * perhaps outdated:  ask fizzix or Blubaron to confirm
   * Mac OS X Cocoa
      * .github/workflows/mac.yaml compiles with src/Makefile.osx on the
         macos-latest runner.
      * backwardsEric can manually check on what he has available.  Currently
         that is macOS 12.7.6 + Xcode 14.2 on Intel and macOS 26.2 +
         Xcode 26.2 on Apple silicon.
      * perhaps outdated:  ask myshkin to confirm
* Check for no warnings
   * The GitHub workflows that have the most sensitive warning settings are:
     "Statistics Build" and "Makefile.std" from linux.yaml.  The workflows
     in nintendo.yaml and dos.yaml can catch unsafe type conversions not
     caught elsewhere.
* Proof the in-game help for typos and obsolete refs
* Update docs:
   * changes.txt (separate gameplay changes from bugfixes and code refactoring)
   * thanks.txt (add any new contributors since the last release)
   * consider a new xyz.txt to highlight major changes this version (see
     330.txt for example)
   * check that all docs in lib/help are up-to-date with gameplay changes
   * check copyrights and licences are correct and up-to-date
      * Copyright reported by 'V' in game is the copyright constant in
        buildid.c.  The SDL and SDL2 front ends also display the first line
        from the buildid.c's copyright constant in their About dialogs.
      * Copyright reported in the Mac front end's About dialog is from
        COPYRIGHT in Makefile.src.
      * Copyright reported in the manual is from copyright in docs/conf.py.
      * The Windows front end does not have an About dialog so there is no
        copyright information displayed there besides what 'V' does.
   * don't forget the top-level README.md!
* Check all version strings
   * configure.ac
   * src/Makefile.src
   * src/angband.man
   * src/buildid.c
   * docs/version.rst
   * README.md
* git tag -a version; git push official tag version
   * With git tag -a, the current practice is to use something like 4.2.6
     as the tag's name and "Version 4.2.6" as the annotation.
   * When pushing do not forget the tag.  It is best to push the tag and last
     revisions as one batch so the triggered release workflow has both.
     With upstream as the remote pointing to Angband's repository, lbranch
     as the name of the local branch whose HEAD points to what's to be released,
     and 4.2.6 as the name of the annotated tag for the release,
     "git push upstream refs/heads/lbranch:refs/heads/master refs/tags/4.2.6:refs/tags/4.2.6" would do the trick.

Release:

* Windows native (automatically built)
* OS X (automatically built)
* What's loaded on angband.live
* Debian (Magnate to build, test and push to mentors.debian.net)
* Announce on
   * http://rephial.org
      * Check out the gh-pages in the repository locally.  Follow the
         instructions in README.md to update rephial.org for the new release.
   * https://angband.live/forums in Vanilla.
   * http://www.roguebasin.com/
      * Use the "Announce new release" link under "New Roguelike Releases"
         to add an entry.  The format of the line used for 4.2.6's entry
         was "* <date> - [[Angband] <version number> [<link to specific release page on GitHub> released]"
      * Change the "Updated" entry in the box summarizing Angband at
         https://roguebasin.com/index.php/Angband .
   * http://reddit.com
      * Create a post in r/angband.
   * http://en.wikipedia.org/wiki/Angband_(video_game)
   * These have been used at some point in the past.  Nothing was done for
      them for 4.2.6:
      * rgra
      * http://www.freshmeat.net/
      * http://www.shacknews.com and its chat thread
      * http://arstechnica.com and its gaming forum
      * http://www.macupdate.com/info.php/id/15184/angband
      * http://digg.com
      * http://slashdot.org
      * http://kottke.org
      * http://xkcd.com and its forums
