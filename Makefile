MKPATH=mk/
include $(MKPATH)buildsys.mk

SUBDIRS = src lib
CLEAN = config.status config.log *.dll *.exe
DISTCLEAN = docs/.deps mk/buildsys.mk mk/extra.mk mk/sinclude.mk

.PHONY: tests dist
tests:
	$(MAKE) -C src tests

TAG = angband-`scripts/version.sh`
OUT = $(TAG).tar.gz

manual:
	echo "To make the manual, please enter docs/ and run `make html` after installing Sphinx."

dist:
	git checkout-index --prefix=$(TAG)/ -a
	scripts/version.sh > $(TAG)/version
	$(TAG)/autogen.sh
	rm -rf $(TAG)/autogen.sh $(TAG)/autom4te.cache
	tar --exclude .gitignore --exclude *.dll --exclude .github \
		--exclude .travis.yml -czvf $(OUT) $(TAG)
	rm -rf $(TAG)

# Hack to clean up test results in tests.
pre-distclean:
	@find tests -name run.out -exec rm {} \;
