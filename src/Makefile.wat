# File: Makefile.wat

# Purpose: Makefile support for "main-ibm.c" for Watcom C/C++

# From: akemi@netcom.com (David Boeren)
# Extra program targets by: mrmarcel@eos.ncsu.edu (Mike Marcelais)

CC = wcc386

CFLAGS  = /mf /3r /3 /wx /s /oneatx /DUSE_IBM /DUSE_WAT
# CFLAGS  = /mf /3r /3 /wx /oaeilmnrt /DUSE_IBM /DUSE_WAT

OBJS = &
  z-util.obj z-virt.obj z-form.obj z-rand.obj z-term.obj &
  variable.obj tables.obj util.obj cave.obj &
  object1.obj object2.obj monster1.obj monster2.obj &
  xtra1.obj xtra2.obj spells1.obj spells2.obj melee1.obj melee2.obj &
  load1.obj load2.obj save.obj files.obj &
  cmd1.obj cmd2.obj cmd3.obj cmd4.obj cmd5.obj cmd6.obj &
  store.obj birth.obj wizard1.obj wizard2.obj &
  generate.obj dungeon.obj init1.obj init2.obj &
  main-ibm.obj main.obj

all: angband.exe gredit.exe makepref.exe

# Use whichever of these two you wish...
angband.exe: $(OBJS) angband.lnk
   wlink system dos4g @angband.lnk
#   wlink system pmodew @angband.lnk

# Use whichever of these two you wish...
gredit.exe: gredit.obj gredit.lnk
   wlink system dos4g @gredit.lnk
#   wlink system pmodew @gredit.lnk

# Use whichever of these two you wish...
makepref.exe: makepref.obj makepref.lnk
   wlink system dos4g @makepref.lnk
#   wlink system pmodew @makepref.lnk

angband.lnk:
    %create  angband.lnk
#   @%append angband.lnk debug all
    @%append angband.lnk OPTION CASEEXACT
    @%append angband.lnk OPTION STACK=16k
    @%append angband.lnk name angband
    @for %i in ($(OBJS)) do @%append angband.lnk file %i

makepref.lnk:
    %create  makepref.lnk
#   @%append makepref.lnk debug all
    @%append makepref.lnk OPTION CASEEXACT
    @%append makepref.lnk OPTION STACK=16k
    @%append makepref.lnk name makepref
    @%append makepref.lnk file makepref.obj

gredit.lnk:
    %create  gredit.lnk
#   @%append gredit.lnk debug all
    @%append gredit.lnk OPTION CASEEXACT
    @%append gredit.lnk OPTION STACK=16k
    @%append gredit.lnk name gredit
    @%append gredit.lnk file gredit.obj

.c.obj:
    $(CC) $(CFLAGS) $[*.c

clean:
    del *.err *.obj *.exe *.lnk
