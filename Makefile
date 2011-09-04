MKPATH=mk/
include $(MKPATH)buildsys.mk

SUBDIRS = src lib doc
CLEAN = config.status config.log *.dll *.exe

.PHONY: tests manual
tests:
	$(MAKE) -C src tests

manual:
	$(MAKE) -C doc manual.html manual.pdf

