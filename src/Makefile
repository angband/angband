# File: Makefile

# This is not a very "nice" Makefile, but it usually works.

#
# Note that you may have to make modifications below according
# to your machine, available libraries, compilation options,
# and your "visual module" of choice.  This Makefile is intended
# for use with Unix machines running X11, Curses, Ncurses, or Vt100,
# or possibly for "Atari" or "Amiga" computers with "Curses" ports,
# see below for more information.
#
# Note that "main-mac.c", the visual module for the Macintosh,
# must be compiled in a special way, see elsewhere.
#
# Note that "main-win.c", the visual module for Windows,
# must be compiled in a special way, see elsewhere.
#
# Note that "main-ibm.c", "main-emx.c", and "main-286.c",
# the visual modules for various types of IBM-PC computers,
# must be compiled with special Makefiles, see elsewhere.
#
# Note that "main-lsl.c", the visual module for ???,
# must be compiled with "Makefile.lsl", see elsewhere.
#
# Note that "main-acn.c", the visual module for Risc Acorn,
# must be compiled with "Makefile.acn", see elsewhere.
#
# Note that "Makefile.w31" and "Makefile.wat" are slight variations
# on "Makefile.win" and "Makefile.ibm", respectively, which allow
# the use of the "main-win.c" and "main-ibm.c" files, respectively,
# with special compilers.
#
# If you are able to construct "main-xxx.c" and/or "Makefile.xxx"
# files for a currently unsupported system, please send them to me
# (benh@voicenet.com) for inclusion in future versions.
#
# This Makefile comes with "default" dependancies that may be obsolete.
#


#
# Here are the basic "source" and "object" files.  You may remove
# the "borg*" and "wiz*" files unless you are compiling with the
# "ALLOW_BORG" or "ALLOW_WIZARD" or "ALLOW_SPOILERS" options.
#

SRCS = \
  main.c main-cap.c main-gcu.c main-x11.c main-xaw.c \
  signals.c util.c io.c init.c save.c save-old.c files.c \
  generate.c birth.c melee.c dungeon.c store.c \
  effects.c cmd1.c cmd2.c cmd3.c cmd4.c cmd5.c cmd6.c \
  misc.c monster.c mon-desc.c object.c obj-desc.c \
  spells1.c spells2.c cave.c tables.c variable.c \
  term.c z-util.c z-virt.c z-form.c \
  borg-ben.c borg-aux.c borg-ext.c borg-obj.c borg-map.c borg.c \
  wizard.c wiz-spo.c

OBJS = \
  main.o main-cap.o main-gcu.o main-x11.o main-xaw.o \
  signals.o util.o io.o init.o save.o save-old.o files.o \
  generate.o birth.o melee.o dungeon.o store.o \
  effects.o cmd1.o cmd2.o cmd3.o cmd4.o cmd5.o cmd6.o \
  misc.o monster.o mon-desc.o object.o obj-desc.o \
  spells1.o spells2.o cave.o tables.o variable.o \
  term.o z-util.o z-virt.o z-form.o \
  borg-ben.o borg-aux.o borg-ext.o borg-obj.o borg-map.o borg.o \
  wizard.o wiz-spo.o


##
## Following are some "system" definitions
##
## No changes are needed to compile a version that will run on both
## X11 and Curses, in debugging mode, with maximal warnings, on many
## normal Unix machines of the Sun OS variety (non-solaris).
##
## To use an "alternative" definition, simply "modify" (or "replace")
## the definition below with one that you like.  For example, you can
## change the compiler to "cc", or remove the "debugging" options, or
## remove the X11 or Curses support, etc, as desired.
##
## See also "config.h" and "h-config.h" for important information.
##
## Some "examples" are given below, they can be used by simply
## removing the FIRST column of "#" signs from the "block" of lines
## you wish to use, and commenting out "standard" block below.
##
## This is not intended to be a "good" Makefile, just a "simple" one.
##


#
# This is my compiler of choice, it seems to work most everywhere
#
CC = gcc


#
# Standard version (see main-x11.c and main-gcu.c)
#
# This version supports both "X11" and "curses" in a single executable.
#
# You may have to add various X11 include/library directories to the
# "CFLAGS", if your machine places files in a weird location.
#
# You may be able to remove "-ltermcap" on some machines (ex: Solaris).
#
# You may have to replace "-lcurses" with "-lncurses" to use the
# "new curses" library instead of the "old curses" library, and
# you may have to add "-l/usr/include/ncurses" to the "CFLAGS".
#
# See "main-gcu.c" and "config.h" for some optional "curses" defines,
# including "USE_GETCH" and "USE_CURS_SET".  Note that "config.h" will
# attempt to "guess" at many of these flags based on your system.
#
CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_GCU"
LIBS = -lX11 -lcurses -ltermcap


##
## Variation -- Only support "main-x11.c" (not "main-gcu.c")
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11"
#LIBS = -lX11


##
## Variation -- Only support "main-gcu.c" (not "main-x11.c")
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_GCU"
#LIBS = -lcurses -ltermcap


##
## Variation -- Use "main-xaw.c" instead of "main-x11.c"
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_XAW" -D"USE_GCU"
#LIBS = -lXaw -lXmu -lXt -lX11 -lcurses -ltermcap


##
## Variation -- Use "main-cap.c" instead of "main-gcu.c"
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_CAP"
#LIBS = -lX11 -ltermcap


##
## Variation -- Only work on simple vt100 terminals
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_CAP" -D"USE_HARDCODE"


##
## Variation -- this might work for Linux 1.1.93 using ncurses-1.8.7.
##
#CFLAGS = -I/usr/X11R6/include -I/usr/include/ncurses \
#         -Wall -O2 -fomit-frame-pointer -m486 \
#         -D"USE_X11" -D"USE_GCU"
#LIBS = -L/usr/X11R6/lib -lX11 -lncurses -ltermcap

##
## Variation -- this might work better than the suggestion above
##
#CFLAGS = -I/usr/include/ncurses \
#         -Wall -O2 -fomit-frame-pointer \
#         -D"USE_X11" -D"USE_GCU" \
#         -D"USE_TPOSIX" -D"USE_CURS_SET"
#LIBS = -lX11 -lncurses
#LDFLAGS = -s


##
## Variation -- compile for Solaris
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_GCU" -D"SOLARIS"
#LIBS = -lX11 -lsocket -lcurses


##
## Variation -- compile for SGI Indigo runnig Irix
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_GCU" -D"SGI"
#LIBS = -lX11 -lcurses -ltermcap -lsun



##
## Variation -- compile for Dec ALPHA OSF/1 v2.0
##
#CC     = cc
##CFLAGS = -std -O -g3 -Olimit 4000 -D"USE_X11" -D"USE_GCU"
#CFLAGS = -std -g -D"USE_X11" -D"USE_GCU"
#LIBS   = -lX11 -lcurses -ltermcap -lrpcsvc


##
## Variation -- compile for Interactive Unix (ISC) systems
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_GCU" -D"ISC"
#LIBS = -lX11 -lcurses -lnsl_s -linet -lcposix


##
## Variation -- Support fat binaries under NEXTSTEP
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_GCU" -arch m68k -arch i386
#LIBS = -lcurses -ltermcap



#
# Hack -- "install" as the base target
#

install: angband
	cp angband ..


#
# Build the "Angband" program
#

angband: $(OBJS)
	$(CC) $(CFLAGS) -o angband $(OBJS) $(LDFLAGS) $(LIBS)


#
# Clean up old junk
#

clean:
	\rm *.bak *.o


#
# Generate dependancies automatically
#

depend:
	makedepend -D__MAKEDEPEND__ $(SRCS)


#
# Basic header files
#

#h-basic.h: h-define.h h-type.h h-system.h h-config.h

#angband.h: config.h defines.h types.h externs.h \
#angband.h: term.h z-util.h z-virt.h z-form.h h-basic.h


# DO NOT DELETE THIS LINE -- make depend depends on it.

birth.o: birth.c angband.h
borg-aux.o: borg-aux.c angband.h borg-aux.h borg-ext.h
borg-aux.o: borg-obj.h borg-map.h borg.h
borg-ben.o: borg-ben.c angband.h borg-ben.h borg-aux.h borg-ext.h
borg-ben.o: borg-obj.h borg-map.h borg.h
borg-ext.o: borg-ext.c angband.h borg-ext.h
borg-ext.o: borg-obj.h borg-map.h borg.h
borg-obj.o: borg-obj.c angband.h borg-obj.h borg.h
borg-map.o: borg-map.c angband.h borg-map.h borg.h
borg.o: borg.c angband.h borg.h
cave.o: cave.c angband.h
cmd1.o: cmd1.c angband.h
cmd2.o: cmd2.c angband.h
cmd3.o: cmd3.c angband.h
cmd4.o: cmd4.c angband.h
cmd5.o: cmd5.c angband.h
cmd6.o: cmd6.c angband.h
dungeon.o: dungeon.c angband.h
effects.o: effects.c angband.h
files.o: files.c angband.h
generate.o: generate.c angband.h
init.o: init.c angband.h
io.o: io.c angband.h
main-cap.o: main-cap.c angband.h
main-gcu.o: main-gcu.c angband.h
main-x11.o: main-x11.c angband.h
main-xaw.o: main-xaw.c angband.h
main.o: main.c angband.h
melee.o: melee.c angband.h
misc.o: misc.c angband.h
mon-desc.o: mon-desc.c angband.h
monster.o: monster.c angband.h
obj-desc.o: obj-desc.c angband.h
object.o: object.c angband.h
save-old.o: save-old.c angband.h
save.o: save.c angband.h
signals.o: signals.c angband.h
spells1.o: spells1.c angband.h
spells2.o: spells2.c angband.h
store.o: store.c angband.h
tables.o: tables.c angband.h
term.o: term.c h-basic.h term.h z-virt.h
util.o: util.c angband.h
variable.o: variable.c angband.h
wiz-spo.o: wiz-spo.c angband.h
wizard.o: wizard.c angband.h
z-form.o: z-form.c h-basic.h z-form.h z-util.h z-virt.h
z-util.o: z-util.c h-basic.h z-util.h
z-virt.o: z-virt.c h-basic.h z-virt.h z-util.h

