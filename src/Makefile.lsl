# File: Makefile.lsl

# Purpose: Makefile for Linux + SVGA library

# To Do: Add the "borg" files if desired

SRCS = \
  main.c main-lsl.c \
  signals.c util.c io.c init.c save.c save-old.c files.c \
  generate.c birth.c melee.c dungeon.c store.c \
  effects.c cmd1.c cmd2.c cmd3.c cmd4.c cmd5.c cmd6.c \
  misc.c monster.c mon-desc.c object.c obj-desc.c \
  spells1.c spells2.c cave.c tables.c variable.c \
  term.c z-util.c z-virt.c z-form.c \
  wizard.c wiz-spo.c

OBJS = \
  main.o main-lsl.o \
  signals.o util.o io.o init.o save.o save-old.o files.o \
  generate.o birth.o melee.o dungeon.o store.o \
  effects.o cmd1.o cmd2.o cmd3.o cmd4.o cmd5.o cmd6.o \
  misc.o monster.o mon-desc.o object.o obj-desc.o \
  spells1.o spells2.o cave.o tables.o variable.o \
  term.o z-util.o z-virt.o z-form.o \
  wizard.o wiz-spo.o

CC = gcc

CFLAGS = -Wall -O6 -D"USE_LSL"
LIBS = -lvgagl -lvga

# Build the program

angsvga: $(SRCS) $(OBJS)
	$(CC) $(CFLAGS)  -o angband $(OBJS) $(LDFLAGS) $(LIBS)

