# File: Makefile

# See "Makefile" for more info.  This file should be the same except for
# the definition of "USE_X11", the inclusion of some extra files, and the
# addition of some libraries.  You may have to add X11 include/library
# directories to the various places they can be added.  This Makefile
# should generate an executable usable on both X11 and Curses.

SRCS = \
  arrays.c birth.c cave.c command.c \
  creature.c death.c desc.c dungeon.c \
  effects.c files.c generate.c help.c io.c magic.c \
  main.c main-mac.c main-x11.c main-cur.c \
  misc1.c misc2.c misc3.c misc4.c \
  moria1.c moria2.c moria3.c moria4.c \
  recall.c save.c signals.c \
  spells1.c spells2.c store.c tables.c term.c \
  util.c variable.c wizard.c \
  z-util.c z-virt.c z-form.c \
  x-metadpy.c x-infowin.c x-infoclr.c x-infofnt.c \
  r-metadpy.c r-infowin.c r-infoclr.c r-infofnt.c

OBJS = \
  arrays.o birth.o cave.o command.o \
  creature.o death.o desc.o dungeon.o \
  effects.o files.o generate.o help.o io.o magic.o \
  main.o main-mac.o main-x11.o main-cur.o \
  misc1.o misc2.o misc3.o misc4.o \
  moria1.o moria2.o moria3.o moria4.o \
  recall.o save.o signals.o \
  spells1.o spells2.o store.o tables.o term.o \
  util.o variable.o wizard.o \
  z-util.o z-virt.o z-form.o \
  x-metadpy.o x-infowin.o x-infoclr.o x-infofnt.o \
  r-metadpy.o r-infowin.o r-infoclr.o r-infofnt.o

CC = gcc
CFLAGS = -Wall -g -D"USE_X11" -D"USE_CURSES"
LIBS = -lX11 -lcurses -ltermcap

angband: $(OBJS)
	$(CC) $(CFLAGS)  -o angband $(OBJS) $(LIBS)

depend:
	makedepend -D__MAKEDEPEND__ $(SRCS)


# DO NOT DELETE THIS LINE -- make depend depends on it.

