MKPATH=mk/
include $(MKPATH)buildsys.mk

SUBDIRS = src lib

clean-posthook:
	-rm config.status config.log
