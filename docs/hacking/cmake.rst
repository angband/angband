===========
CMake Hints
===========

To keep things simpler, :doc:`compiling` uses CMake in a standardized way:
``cmake [options] -B build`` to configure the build and
``cmake --build build [options]`` to perform the build.  Both are run in the
top-level directory of the sources which is where CMakeLists.txt is.  Here, we
will provide hints about ways you can use CMake to match how you want to build
the game.

CMake's command line syntax is documented at `cmake(1) <https://cmake.org/cmake/help/latest/manual/cmake.1.html>`_.
CMake does have interactive interfaces which are not used here or in
:doc:`compiling`.  If one of those is more to your liking, information about
them are at `ccmake(1) <https://cmake.org/cmake/help/latest/manual/ccmake.1.html>`_
and `cmake-gui(1) <https://cmake.org/cmake/help/latest/manual/cmake-gui.1.html>`_.
For information about using CMake and Visual Studio, see
`Microsoft's introduction <https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170>`_.

Configuring the Build
=====================

Setting the Build and Source Directories
----------------------------------------

Since CMake 3.13, the location of the build directories and the source
directories can be specified with the ``-B`` and ``-S`` options when
configuring the build: ``cmake [options] -B path-to-build [-S path-to-source]``.
In :doc:`compiling`, CMake is run where the game's CMakeLists.txt is located
and the build's products are placed in the build subdirectory, so the
configuration step looks like ``cmake [options] -B build``.  If you want to
run CMake in a different location or place the build products in a different
location, feel free to make use the full range of what the ``-B`` and ``-S``
options allow.

An alternate way to perform the configuration step, supported by all versions
of CMake, is to run the configuration step in the top-level directory of where
the build's products will be placed and specify the path to the directory
containing the relevant CMakeLists.txt file as the last argument:
``cmake [options] path-to-source``.  So on a system that looks like Unix, these
commands function similarly to the ``cmake [options] -B build`` used in
:doc:`compiling`::

    mkdir build && cd build
    cmake [options] ..

Choosing the Underlying Build Tool
----------------------------------

CMake supports a variety of native build tools:  make, ninja, ....  In
CMake's parlance, one selects a "generator" when configuring the build, and
CMake uses that to set up the build directories for a native build tool.  In
:doc:`compiling`, most examples used the default generator.  Running
``cmake --help`` will display the generators that are available and which is
the default.

One reason to choose a generator other than the default is performance.  On
systems where the ``Unix Makefiles`` generator is the default, you might prefer
to use the ``Ninja`` generator because ninja is faster than make.  Another
reason is that the native build tool corresponding to the default generator
is not installed but you do have access to another build tool supported by
another generator.

When configuring the build, you can specify the generator by with
``-G name-of-generator``.  In :doc:`compiling`, that was used in some places
to use the generator for ninja::

    cmake -G Ninja -B build
    cmake --build build

On Windows if you prefer to use Visual Studio rather than something
like MSYS2 or Cygwin, you might install CMake and then use something like
this from Cmd.exe's command line to build the game and build and run the
unit tests from the top-level directory of the game's source files::

    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
    cmake -G "Visual Studio 17 2022" -A Win32 -DSUPPORT_WINDOWS_FRONTEND=ON -DSUPPORT_BUNDLED_PNG=ON -B build
    cmake --build build 
    cmake --build build -t allunittests

Of course, you would have to adjust the path to vcvars32.bat and, perhaps, the
name of the generator to be compatible with the version of Visual Studio you
have installed.

Adjusting the Configuration with Environment Variables
------------------------------------------------------

CMake's documentation on environment variables that influence how a build is
configured is at `cmake-env-variables(7) <https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html#manual:cmake-env-variables(7)>`_.
The environment variables likely to be of use when configuring the build for the
game are ``CMAKE_BUILD_TYPE``, ``CMAKE_GENERATOR``, ``CC`` (the C compiler to
use), ``CFLAGS`` (options to pass to the C compiler), ``LDFLAGS`` (options to
pass when linking), ``RC`` (the compiler for resource files; only relevant if
building the Windows front end), and ``RCFLAGS`` (options to pass when
compiling resource files; only relevant if building the Windows front end).
In :doc:`compiling`, the example commands do not use environment variables but
specify everything with command line options (``-G name`` to set the generator,
``-DCMAKE_BUILD_TYPE=...`` to set the build type, and ``-DCMAKE_C_FLAGS=...``
to set options for the C compiler).

Determining What Variables To Set
---------------------------------

Running ``cmake --help-variable-list`` will list CMake's variables which have
documentation and running ``cmake --help-variable x`` will display the
documentation for the variable x.  Those do not provide any information about
the variables specific to the game's CMakeLists.txt.  For those, running
``cmake -N -LH path-to-build`` (i.e. running ``cmake -N -LH  .`` in the build
directory) after you have already configured a build will display a brief
description and current setting for those variables.

Debugging the Configuration Process
-----------------------------------

In some cases, CMake compiles or runs stuff during configuration to test what
works.  When such a test fails and it is unclear from CMake's output what
exactly went wrong, rerunning the configuration step with ``--debug-trycompile``
in the options will leave the temporary directories CMake creates for the tests
in place.  Then it is possible to inspect the commands that were involved the
test and rerun them outside of CMake to better determine what went wrong.

Performing the Build
====================

Using the Native Build Tool Rather Than CMake
---------------------------------------------

The instructions in :doc:`compiling` use ``cmake --build path-to-build [options]``
to perform the build.  You could also use native build tool directly.  For
instance, rather than ``cmake --build ./build -t allunittests``, you could
change directories to ``./build`` and run ``make allunittests`` if make is the
native build tool or run ``ninja allunittests`` if ninja is the native build
tool.

Parallel Builds
---------------

To speed up the build by running independent portions simultaneously, use
``cmake --build path-to-build -j n [options]``

where n is the maximum number of jobs to run in parallel.  That requires CMake
3.12 or later.  When you know the native build tools option for parallel builds,
you can use that instead.  With GNU Make, use ``make -j n [options]``.  With
ninja, use ``ninja -j n [options]``.  Rather than specifying a fixed number
for n, it can be useful to get the number of processor available on the
machine.  For instance, on Linux use::

    -j $(nproc)

On macOS use::

    -j $(sysctl -n hw.activecpu)

On PowerShell use::

    -j ${env:NUMBER_OF_PROCESSORS}

Useful Targets
--------------

Targets that are likely to be useful with the ``-t`` option to
``cmake --build`` or as an argument to make or ninja:

all
    This is the default target, used when no other targets are specified.  It
    will compile the game and, if BUILD_DOC was turned on when the build was
    configured, the user manual.  This target is common to all CMake projects.

alltests
    Performs all the actions of the allunittests target.  If not cross-compiling
    and the test front end has been configured, builds the game, if necessary,
    and runs tests/run-test from the source directory.  This target is specific
    to the game's CMakeLists.txt.

allunittests
    If not cross-compiling or an emulator is available, builds, if necessary,
    all of the unit tests from src/tests in the source directory and then
    runs all of those tests.  This target is specific to the game's
    CMakeLists.txt.

clean
    Removes build intermediates.   Another way to do that is by running,
    ``cmake --build path-to-build --clean-first [options]`` when you also want
    to build something immediately after cleaning up the intermediates.  This
    target is common to all CMake projects.

coverage
    Reports how well the tests performed by the ``alltests`` target cover the
    code.  Peforms the actions of the ``resetcoverage`` target to remove any
    prior coverage information.  Then performs the actions of the ``alltests``
    target.  Finally, generates the coverage reports by performing the actions
    of the ``reportcoverage`` target.  This target is specific to the game's
    CMakeLists.txt and is only available if SUPPORT_COVERAGE was turned on when
    the build was configured.

install
    Builds the game, if necessary, and installs it at the location set when
    build was configured.  Useful if the build was configured with
    SHARED_INSTALL or READONLY_INSTALL turned on.  For builds configured
    with SC_INSTALL turned on (which is the default), there is an alternative
    that makes it easier to control where the game is installed:  build without
    any target specified and run
    ``cmake --install path-to-build-directory --prefix desired-installation-location``.
    This target is common to all CMake projects.

OurManual
    Builds the user manual.  This target is specific to the game's
    CMakeLists.txt and is only available if BUILD_DOC was turned on when the
    build was configured.

print-executable-name
    Writes ``our-executable-name=x`` to standard output where x is the file
    name (no directory information) of the game's executable.  This target is
    specific to the game's CMakeLists.txt.

print-project-name
    Writes ``our-project-name=x`` to standard output where x is what is
    set as the project's name by calling project() in CMakeLists.txt.  This
    target is specific to the game's CMakeLists.txt.

reportcoverage
    Using the current accumulated coverage data, generate a coverage report
    for each source file (each such report has a .gcov extension) and a summary,
    generated by src/gen-coverage in the source directories, across all source
    files.  That summary is written to standard output.  This target is
    specific to the game's CMakeLists.txt and is only available if
    SUPPORT_COVERAGE was turned on when the build was configured.

resetcoverage
    Clears any accumulated coverage data and any prior coverage report for each
    source file.  This target is specific to the game's CMakeLists.txt and is
    only available if SUPPORT_COVERAGE was turned on when the build was
    configured.

run-unittest-*x*
    Builds, if necessary, a specific unit test and runs that test (if
    cross-compiling and an emulator is not available, running will not work).
    *x* is the name of the unit test's source file within the source directory
    with the leading ``src/tests`` and trailing ``.c`` removed, and each ``/``
    converted to a ``-``.  For instance, the target ``run-unittest-parse-parse``
    would build and run the tests in src/tests/parse/parse.c.  This target is
    specific to the game's CMakeLists.txt.

Changing Configurations
=======================

You can continue to reuse a build directory as you make changes to the source
code, including adding, moving, or removing files, and then rebuild.  However,
if you want to change the configuration of the build, you will need to
configure a new build.  That could be done in a different directory.  If you
want to use a build directory with the same name as what you were using, you
could use the ``--fresh`` option to CMake when reconfiguring (requires CMake
3.24 or later) or remove that directory and all of its contents
(i.e.  ``rm -rf path-to-build`` on Unix) and have the configuration step
regenerate that directory.

End Products
============

The game's CMakeLists.txt puts the game's executable and the lib directory it
depends on in the subdirectory, ``game``, of the build directory.  That avoids
trouble on MSYS2 (the game's DLLs interfere with the MinGW compiler) and
separates the game from the other intermediates generated during the build.

The compiled user's manual for the game will be in the subdirectory,
``manual-output/html``, of the build directory.
