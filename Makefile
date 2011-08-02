MKPATH=mk/
include $(MKPATH)buildsys.mk

SUBDIRS = src lib

.PHONY: tests manual
tests:
	$(MAKE) -C src tests

manual:
	$(MAKE) -C doc/manual manual

clean-posthook:
	-rm config.status config.log
	-rm *.dll
	-rm angband.exe
