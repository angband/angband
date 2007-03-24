#
# File: Makefile.inc
#
# A makefile which includes the current object files, hopefully includable
# across a range of different makefiles, to cut down on maintenance.
#

HDRS = h-basic.h h-define.h h-type.h h-system.h h-config.h
INCS = angband.h config.h defines.h types.h externs.h \
       z-term.h z-rand.h z-util.h z-virt.h z-form.h $(HDRS)

BASEOBJS= \
  birth$(OBJEXT) \
  cave$(OBJEXT) \
  cmd1$(OBJEXT) \
  cmd2$(OBJEXT) \
  cmd3$(OBJEXT) \
  cmd4$(OBJEXT) \
  cmd5$(OBJEXT) \
  cmd6$(OBJEXT) \
  dungeon$(OBJEXT) \
  files$(OBJEXT) \
  generate$(OBJEXT) \
  init1$(OBJEXT) \
  init2$(OBJEXT) \
  load$(OBJEXT) \
  melee1$(OBJEXT) \
  melee2$(OBJEXT) \
  monster1$(OBJEXT) \
  monster2$(OBJEXT) \
  object1$(OBJEXT) \
  object2$(OBJEXT) \
  obj-info$(OBJEXT) \
  randart$(OBJEXT) \
  save$(OBJEXT) \
  spells1$(OBJEXT) \
  spells2$(OBJEXT) \
  squelch$(OBJEXT) \
  store$(OBJEXT) \
  tables$(OBJEXT) \
  use-obj$(OBJEXT) \
  util$(OBJEXT) \
  variable$(OBJEXT) \
  wizard1$(OBJEXT) \
  wizard2$(OBJEXT) \
  x-spell$(OBJEXT) \
  xtra1$(OBJEXT) \
  xtra2$(OBJEXT)

ZOBJS= \
  z-form$(OBJEXT) \
  z-rand$(OBJEXT) \
  z-term$(OBJEXT) \
  z-util$(OBJEXT) \
  z-virt$(OBJEXT)

# Just print out a list of object files
inc-print:
	@echo $(BASEOBJS)

# Note dependencies
$(BASEOBJS): $(INCS)
$(ZOBJS): $(HDRS)
BASEOBJS += $(ZOBJS)

z-form$(OBJEXT): z-form.h z-util.h z-virt.h
z-rand$(OBJEXT): z-rand.h
z-term$(OBJEXT): z-term.h z-virt.h
z-util$(OBJEXT): z-util.h
z-virt$(OBJEXT): z-virt.h z-util.h

# Depend on script.h
SCRIPTOBJS = cmd5$(OBJEXT) cmd6$(OBJEXT) init2$(OBJEXT) \
             script$(OBJEXT) wizard2$(OBJEXT) xtra1$(OBJEXT)
$(SCRIPTOBJS): script.h

# Depend on init.h
INITOBJS = init1$(OBJEXT) init2$(OBJEXT) randart$(OBJEXT)
$(INITOBJS): init.h
