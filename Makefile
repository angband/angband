MKPATH=mk/
include $(MKPATH)buildsys.mk

SUBDIRS = src lib doc
CLEAN = config.status config.log *.dll *.exe

.PHONY: tests manual clean-manual dist
tests:
	$(MAKE) -C src tests

manual:
	$(MAKE) -C doc manual.html manual.pdf

clean-manual:
	$(MAKE) -C doc clean

TAG = angband-`git describe`
OUT = $(TAG).tar.gz

dist: manual
	git checkout-index --prefix=$(TAG)/ -a
	git describe > $(TAG)/version
	$(TAG)/autogen.sh
	rm -rf $(TAG)/autogen.sh $(TAG)/autom4te.cache
	cp doc/manual.html doc/manual.pdf $(TAG)/doc/
	tar --exclude .gitignore --exclude *.dll -czvf $(OUT) $(TAG)
	rm -rf $(TAG)
