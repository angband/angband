Compiling Instructions
======================

The methods for compiling Angband vary by platform and by build system. If
you get Angband working on a different platform or build system please let us
know so we can add to this file.  Unless otherwise noted, all the commands
listed are to be run from top-level directory of the Angband source files.

.. contents:: Contents
   :local:

Linux / other UNIX with CMake
-----------------------------

The compilation process with CMake requires version 3.7 or later and the
commands used here require 3.13 or later.  For alternate ways of writing the
CMake commands that follow, including ways to be compatible with versions prior
to 3.13, see :doc:`cmake`.

By default the compilation process uses the X11 front end unless one or more of
the other graphical front ends are selected.  The graphical front ends are:
GCU, SDL, SDL2 and X11.  All of the following generate a self-contained
directory, ``build/game``, that you can move elsewhere or rename.  To run the
result, change directories to ``build/game`` or whatever you renamed it to) and
run ``./angband``.

To build Angband with the X11 front end::

    cmake -B build
    cmake --build build

If you want to build the X11 front end while building one of the other
graphical front ends, the option to pass to CMake is
``-DSUPPORT_X11_FRONTEND=ON``.

To build Angband with the SDL front end::

    cmake -DSUPPORT_SDL_FRONTEND=ON -B build
    cmake --build build

To build Angband with the SDL2 front end::

    cmake -DSUPPORT_SDL2_FRONTEND=ON -B build
    cmake --build build

To build Angband with the GCU front end::

    cmake -DSUPPORT_GCU_FRONTEND=ON -B build
    cmake --build build

You can build support for more than one of the graphical front ends by setting
all the desired SUPPORT_*_FRONTEND options when running CMake (the exception to
this are the SDL and SDL2 which can not be built at the same time).  If you
want the executable to have support for sound, pass ``-DSUPPORT_SDL_SOUND=ON``
or ``-DSUPPORT_SDL2_SOUND=ON`` to CMake (as with the SDL and SDL2 front ends,
you can't build support for both SDL and SDL2 sound; it is also not possible to
build the SDL front end with SDL2 sound or the SDL2 front end with SDL sound).

There are options to not build a self-contained installation and, instead,
organize the files for a typical Linux or Unix layout.  One such option
installs the executable as setgid so the high score and save files can be
stored in a centralized location for multiple users.  To enable that option,
pass ``-DSHARED_INSTALL=ON`` to CMake.  To specify the group used for the setgid
executable, pass ``-DINSTALL_GROUP_ID=xxx`` to CMake where you replace xxx with
the name or number of the group to use.  If you do not set the group, the games
group will be used.  Another option creates a read-only installation with any
variable state, including the high score and save files, stored on a per-user
basis in the user's own directories.  To enable that option, pass
``-DREADONLY_INSTALL=ON`` to CMake.  With either SHARED_INSTALL or
READONLY_INSTALL, you will need to run ``cmake --build build -t install`` after
the other steps for compiling with CMake.  As an example, this would build a
shared installation with an executable that is setgid for the games group::

    cmake -DSHARED_INSTALL=ON -DSUPPORT_GCU_FRONTEND=ON -B build
    cmake --build build
    sudo cmake --build build -t install

Turning on both SHARED_INSTALL and READONLY_INSTALL is not supported and will
cause CMake to exit with an error.  Turning either SHARED_INSTALL or
READONLY_INSTALL when SUPPORT_WINDOWS_FRONTEND is on is also not supported
and will cause CMake to exit with an error.  To customize where the shared
and read-only installations place files, pass -DCMAKE_INSTALL_PREFIX=prefix
to install all the files within the given prefix (i.e. using
``-DCMAKE_INSTALL_PREFIX=/opt/Angband-4.2`` would place all the files within
/opt/Angband-4.2 or its subdirectories).  For finer-grained placement of the
files within the given prefix, you could also set CMAKE_INSTALL_BINDIR
(for the subdirectory of prefix where the executable will be placed; by
default that is bin), CMAKE_INSTALL_DATAROOTDIR (for the subdirectory of
prefix to hold read-only data not configured for the site; by default that is
share), CMAKE_INSTALL_SYSCONFDIR (for the subdrectory of prefix to hold data
configured for the site; by default that is etc), and
CMAKE_INSTALL_SHAREDSTATEDIR (for the subdirectory of prefix to hold writable
persistent state; by default that is com).  Because paths to the data are
hardwired in the executable, setting the destination directory when performing
the build (i.e. by setting DESTDIR) is not supported and will not work in
general:  set the destination when configuring the build by setting the
variables mentioned above.

Debug build
~~~~~~~~~~~

To build the game for easier source-level debugging and with the address and
undefined behavior sanitizers included, use this when configuring the build
with CMake (of course, you can specify front ends and other options by putting
them before ``-B build``)::

    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF \
        -DCMAKE_C_FLAGS="-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer" \
        -B build

Test cases
~~~~~~~~~~

To compile and run the unit tests and run the run-tests script while using
CMake, do the following::

    cmake -DSUPPORT_TEST_FRONTEND=ON -B build
    cmake --build build -t alltests

If you only want the unit tests while using CMake, it is a little simpler::

    cmake -B build
    cmake --build build -t allunittests

To compile, as necessary, and run a single unit test, use the target,
run-unittest-*something* where *something*, is the name of the test in src/tests
with the leading ``src/tests`` and trailing ``.c`` dropped and any forward
slashes converted to hyphens.  So, if you have configured the build and only
want to run the tests in src/tests/object/pile.c, you can do that with::

    cmake --build build -t run-unittest-object-pile

There is some support for measuring how well the test cases cover the code.
If you have perl and either gcc and gcov or clang and llvm-cov, then you
can configure code coverage support by including ``-DSUPPORT_COVERAGE=ON``
in the options to CMake when configuring the build.  That adds three targets for
manipulating coverage results.  The ``reportcoverage`` target generates per-file
coverage reports (.gcov files in the directory where you are building) using
the current accumulated coverage data and then writes a summary of those
reports to standard output.  The gen-coverage Perl script in the src directory
is used to generate the summary.  The ``resetcoverage`` target removes the
accumulated coverage data (.gcda files) and any per-file coverage reports.
The ``coverage`` target is equivalent to building the ``resetcoverage``,
``alltests``, and ``reportcoverage`` targets in that order.  So if you wanted
to determine how well the unit tests cover the code, this would configure the
build and generate the necessary coverage reports::

    cmake -DSUPPORT_COVERAGE=ON -B build
    cmake --build build -t coverage

Statistics build
~~~~~~~~~~~~~~~~

To get the statistic front end and support for the debugging commands related
to statistics, include ``-DSUPPORT_STATS_FRONTEND=ON`` in the options to CMake
when configuring the build.  That requires the sqlite3 headers and libraries.
On Debian and Ubuntu, the libsqlite3-dev package and its dependencies provides
that.  If you only want the debugging commands related to statistics, include
``-DSUPPORT_STATS_BACKEND=ON`` in the options to CMake when configuring the
build and either do nothing for ``SUPPORT_STATS_FRONTEND`` or explicitly turn
it off by also including ``-DSUPPORT_STATS_FRONTEND=OFF`` in the options.

Linux / other UNIX with autotools
---------------------------------

Some sets of source code (e.g. downloads from Rephial.org) will contain a
"configure" script in the root directory of the unpacked files, while other
filesets (e.g. the "Source code (tar.gz)" links on the Releases pages or a
cloned git repository) will not contain the "configure" script.

If the code you download does not contain the "configure" script, then you
will first need to run the following command to create that script::

    ./autogen.sh

autogen.sh requires autoconf, autoheader, and aclocal to be installed.  On
Debian and Ubuntu, those are in the autoconf and automake packages.

To see a list of the useful options and variables that influence what
configure does, run::

    ./configure --help

To build Angband to be run in-place, run this::

    ./configure --with-no-install [other options as needed]
    make

That'll create an executable in the src directory.  You can run it from the
same directory where you ran make with::

    src/angband

To see what command line options are accepted, use::

    src/angband -?

Note that some of Angband's makefiles (src/Makefile and src/tests/Makefile are
the primary offenders) may assume features present in GNU make.  If the default
make on your system is not GNU make, you'll likely have to replace instances
of make in the quoted commands with whatever will run GNU make.  On OpenBSD,
for instance, that is gmake (which can be installed by running
``pkg_add gmake``).

On systems where there's several C compilers, ./configure may choose the
wrong one.  One example of that is on OpenBSD 6.9 when building Angband with
SDL2:  ./configure chooses gcc but the installed version of gcc can't handle
the SDL2 header files that are installed via pkg_add.  To override ./configure's
default selection of the compiler, use::

    ./configure [the appropriate configure options] CC=the_good_compiler

Replace the_good_compiler in that command with the command for running the
compiler that you want.  For OpenBSD 6.9 when compiling with SDL2, you'd
replace the_good_compiler with cc or clang.

To build Angband to be installed in some other location, run this::

    ./configure --prefix /path/to [other options as needed]
    make
    make install

On some BSDs, you may need to copy install-sh into lib/ and various
subdirectories of lib/ in order to install correctly.

Debug build
~~~~~~~~~~~

**WARNING** this build is intended primarily for debugging purposes. It might have a somewhat slower performance, higher memory requirements and panic saves don't always work (in case of a crash there is a higher chance of losing progress).

When debugging crashes it can be very useful to get more information about *what exactly* went wrong. There are many tools that can detect common issues and provide useful information. Two such tools that are best used together are AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan). To use them you'll need to enable them when compiling angband::

    ./configure [options]
    SANITIZE_FLAGS="-fsanitize=undefined -fsanitize=address" make

Note that compiling with these tools will require installing additional dependencies: libubsan libasan (names of the packages might be different in your distribution).

Test cases
~~~~~~~~~~

To compile and run the unit tests if you used ./configure --with-no-install,
do this::

    make tests

If you want to rerun just one part, say monster/attack, of the unit tests,
that's most easily done by directly running from the top-level directory::

    src/tests/monster/attack.exe

Older versions put the test executables in src/tests/bin.  With those
versions, the line above would be::

    src/tests/bin/monster/attack

There's a separate set of tests that use scripts to control a character in
the full game.  To run those tests, you'll need to enable the test module
when running configure and then run the run-tests script in the top-level
directory::

    ./configure --with-no-install --enable-test
    make
    ./run-tests

There is some support for measuring how well the test cases cover the code.
If you have gcc, gcov, and perl, you can run this in src directory after
running configure::

    make coverage

That cleans the directories (removing object files, intermediates generated
for code coverage, and coverage reports), rebuilds the game with code coverage
profiling enabled, runs the unit tests, generates coverage reports for
individual source files (.gcov files in the src directory), and then writes a
summary of those reports to standard output.  The gen-coverage Perl script in
the src directory is used to generate the summary.

Statistics build
~~~~~~~~~~~~~~~~

To get the statistics front end and support for the debugging commands related
to statistics (see the descriptions for ``S``, ``D``, and ``P`` in
:ref:`DebugDungeon`), include ``--enable-stats`` in the options to configure.
For that to work, you'll need to have sqlite3's headers and libraries
installed (on Debian and Ubuntu, the libsqlite3-dev package and its
dependencies provides those).

Windows native build
--------------------

Using MSYS2 (with MinGW64) and CMake
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Install the dependencies by::

    pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja

Additional dependency for the native Windows client is::

    pacman -S mingw-w64-x86_64-libpng

The additional dependency for ncurses is::

    pacman -S mingw-w64-x86_64-ncurses

Additional dependencies for the SDL2 client are::

    pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image \
        mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_mixer

Then run the following to compile for native Windows::

    cmake -G Ninja -DSUPPORT_WINDOWS_FRONTEND=ON \
        -DSUPPORT_STATIC_LINKING=ON \
        -B build
    cmake --build build

For ncurses, do::

    cmake -G Ninja -DSUPPORT_GCU_FRONTEND=ON \
        -DSUPPORT_STATIC_LINKING=ON \
        -B build
    cmake --build build

For SDL2, do::

    cmake -G Ninja -DSUPPORT_SDL2_FRONTEND=ON \
        -DSUPPORT_SDL2_SOUND=ON \
        -DSUPPORT_STATIC_LINKING=ON \
        -B build
    cmake --build build

Once built, go to build/game/ subdirectory and start angband by::

    cd build/game
    ./angband.exe

Using MinGW
~~~~~~~~~~~

This build uses autotools, so it is very similar to builds on Linux or Unix with
autotools.  Open the MinGW shell (MSYS) by running msys.bat.

If your source files are from rephial.org, from a "Source code" link on the
github releases page, or from cloning the git repository, you'll first need to
run this to create the configure script::

        ./autogen.sh

That is not necessary for source files that are from the github releases page
but not from a "Source code" link on that page.

Then run these commands::

        ./configure --enable-win
        make install

The last step only works with relative recent versions.  For older ones, use
"make" rather than "make install" and copy src/angband.exe,
src/win/dll/libpng12.dll, and src/win/dll/zlib1.dll to the top-level directory.

Using Cygwin with MinGW and CMake
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use this option if you want to build a native Windows executable that
can run with or without Cygwin.

Use the Cygwin setup.exe to install cmake, ninja, and mingw64-i686-gcc-core.
Build with::

    cmake -G Ninja \
        -DCMAKE_C_COMPILER=/usr/bin/i686-w64-mingw32-gcc \
        -DCMAKE_RC_COMPILER=/usr/bin/i686-w64-mingw32-windres \
        -DSUPPORT_WINDOWS_FRONTEND=ON \
        -DSUPPORT_BUNDLED_PNG=ON \
        -B build
    cmake --build build

Run with::

    cd build/game
    ./angband.exe

If you want to build the Unix version of Angband that uses X11 or
Curses and run it under Cygwin, then follow the Linux/Unix instructions.

Using Cygwin with MinGW and autotools
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use this option if you want to build a native Windows executable that
can run with or without Cygwin.

Use the Cygwin setup.exe to install autoconf, automake, make, and
mingw64-i686-gcc-core.  Build with::

    ./autogen.sh
    ./configure --enable-win --host=i686-w64-mingw32
    make install

And run::

    ./angband.exe

If you want to build the Unix version of Angband that uses X11 or
Curses and run it under Cygwin, then follow the Linux/Unix instructions.

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

You still need to run ./autogen.sh, if your source files are from a
"Source code" link on the github releases page or from cloning the
git repository, and ./configure manually, outside eclipse (see above)

Using Visual Studio
~~~~~~~~~~~~~~~~~~~

Blue Baron has detailed instructions for setting this up at:

    src/win/angband_visual_studio_step_by_step.txt

Debug build
~~~~~~~~~~~

To compile with the address and undefined behavior sanitizer on Windows,
similar to what's done for the debugging build on Linux, you need MSYS2 CLANG64
since other shells and compilers do not properly support those sanitizers
at the time this was written.

Run::

    C:/msys64/clang64.exe

Install dependencies and build with::

    pacman -S \
        mingw-w64-clang-x86_64-clang \
        mingw-w64-clang-x86_64-compiler-rt \
        mingw-w64-clang-x86_64-cmake \
        mingw-w64-clang-x86_64-ninja \
        mingw-w64-clang-x86_64-libpng \
        winpty
    cmake -G Ninja \
        -DCMAKE_C_FLAGS="-fsanitize=address -fsanitize=undefined" \
        -B build
    cmake --build build

Run the tests or the game from winpty because Windows won't printf to an MSYS2
terminal::

    winpty cmake --build build -t alltests
    cd build/game
    winpty ./angband.exe

We also still need the path from the MSYS2 shell so that it can find the
required DLLs (libclang_rt.asan_dynamic-x86_64.dll and libc++.dll), although
we can also copy those.

Statistics build
~~~~~~~~~~~~~~~~

The Windows front end bypasses main.c and can not use the statistics front end.
It is possible to enable the debugging commands related to statistics (see
the descriptions for ``S``, ``D``, and ``P`` in :ref:`DebugDungeon`).  With
CMake, you can do that by including ``-DSUPPORT_STATS_BACKEND=ON`` in the
options when configuring the build.  With other build tools, set the compiler
flags so tha the USE_STATS preprocessor macro is set.  With configure, that can
be done by including ``CFLAGS=-DUSE_STATS`` in the options to configure.

Cross-building for Windows with MinGW and CMake
-----------------------------------------------

Many developers (as well as the auto-builder) build Angband for Windows using
MinGW on Linux. This requires that the necessary MinGW packages are all
installed.  CMake's documentation for cross-compiling is available
`here <https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html>`__.
``toolchains/linux-i686-mingw32-cross.cmake`` is an example of a toolchain file
set up to build for Windows using MinGW as it is installed on recent versions
of Debian and Ubuntu (tool names prefixed with ``i686-w64-mingw32-`` and
relevant files placed within ``/usr/i686-w64-mingw32``).  If your installation
is not compatible with that, create a copy of that toolchain file, edit the
names or paths in it to match what your installation expects, and use that
copy instead of ``toolchains/linux-i686-mingw32-cross.cmake`` in the example
below.

To perform the build::

    cmake \
        -DCMAKE_TOOLCHAIN_FILE=toolchains/linux-i686-mingw32-cross.cmake \
        -DSUPPORT_BUNDLED_PNG=ON \
        -B build
    cmake --build build

That will leave an angband.exe and the needed .dll files in the sub directory
build/game/.  That executable can be run with wine::

    cd build/game
    wine angband.exe

To include support for the debugging statistics commands, include
``-DSUPPORT_STATS_BACKEND=ON`` in the options to CMake when configuring the
build.

If wine is installed and mentioned as the ``CMAKE_CROSSCOMPILING_EMULATOR`` in
the toolchain file, the unit test cases can be run from CMake with
``cmake --build build -t allunitests`` or for a particular unit test, something
like ``cmake --build build -t run-unittest-player-birth``.

Cross-building for Windows with MinGW and autotools
---------------------------------------------------

This type of build is very similar to a native build on Linux or Unix using
autotools.  The key difference is setting up to cross-compile and the use
of ``--enable-win`` when running configure.

If your source files are from rephial.org, from a "Source code" link on the
github releases page, or from cloning the git repository, you'll first need to
run this to create the configure script::

    ./autogen.sh

That is not necessary for source files that are from the github releases page
but not from a "Source code" link on that page.

Then configure the cross-compilation and perform the compilation itself::

    ./configure --enable-win --build=i686-pc-linux-gnu --host=i686-w64-mingw32
    make install

You may need to change the --build and --host options there to match your
system.  MinGW installs commands like ``i686-w64-mingw32-gcc``. The value of
--host should be that same command with the ``-gcc`` removed. Instead of i686
you may see i686, amd64, etc. The value of --build should be the host you are
building on (see http://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.68/html_node/Specifying-Target-Triplets.html#Specifying%20Names
for gory details of how these triplets are arrived at).  The ``make install``
step only works with relatively recent versions.  For older ones, use this
instead of the last step::

    make
    cp src/angband.exe .
    cp src/win/dll/*.dll .

To run the result, you can use wine like this::

    wine angband.exe

Since the Windows front end bypasses main.c, it is not possible to use the
statistics front end.  To compile in support for the debugging statistics
commands, set configure to include ``-DUSE_STATS`` in CFLAGS::

    ./configure --enable-win --build=i686-pc-linux-gnu --host=i686-w64-mingw32 \
        CFLAGS=-DUSE_STATS
    make install

The test cases can be built, with ``make tests``, but when that target tries
to run the test cases, that will fail as src/tests/run-tests is not set up to
run the individual test cases in an emulator like wine.  You can run those
test cases manually.  For instance, this::

    wine src/tests/game/mage.exe

would run the tests from src/tests/game/mage.c or this::

    for t in src/tests/*/*.exe ; do
        wine $t
    done

would run all of the unit tests.

TODO: except for recent versions (after Angband 4.2.3) you likely need to
manually disable curses (add --disable-curses to the options passed to
configure), or the host curses installation will be found causing the build
process to fail when linking angband.exe (the error message will likely be
"cannot find -lncursesw" and "cannot find -ltinfo").  Most of the --with or
--enable options for configure are not appropriate when using --enable-win.
The ones that are okay are --with-private-dirs (on by default),
--with-gamedata-in-lib (has no effect), and --enable-release.

macOS
-----

To build the new Cocoa front-end::

    cd src
    make -f Makefile.osx

That'll create a self-contained Mac application, Angband.app, in the directory
above src.  You may use that application where it is or move it to wherever
is convenient for you.

By default, the current Makefile.osx builds an application that'll run natively
on x86_64 or arm64 machines.  If only one of those architectures is of interest
to you or the version of Xcode you have doesn't support building both (a
typical error message in that case is something like ``/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/sys/cdefs.h:784:2: error: Unsupported architecture``),
you can change the architectures built by setting ARCHS on the command line of
make.  To only build for x86_64, for instance, you would use::

    cd src
    make -f Makefile.osx clean
    make -f Makefile.osx ARCHS=x86_64

(the clean step is to ensure that nothing from a previous build would cause
trouble; you'll typically need to do that if you've built it before and then
want to change the set of architectures to use).  To build for multiple
architectures, use a list of architectures separated by whitespace, which
you'll have to quote.  This is the equivalent to what Makefile.osx does by
default::

    make -f Makefile.osx ARCHS="x86_64 arm64"

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
that's most easily done from the top directory of the source distribution::

    src/tests/monster/attack.exe

Somewhat older versions configure the test cases so they should be run
from src/tests.  For those you would either use::

    cd src/tests
    monster/attack.exe

or, for the even older versions that put the test executables in src/tests/bin,
use::

    cd src/tests
    bin/monster/attack

Statistics build
~~~~~~~~~~~~~~~~

The Mac front end bypasses main.c and can not use the statistics front end.
It is possible to enable the debugging commands related to statistics (see
the descriptions for ``S``, ``D``, and ``P`` in :ref:`DebugDungeon`).  To do so,
include ``-DUSE_STATS`` in the setting for OPT passed to make.  The equivalent
of the standard build with those debugging commands enabled would be::

    cd src
    make -f Makefile.osx OPT="-DUSE_STATS -O2"

If you had already built everything without statistics enabled, you would need
to run either "rm wiz-stats.o" or "make -f Makefile.osx clean" immediately
after running "cd src".

Nintendo DS / Nintendo 3DS
--------------------------

Builds for the Nintendo DS are made using devkitARM and libnds (or libctru for
the Nintendo 3DS respectively). All required dependencies can be installed by
selecting the appropriate package group while following the installation
instructions for devkitPro ( https://devkitpro.org/wiki/Getting_Started ).

The executable can then be built using::

        cd src
        make -f Makefile.nds

This will generate ``angband.nds`` in the current directory. For the Nintendo
3DS, replace the ``Makefile.nds`` part of the command with ``Makefile.3ds``,
and ``angband.3dsx`` will be generated instead.

Debugging
~~~~~~~~~

Homebrew can be debugged using a gdbstub-enabled emulator, such as a Windows Dev+ build
of DeSmuMe (if you really dare to, note that it is very slow compared to real hardware)
for the Nintendo DS or Citra for the Nintendo 3DS. A Nintendo 3DS that has been modified
with custom firmware (such as Luma3DS) may also have the ability to debug homebrew on-device.

It is recommended to set/export ``NDS_DEBUG=1`` and to do a clean build when debugging,
as this disables some optimization and enables more debugging information.

Once the GDB server has been set up (and the host and port noted), the GDB client
can be loaded with the executable information::

        /path/to/devkitARM/bin/arm-none-eabi-gdb angband.elf

The ``angband.elf`` file is a byproduct from the build process, and it has to match
the executable that is currently running in the emulator or on the device.
It is always named ``angband.elf`` for the Nintendo 3DS, and it's always either
``angband.arm7.elf`` or ``angband.arm9.elf`` for the Nintendo DS, depending on
which processor should be debugged (as the main game runs on the ARM9 core exclusively,
this will almost always be the core that should be debugged).

Once the GDB command prompt is available, the following command can be used to
connect to the target device::

        target remote <host>:<port>

Afterwards, the debugging target will pause automatically and it can be debugged as usual
using GDB.

Cross-building for DOS with DJGPP
---------------------------------
These instructions were written using a Slackware64-15.0 host.

Install the following cross-compiler,
https://github.com/andrewwutw/build-djgpp , by running::

    git clone https://github.com/andrewwutw/build-djgpp.git
    cd build-djgpp
    DJGPP_PREFIX=$HOME/local/cross-djgpp ./build-djgpp.sh 10.3.0

Then build angband using the cross-compiler::

    cd angband/src
    PATH=$PATH:$HOME/local/cross-djgpp/bin
    make -f Makefile.ibm

Optionally build the documentation (requires Sphinx)::

    make -f Makefile.ibm docs

To create the angband.zip distribution, run::

    make -f Makefile.ibm dist

User Documentation
------------------

To convert the user manual from restructured text to the desired output
format, you'll need Sphinx ( https://www.sphinx-doc.org/en/master/ )
and, unless you change the theme in the documentation configuration, the
sphinx-better-theme ( https://pypi.org/project/sphinx-better-theme/ ; which
can be installed via pip using::

    pip install sphinx-better-theme

).

With CMake
~~~~~~~~~~

To build the documentation with CMake, ensure the prerequisites, described
above, are installed and include ``-DBUILD_DOC=ON`` in the options to CMake
when configuring the build.  If you want to override the default theme,
specify the theme's name in the DOC_HTML_THEME variable.  For instance, running
this at the top level of the distribution::

    cmake -DBUILD_DOC=ON -DDOC_HTML_THEME=alabaster -B build
    cmake --build build -t OurManual

would configure the build to include the documentation, using Sphinx's builtin
alabaster theme, and then build the documentation.  The generated user manual
will be in the subdirectory, ``manual-output/html``, in the build directory.
So with the example commmands above, the user manual is in
``build/manual-output/html``.

With autotools
~~~~~~~~~~~~~~

To build the documentation with autotools, ensure the prerequisites, described
above, are installed and include ``--with-sphinx`` in the options to configure.
If you want to override the default theme, specify the theme's name in the
DOC_HTML_THEME variable.  For instance, running this at the top level of the
distribution::

    ./configure --with-no-install --with-sphinx DOC_HTML_THEME=alabaster
    make manual

would configure the build to include the documentation, using Sphinx's builtin
alabaster theme, and then build the documentation.  The generated user manual
will be in docs/_build/html.

Without CMake and autotools
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To build the user manual without configure or CMake, make sure sphinx-build
is in your path, change directories to the docs subdirectory of the top-level
directory in the source files, and then run::

    make html

That will generate a _build/html directory with the result of the conversion;
_build/html/index.html is the top-level help with links to everything else.

Other output formats besides HTML are possible.  Run::

    make

without any arguments in the docs subdirectory to see the formats that Sphinx
can generate.

Developer Documentation
-----------------------

To extract documentation from comments in the source code, you will need
doxygen, https://www.doxygen.nl .  Then you can run this in the top level
directory of the distribution::

        doxygen src/doc/doxygen.conf

to assemble the documentation and place it in src/doc/_doxygen .
