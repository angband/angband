MKPATH=mk/

include $(MKPATH)rules.mk
include $(MKPATH)objective.mk

SUBDIRS = src lib

clean-posthook:
	-rm config.status config.log
