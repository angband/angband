#
# File: Makefile.inc
#
# A makefile which includes the current object files, hopefully includable
# across a range of different makefiles, to cut down on maintenance.
#

HDRS = h-basic.h h-config.h
INCS = angband.h config.h defines.h types.h externs.h \
       z-term.h z-rand.h z-util.h z-virt.h z-form.h $(HDRS)

BASEOBJS := $(addsuffix $(OBJEXT),\
  birth \
  cave \
  cmd1 \
  cmd2 \
  cmd3 \
  cmd4 \
  cmd5 \
  cmd6 \
  dungeon \
  files \
  generate \
  init1 \
  init2 \
  load \
  melee1 \
  melee2 \
  monster1 \
  monster2 \
  object1 \
  object2 \
  obj-info \
  randart \
  save \
  spells1 \
  spells2 \
  squelch \
  store \
  tables \
  ui \
  use-obj \
  util \
  variable \
  wizard1 \
  wizard2 \
  x-spell \
  xtra1 \
  xtra2)

ZOBJS := $(addsuffix $(OBJEXT), z-file z-form z-rand z-term z-util z-virt)

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
z-file$(OBJEXT): z-file.h

# Various files depend on various headers
$(addsuffix $(OBJEXT), cmd5 cmd6 init2 script wizard2 xtra1): script.h
$(addsuffix $(OBJEXT), init1 init2 randart): init.h
$(addsuffix $(OBJEXT), cmd4 files init1 init2 util squelch save load wizard1): z-file.h
