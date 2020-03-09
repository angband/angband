MKPATH=mk/
include $(MKPATH)buildsys.mk

SUBDIRS = src lib
CLEAN = config.status config.log *.dll *.exe

.PHONY: tests dist
tests:
	$(MAKE) -C src tests

TAG = angband-`git describe`
OUT = $(TAG).tar.gz

manual:
	echo "To make the manual, please enter docs/ and run `make html` after installing Sphinx."

dist:
	git checkout-index --prefix=$(TAG)/ -a
	git describe > $(TAG)/version
	$(TAG)/autogen.sh
	rm -rf $(TAG)/autogen.sh $(TAG)/autom4te.cache
	tar --exclude .gitignore --exclude *.dll -czvf $(OUT) $(TAG)
	rm -rf $(TAG)
