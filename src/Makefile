# File: Makefile

# This is not a very "nice" Makefile, but it usually works.

##
## Note that you may have to make modifications below according
## to your machine, available libraries, compilation options,
## and your "visual module" of choice.  See below...
##
## Note that "main-ibm.c", "main-emx.c", "main-wat.c", and
## "main-win.c", all visual module files for IBM-ish computers,
## have their own Makefiles, see "Makefile.xxx".
##
## Note that "main-mac.c", the visual module for the Macintosh,
## requires a special compiler-specific "project" file instead
## of a Makefile.
##

SRCS = \
  main.c main-gcu.c main-ncu.c main-cur.c main-x11.c main-xaw.c \
  signals.c util.c io.c init.c save.c save-old.c files.c \
  generate.c birth.c melee.c dungeon.c store.c \
  effects.c cmd1.c cmd2.c cmd3.c cmd4.c cmd5.c cmd6.c \
  misc.c monster.c mon-desc.c object.c obj-desc.c \
  spells1.c spells2.c cave.c tables.c variable.c \
  term.c random.c z-util.c z-virt.c z-form.c

OBJS = \
  main.o main-gcu.o main-ncu.o main-cur.o main-x11.o main-xaw.o \
  signals.o util.o io.o init.o save.o save-old.o files.o \
  generate.o birth.o melee.o dungeon.o store.o \
  effects.o cmd1.o cmd2.o cmd3.o cmd4.o cmd5.o cmd6.o \
  misc.o monster.o mon-desc.o object.o obj-desc.o \
  spells1.o spells2.o cave.o tables.o variable.o \
  term.o random.o z-util.o z-virt.o z-form.o




###
### Following are some "system" definitions
###
### No changes are needed to compile a version that will run on both
### X11 and Curses, in debugging mode, with maximal warnings.
###
### To use an "alternative" definition, simply "modify" (or "replace")
### the definition below with one that you like.  For example, you can
### change the compiler to "cc", or remove the "debugging" options, or
### remove the X11 or Curses support, etc, as desired.
###
### See also "config.h" and "h-config.h".
###
### Some "examples" are given below, they can be used by simply
### removing the FIRST column of "#" signs from the "block" of lines
### you wish to use.  Remember to comment out the "standard" stuff first!
###


#
# This is my compiler of choice, it seems to work most everywhere
#
CC = gcc


#
# Standard version (see main-x11.c and xtra-x11.c and main-gcu.c)
#
# This version supports both X11 and "curses" in a single executable
#
# Note that "-ltermcap" is not needed for some machines (ex: Solaris)
#
# You may be able to define "USE_GETCH" or "USE_CURS_SET", see "main-gcu.c"
#
CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_GCU"
LIBS = -lX11 -lcurses -ltermcap


##
## Pure "X11" version (see main-x11.c and xtra-x11.c)
##
## You may have to add various X11 include/library directories to the
## "CFLAGS" line, this will vary according to machine.
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11"
#LIBS = -lX11


##
## Pure "Curses" version (see main-gcu.c)
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_GCU"
#LIBS = -lcurses -ltermcap


##
## Use the "Athena Widget Set" and "main-xaw.c" instead of "main-x11.c"
## But otherwise work the same as the "standard" version (above)
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_XAW" -D"USE_GCU"
#LIBS = -lXaw -lXmu -lXt -lX11 -lcurses -ltermcap


##
## Standard version (see main-x11.c and xtra-x11.c and main-gcu.c)
## This one seems to work for Linux 1.1.93 using ncurses-1.8.7.
##
## This version supports both X11 and "curses" in a single executable
##
## You may be able to define "USE_GETCH" or "USE_CURS_SET", see "main-gcu.c"
## I have heard a rumor that "USE_GETCH" is not a good idea on Linux.
##
#CFLAGS = -I/usr/X11R6/include -I/usr/include/ncurses \
#         -Wall -O2 -fomit-frame-pointer -m486 -D"USE_X11" -D"USE_GCU"
#LIBS = -L/usr/X11R6/lib -lX11 -lncurses -ltermcap

##
## Here is an alternate version of the code just above me
## I cannot vouch for either one, try both and see what happens.
## Note that the third line of defines tells "main-gcu.c" to
## use the "best" options for a standard Linux machine.
##
#CFLAGS = -O2 -fomit-frame-pointer -I/usr/include/ncurses -Wall \
#         -D"USE_X11" -D"USE_GCU" \
#         -D"USE_TPOSIX" -D"USE_CURS_SET"
#LIBS = -lX11 -lncurses
#LDFLAGS = -s


##
## Solaris with X11 and curses
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_GCU" -D"SOLARIS"
#LIBS = -lX11 -lsocket -lcurses


##
## SGI Indigo runnig Irix
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_GCU" -D"SGI"
#LIBS = -lX11 -lcurses -ltermcap -lsun



##
## Dec ALPHA OSF/1 v2.0
##
#CC     = cc
##CFLAGS = -std -O -g3 -Olimit 4000 -D"USE_X11" -D"USE_GCU"
#CFLAGS = -std -g -D"USE_X11" -D"USE_GCU"
#LIBS   = -lX11 -lcurses -ltermcap -lrpcsvc


##
## Interactive Unix (ISC) systems
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_GCU" -D"ISC"
#LIBS = -lX11 -lcurses -lnsl_s -linet -lcposix


##
## Standard version, but use "ncurses" instead of "curses"
##
## You may be able to replace "USE_NCU" with "USE_GCU" below.
##
## This version supports both X11 and Ncurses in a single executable
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_X11" -D"USE_NCU" -I/usr/include/ncurses
#LIBS = -lX11 -lncurses


##
## Pure "ncurses" version (see above)
##
#CFLAGS = -Wall -O1 -pipe -g -D"USE_NCU" -I/usr/include/ncurses
#LIBS = -lncurses



#
# Build the "Angband" program and move it up one directory
#

angband: $(OBJS)
	$(CC) $(CFLAGS)  -o angband $(OBJS) $(LIBS) $(LDFLAGS)
	mv angband ..

#
# Generate dependancies automatically (not required)
#

depend:
	makedepend -D__MAKEDEPEND__ $(SRCS)



# DO NOT DELETE THIS LINE -- make depend depends on it.

