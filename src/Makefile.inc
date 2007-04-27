#
# File: Makefile.inc
#
# Includes Makefile.src and contains dependency info
#

include Makefile.src

BASEOBJS := $(addsuffix $(OBJEXT), $(ANGFILES))
ZOBJS := $(addsuffix $(OBJEXT), $(ZFILES))


# Note dependencies
$(BASEOBJS): $(HEADERS)
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

