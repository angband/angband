MKPATH=mk/
include $(MKPATH)buildsys.mk

SUBDIRS = src lib

.PHONY: tests
tests:
	$(MAKE) -C src tests

clean-posthook:
	-rm config.status config.log
	-rm *.dll
	-rm angband.exe
