# File: Makefile.wat

# Purpose: Makefile support for "main-ibm.c" for Watcom C/C++

# From: akemi@netcom.com (David Boeren)

CC = wcc386

CFLAGS  = /mf /3r /3 /wx /s /oneatx /DUSE_IBM /DUSE_WAT
# CFLAGS  = /mf /3r /3 /wx /oaeilmnrt /DUSE_IBM /DUSE_WAT

OBJS = &
    main.obj main-ibm.obj signals.obj util.obj io.obj init.obj save.obj &
    save-old.obj files.obj generate.obj birth.obj melee.obj &
    dungeon.obj effects.obj store.obj cmd1.obj cmd2.obj cmd3.obj cmd4.obj &
    cmd5.obj cmd6.obj misc.obj monster.obj mon-desc.obj object.obj &
    obj-desc.obj spells1.obj spells2.obj cave.obj tables.obj variable.obj &
    term.obj z-util.obj z-virt.obj z-form.obj

# Use whichever of these two you wish...
angband.exe: $(OBJS) angband.lnk
   wlink system dos4g @angband.lnk
#   wlink system pmodew @angband.lnk

angband.lnk:
    %create  angband.lnk
    @%append angband.lnk debug all
    @%append angband.lnk OPTION CASEEXACT
    @%append angband.lnk OPTION STACK=16k
    @%append angband.lnk name angband
    @for %i in ($(OBJS)) do @%append angband.lnk file %i

.c.obj:
    $(CC) $(CFLAGS) $[*.c

