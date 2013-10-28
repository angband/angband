############################ -*- Mode: Makefile -*- ###########################
## debconf.mk --- 
## Author           : Manoj Srivastava ( srivasta@glaurung.internal.golden-gryphon.com ) 
## Created On       : Fri Mar 12 11:11:31 2004
## Created On Node  : glaurung.internal.golden-gryphon.com
## Last Modified By : Manoj Srivastava
## Last Modified On : Mon Apr 11 13:19:10 2005
## Last Machine Used: glaurung.internal.golden-gryphon.com
## Update Count     : 20
## Status           : Unknown, Use with caution!
## HISTORY          : 
## Description      : helps with using debconf
## 
## arch-tag: 32b933a9-05ad-4c03-97a8-8644745b832a
##
###############################################################################

# The idea behind this scheme is that the maintainer (or whoever's
# building the package for upload to unstable) has to build on a
# machine with po-debconf installed, but nobody else does.

# When building with po-debconf, a format 1 (no encoding specifications,
# woody-compatible) debian/templates file is generated in the clean target
# and shipped in the source package, but a format 2 (UTF8-encoded,
# woody-incompatible) debian/templates file is generated in binary-arch
# for the binary package only.

# When building without po-debconf, the binary package simply reuses the
# woody-compatible debian/templates file that was produced by the clean
# target of the maintainer's build.

# Also, make sure that debian/control has ${debconf-depends} in the
# appropriate Depends: line., and use the following in the binary
# target:
#  dpkg-gencontrol -V'debconf-depends=debconf (>= $(MINDEBCONFVER))'
#

# WARNING!! You need to create the templates.master file before this all works.

ifeq (,$(wildcard /usr/bin/po2debconf))
 PO2DEBCONF    := no
 MINDEBCONFVER := 0.5
else
 PO2DEBCONF    := yes
 MINDEBCONFVER := 1.2.0
endif


# Hack for woody compatibility. This makes sure that the
# debian/templates file shipped in the source package doesn't specify
# encodings, which woody's debconf can't handle. If building on a
# system with po-debconf installed the binary-arch target will
# generate a better version for sarge. Only do this if there is a
# templates.master, or else the debian/templates file can get
# damamged. 
ifeq ($(PO2DEBCONF),yes)
  ifeq (,$(wildcard debian/templates.master))
define CREATE_COMPATIBLE_TEMPLATE
	echo Not modifying templates
endef
  else
define CREATE_COMPATIBLE_TEMPLATE
	echo 1 > debian/po/output
	po2debconf debian/templates.master > debian/templates
	rm -f debian/po/output
endef
  endif
else
define CREATE_COMPATIBLE_TEMPLATE
	echo Not modifying templates
endef
endif


ifeq ($(PO2DEBCONF),yes)
  ifeq (,$(wildcard debian/templates.master))
define INSTALL_TEMPLATE
	echo using old template
endef
  else
define INSTALL_TEMPLATE
	po2debconf debian/templates.master > debian/templates
endef
  endif
else
define INSTALL_TEMPLATE
	echo using old template
endef
endif

# the tool podebconf-report-po is also a great friend to have in such
# circumstances 
define CHECKPO
	@for i in debian/po/*.po; do                         \
	  if [ -f $$i ]; then                        \
	    echo \"Checking: $$i\";                  \
	    msgmerge -U $$i debian/po/templates.pot;        \
	    msgfmt -o /dev/null -c --statistics $$i; \
	  fi;                                        \
	done
endef
