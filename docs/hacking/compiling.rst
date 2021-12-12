Compiling Instructions
======================

The methods for compiling Angband vary by platform and by build system. If
you get Angband working on a different platform or build system please let us
know so we can add to this file.  Unless otherwise noted, all the commands
listed are to be run from top-level directory of the Angband source files.

.. contents:: Contents
   :local:

macOS
-----

To build the new Cocoa front-end::

    cd src
    make -f Makefile.osx

That'll create a self-contained Mac application, Angband.app, in the directory
above src.  You may use that application where it is or move it to wherever
is convenient for you.

Debug build
~~~~~~~~~~~

This will generate a debugging build like that described in the Linux section::

    cd src
    make -f Makefile.osx clean
    make -f Makefile.osx OPT="-g -O1 -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address"

The clean step is there to clean out object files that were compiled with the
default options.  The "-g" adds debugging symbols.
"-O1 -fno-omit-frame-pointer" dials back the optimization to get call stack
traces that are easier to interpret.  For even clearer call stack traces, you
could add "-fno-optimize-sibling-calls" to the options or omit optimization
entirely by replacing "-O1 -fno-omit-frame-pointer" with "-O0".
"-fsanitize=address -fsanitize=undefined" enables the AddressSanitizer and
UndefinedBehaviorSanitizer tools.

To run the generated executable under Xcode's command-line debugger, lldb, do
this if you are already in the src directory from the compilation step::

    cd ../Angband.app/Contents/MacOS
    lldb ./angband

Test cases
~~~~~~~~~~

To compile and run the unit tests, do this::

    cd src
    make -f Makefile.osx tests

If you want to rerun just one part, say monster/attack, of the unit tests,
that's most easily done by::

    cd src/tests
    bin/tests/monster/attack

The reason for changing directories to src/tests is to match up with how the
tests were compiled:  they expect Angband's configuration data to be in
../../lib.

Statistics build
~~~~~~~~~~~~~~~~

The Mac front end bypasses main.c and can not use the statistics front end.
It is possible to enable the debugging commands related to statistics (see
the descriptions for ``S``, ``D``, and ``P`` in :ref:`DebugDungeon`).  To do so,
include "-DUSE_STATS" in the setting for OPT passed to make.  The equivalent
of the standard build with those debugging commands enabled would be::

    cd src
    make -f Makefile.osx OPT="-DUSE_STATS -O2"

If you had already built everything without statistcs enabled, you would need to
run either "rm wiz-stats.o" or "make -f Makefile.osx clean" immediately after
running "cd src".

Linux / other UNIX
------------------

Native builds
~~~~~~~~~~~~~

Linux builds using autotools. There are several different front ends that you
can optionally build (GCU, SDL, SDL2, and X11) using arguments to configure
such as --enable-sdl, --disable-x11, etc. Each front end has different
dependencies (e.g. ncurses, SDL libraries, etc).

If your source files are from cloning the git repository, you'll first need
to run this to create the configure script::

    ./autogen.sh

That is not necessary if your source files are from the source archive,
a .tar.gz file, for a release.

To build Angband to be run in-place, then run this::

    ./configure --with-no-install [other options as needed]
    make

That'll create an executable in the src directory.  You can run it from the
same directory where you ran make with::

    src/angband

To see what command line options are accepted, use::

    src/angband -?

Note that some of Angband's makefiles (src/Makefile and src/tests/Makefile are
the primary offenders) assume features present in GNU make.  If the default
make on your system is not GNU make, you'll likely have to replace instances
of make in the quoted commands with whatever will run GNU make.  On OpenBSD,
for instance, that is gmake (which can be installed by running "pkg_add gmake").

On systems where there's several C compilers, ./configure may choose the
wrong one.  One example of that is on OpenBSD 6.9 when building Angband with
SDL2:  ./configure chooses gcc but the installed version of gcc can't handle
the SDL2 header files that are installed via pkg_add.  To override ./configure's
default selection of the compiler, use::

    env CC=the_good_compiler ./configure [the appropriate configure options]

Replace the_good_compiler in that command with the command for running the
compiler that you want.  For OpenBSD 6.9 when compiling with SDL2, you'd
replace the_good_compiler with cc or clang.

To build Angband to be installed in some other location, run this::

    ./configure --prefix /path/to [other options as needed]
    make
    make install

On some BSDs, you may need to copy install-sh into lib/ and various
subdirectories of lib/ in order to install correctly.

Compilation with CMake
~~~~~~~~~~~~~~~~~~~~~~

The compilation process with CMake requires a version greater than 3,
by default the compilation process uses the X11 front end unless
one or more of the other graphical front ends are selected. The graphical front
ends are: GCU, SDL, SDL2 and X11.  All of the following generate a
self-contained directory, build, that you can move elsewhere or rename.  To
run the result, change directories to build (or whatever you renamed it to) and
run ./Angband .

To build Angband with the X11 front end::

    mkdir build && cd build
    cmake ..
    make

If you want to build the X11 front end while building one of the other
graphical front ends, the option to pass to cmake is -DSUPPORT_X11_FRONTEND=ON .

To build Angband with the SDL front end::

    mkdir build && cd build
    cmake -DSUPPORT_SDL_FRONTEND=ON ..
    make

To build Angband with the SDL2 front end::

    mkdir build && cd build
    cmake -DSUPPORT_SDL2_FRONTEND=ON ..
    make

To build Angband with the GCU front end::

    mkdir build && cd build
    cmake -DSUPPORT_GCU_FRONTEND=ON ..
    make

On OpenBSD (at least with OpenBSD 6.9), there's known issues with detecting
the software needed for the GCU front end.  As a workaround, you could use
this instead::

    mkdir build && cd build
    mkdir -p ncursesw/include/ncursesw
    ln -s /usr/include/ncurses.h ncursesw/include/ncursesw
    mkdir -p ncursesw/lib
    ln -s /usr/lib/libncursesw.so* ncursesw/lib
    cmake -DCMAKE_PREFIX_PATH=`pwd`/ncursesw -DSUPPORT_GCU_FRONTEND=ON ..
    make

You can build support for more than one of the graphical front ends by setting
all the desired SUPPORT_*_FRONTEND options when running cmake (the exception to
this are the SDL and SDL2 which can not be built at the same time).  If you
want the executable to have support for sound, pass -DSUPPORT_SDL_SOUND=ON or
-DSUPPORT_SDL2_SOUND=ON to cmake (as with the SDL and SDL2 front ends, you
can't build support for both SDL and SDL2 sound; it is also not possible to
build the SDL front end with SDL2 sound or the SDL2 front end with SDL sound).

Cross-building for Windows with Mingw
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Many developers (as well as the auto-builder) build Angband for Windows using
Mingw on Linux. This requires that the necessary Mingw packages are all
installed.

This type of build now also uses autotools so the overall procedure is very
similar to that for a native build.  The key difference is setting up to
cross-compile when running configure.

If your source files are from cloning the git repository, you'll first need
to run this to create the configure script::

        ./autogen.sh

That is not necessary if your source files are from the source archive,
a .tar.gz file, for a release.

Then configure the cross-comilation and perform the compilation itself::

	./configure --enable-win --build=i686-pc-linux-gnu --host=i586-mingw32msvc
	make

One way to run the generated executable, src/angband.exe, with wine is to first
set up symbolic links to the executable and DLLs it uses::

	ln -s src/angband.exe .
	ln -s src/win/dll/*.dll .

Then use wine::

	wine angband.exe

Mingw installs commands like 'i586-mingw32msvc-gcc'. The value of --host
should be that same command with the '-gcc' removed. Instead of i586 you may
see i686, amd64, etc. The value of --build should be the host you're building
on. (See http://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.68/html_node/Specifying-Target-Triplets.html#Specifying%20Names for
gory details of how these triplets are arrived at)

TODO: except for recent versions (after Angband 4.2.3) you likely need to
manually disable curses (add --disable-curses to the options passed to
configure), or the host curses installation will be found causing the build
process to fail when linking angband.exe (the error message will likely be
"cannot find -lncursesw" and "cannot find -ltinfo"). More checking of
permissible combinations to configure is necessary. The --enable-release and
--enable-more-gcc-warnings options are safe to use with --enable-win.

Debug build
~~~~~~~~~~~

**WARNING** this build is intended primarily for debugging purposes. It might have a somewhat slower performance, higher memory requirements and panic saves don't always work (in case of a crash there is a higher chance of losing progress).

When debugging crashes it can be very useful to get more information about *what exactly* went wrong. There are many tools that can detect common issues and provide useful information. Two such tools that are best used together are AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan). To use them you'll need to enable them when compiling angband::

    ./configure [options]
    SANITIZE_FLAGS="-fsanitize=undefined -fsanitize=address" make

Note that compiling with this tools will require installing additional dependancies: libubsan libasan (names of the packages might be different in your distribution).

There is probably a way to get these tools to work on Windows. If you know how, please add the information to this file.

Test cases
~~~~~~~~~~

To compile and run the unit tests if you used ./configure --with-no-install,
do this::

    make tests

If you want to rerun just one part, say monster/attack, of the unit tests,
that's most easily done by directly running from the top-level directory::

    src/tests/bin/monster/attack

There's a separate set of tests that use scripts to control a character in
the full game.  To run those tests, you'll need to enable the test module
when running configure and then run the run-tests script in the top-level
directory::

    ./configure --with-no-install --enable-test
    make
    ./run-tests

To compile and run the unit tests and run the run-tests script while using
CMake, do the following::

    mkdir build && cd build
    cmake -DSUPPORT_TEST_FRONTEND=ON ..
    make alltests

If you only want the unit tests while using CMake, its a little simpler::

    mkdir build && cd build
    cmake ..
    make allunittests

Statistics build
~~~~~~~~~~~~~~~~

If building directly for Linux/Unix using configure, you can get the statistics
front end and support for the debugging commands related to statistics (see
the descriptions for ``S``, ``D``, and ``P`` in :ref:`DebugDungeon`) by
including --enable-stats in the options to configure.  For that to work, you'll
need to have sqlite3's headers and libraries installed (on Debian and Ubuntu,
the libsqlite3-dev package and its dependencies provides those).   If using
CMake, pass -DSUPPORT_STATS_FRONTEND=ON to cmake to get the statistics front
end and support for the debugging commnands related to statistics; like builds
with configure that use --enable-stats, that requires sqlite3.  With CMake, you
also have an the option to only include support for the debugging commands
related to statistics:  pass -DSUPPORT_STATS_BACKEND=ON to cmake and either do
nothing for SUPPORT_STATS_FRONTEND or explicitly turn it off by passing
-DSUPPORT_STATS_FRONTEND=OFF to cmake.

When cross-compiling for Windows, the statistics front end is not useful
(the Windows front end bypasses main.c and can not use the statistics front
end).  With configure, you could include support for debugging commands
related to statistics by setting CFLAGS to include -DUSE_STATS::

    ./configure [your cross-compiling options] --enable-win CFLAGS=-DUSE_STATS

Windows
-------

Using MinGW
~~~~~~~~~~~

This build now also uses autotools, so should be very similar to the Linux
build. Open the MinGW shell (MSYS) by running msys.bat.

If your source files are from cloning the git repository, you'll first need
to run this in the directory to create the configure script::

        ./autogen.sh

That is not necessary if your source files are from the source archive,
a .tar.gz file, for a release.

Then run these commands::

        ./configure --enable-win
        make

The install target almost certainly won't work

Following build, to get the program to run, you need to copy the executable
from the src directory into the top-level dir, and copy 2 DLLs (libpng12.dll
and zlib1.dll) from src/win/dll to the top-level dir

Using Cygwin (with MinGW)
~~~~~~~~~~~~~~~~~~~~~~~~~

Use this option if you want to build a native Windows executable that
can run with or without Cygwin.

Use the Cygwin setup.exe to install the mingw-gcc-core package and any
dependencies suggested by the installer.

If your source files are from cloning the git repository, you'll first need
to run this in the directory to create the configure script::

        ./autogen.sh

That is not necessary if your source files are from the source archive,
a .tar.gz file, for a release.

Then run these commands::

	./configure --enable-win --with-no-install --host=i686-pc-mingw32
	make

As with the "Using MinGW" process, you need to copy the executable and
DLLs to the top-level dir.

If you want to build the Unix version of Angband that uses X11 or
Curses and run it under Cygwin, then follow the native build
instructions (./autogen.sh; ./configure; make; make install).

Using MSYS2 (with MinGW64) 
~~~~~~~~~~~~~~~~~~~~~~~~~~

Install the dependencies by::

	pacman -S make mingw-w64-x86_64-toolchain mingw-w64-x86_64-ncurses

Additional dependencies for SDL2 client::

	pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_gfx \
		  mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_ttf

Then run the following to compile with ncurse::

	cd src
	make -f Makefile.msys2

For SDL2, do::

	cd src
	make -f Makefile.msys2.sdl2

Go to the root of the source directory and start angband by::

	./angband.exe -uPLAYER

The ncurse client may not be able to start properly from msys2 shell, try::

	start bash

and run::

	export TERM=
	./angband.exe -uPLAYER

Using eclipse (Indigo) on Windows (with MinGW)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* For eclipse with EGit, select File | Import..., Git | Projects from Git, Next >
* Clone your/the upstream repo, or Add your existing cloned repo, Next >
* Select "Use the New Projects Wizard", Finish
* In the New Project Wizard, select C/C++ | Makefile Project with Existing Code, Next >
* Give the project a name (Angband),
  * navigate to the repo you cloned in "Existing Code Location",
  * Select "C", but not "C++"
  * Choose "MinGW GCC" Toolchain, Finish
* Once the project is set up, r-click | Properties
* Go to C/C++ Build | Toolchain Editor, select "Gnu Make Builder" instead of "CDT Internal Builder"
* go to C/C++ Build, uncheck "Generate Makefiles automatically"

You still need to run ./autogen.sh, if your source files are from cloning the
git repository, and ./configure manually, outside eclipse (see above)

Using Visual Studio
~~~~~~~~~~~~~~~~~~~

Blue Baron has detailed instructions for setting this up at:

    src/win/angband_visual_studio_step_by_step.txt

Statistics build
~~~~~~~~~~~~~~~~

The Windows front end bypasses main.c and can not use the statistics front end.
It is possible to enable the debugging commands related to statistics (see
the descriptions for ``S``, ``D``, and ``P`` in :ref:`DebugDungeon`).  To do
so, set your compiler options so that the USE_STATS preprocessor macro is set.
When using mingw (either stand-alone or as part of Cygwin) and configure,
include CFLAGS=-DUSE_STATS in the options to configure to do that.

Documentation
-------------
To convert the documentation from restructured text to the desired output
format, you'll need Sphinx ( https://www.sphinx-doc.org/en/master/ )
and, unless you change the theme in the documentation configuration, the
sphinx-better-theme ( https://pypi.org/project/sphinx-better-theme/ ; which
can be installed via pip using::

	pip install sphinx-better-theme

).

With those utilities in place and sphinx-build in your path, you can perform
the conversion by running::

	make html

in the docs subdirectory of the top-level directory in the source files.
That will generate a _build/html directory with the result of the conversion;
_build/html/index.html is the top-level help with links to everything else.

Other output formats besides HTML are possible.  Run::

	make

without any arguments in the docs subdirectory to see the formats that Sphinx
can generate.
