#
# File: Makefile.inc
#
# Includes Makefile.src and contains dependency info
#

include Makefile.src

BASEOBJS := $(ANGFILES)
ZOBJS := $(ZFILES)

# Note dependencies
$(BASEOBJS): $(HEADERS)
$(ZOBJS): $(HDRS)
BASEOBJS += $(ZOBJS)

z-form.o: z-form.h z-util.h z-virt.h
z-term.o: z-term.h z-virt.h
z-virt.o: z-virt.h z-util.h
z-smap.o: z-smap.h

# Various files depend on various headers
$(addsuffix .o, cmd5 cmd6 init2 script wizard2 xtra1): script.h
$(addsuffix .o, init1 init2 randart): init.h
$(addsuffix .o, cmd4 files init1 init2 util squelch save load wizard1): z-file.h
$(addsuffic .o, init1 effects): effects.h
