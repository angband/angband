############################ -*- Mode: Makefile -*- ###########################
## targets.mk ---
## Author	    : Manoj Srivastava ( srivasta@glaurung.green-gryphon.com )
## Created On	    : Sat Nov 15 01:10:05 2003
## Created On Node  : glaurung.green-gryphon.com
## Last Modified By : Manoj Srivastava
## Last Modified On : Sat Apr 26 22:33:09 2008
## Last Machine Used: anzu.internal.golden-gryphon.com
## Update Count	    : 131
## Status	    : Unknown, Use with caution!
## HISTORY	    :
## Description	    : The top level targets mandated by policy, as well as
##		      their dependencies.
##
## arch-tag: a81086a7-00f7-4355-ac56-8f38396935f4
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
##
###############################################################################

#######################################################################
#######################################################################
###############		    Miscellaneous		###############
#######################################################################
#######################################################################
source diff:
	@echo >&2 'source and diff are obsolete - use dpkg-source -b'; false

define TESTROOT
	@test $$(id -u) = 0 || (echo need root privileges; exit 1)
endef

testroot:
	$(TESTROOT)

checkpo:
	$(CHECKPO)

# arch-buildpackage likes to call this
prebuild:

# OK. We have two sets of rules here, one for arch dependent packages,
# and one for arch independent packages. We have already calculated a
# list of each of these packages.

# In each set, we may need to do things in five steps: configure,
# build, install, package, and clean. Now, there can be a common
# actions to be taken for all the packages, all arch dependent
# packages, all all independent packages, and each package
# individually at each stage.

###########################################################################
# The current code does a number of things: It ensures that the highest	  #
# dependency at any stage (usually the -Common target) depends on the	  #
# stamp-STAGE of the previous stage; so no work on a succeeding stage can #
# start before the previous stage is all done.				  #
###########################################################################

#################################################################################
# In the following, the do_* targets make sure all the real non-generic work is #
# done, but are not in the direct line of dependencies. This makes sure	        #
# that previous step in the order is all up to date before any of the per       #
# package target dependencies are run.					        #
#################################################################################


#######################################################################
#######################################################################
###############		    Configuration		###############
#######################################################################
#######################################################################
# Just a dummy target to make sure that the stamp directory exists
debian/stamp/dummy-config-common:
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

# Configuration tasks common to arch and arch indep packages go here
debian/stamp/pre-config-common: debian/stamp/dummy-config-common
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
# Do not add dependencies to this rule
debian/stamp/do-pre-config-common: debian/stamp/dummy-config-common
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/pre-config-common
	@echo done > $@

# Arch specific and arch independent tasks go here
debian/stamp/pre-config-arch:	debian/stamp/do-pre-config-common
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
# Do not add dependencies to this rule
debian/stamp/do-pre-config-arch:	debian/stamp/do-pre-config-common
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/pre-config-arch
	@echo done > $@


debian/stamp/pre-config-indep: debian/stamp/do-pre-config-common
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
# Do not add dependencies to this rule
debian/stamp/do-pre-config-indep: debian/stamp/do-pre-config-common
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/pre-config-indep
	@echo done > $@

# Per package work happens as an added dependency of this rule.
$(patsubst %,debian/stamp/CONFIG/%,$(DEB_ARCH_PACKAGES))  : debian/stamp/CONFIG/% : debian/stamp/do-pre-config-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp/CONFIG || mkdir -p debian/stamp/CONFIG
	@echo done > $@
$(patsubst %,debian/stamp/CONFIG/%,$(DEB_INDEP_PACKAGES)) : debian/stamp/CONFIG/% : debian/stamp/do-pre-config-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp/CONFIG || mkdir -p debian/stamp/CONFIG
	@echo done > $@

# Do not add dependencies to this rule
debian/stamp/dep-configure-arch: debian/stamp/do-pre-config-arch $(patsubst %,debian/stamp/CONFIG/%,$(DEB_ARCH_PACKAGES)) 
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

# Do not add dependencies to this rule
debian/stamp/dep-configure-indep: debian/stamp/do-pre-config-indep $(patsubst %,debian/stamp/CONFIG/%,$(DEB_INDEP_PACKAGES))
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

debian/stamp/do-configure-arch: debian/stamp/do-pre-config-arch
	$(REASON)
	@test -d debian/stamp/CONFIG || mkdir -p debian/stamp/CONFIG
	$(MAKE) -f debian/rules debian/stamp/dep-configure-arch
	@echo done > $@
debian/stamp/do-configure-indep: debian/stamp/do-pre-config-indep
	$(REASON)
	@test -d debian/stamp/CONFIG || mkdir -p debian/stamp/CONFIG
	$(MAKE) -f debian/rules debian/stamp/dep-configure-indep
	@echo done > $@

# These three targets are required by policy
configure-arch:	 debian/stamp/do-configure-arch
	$(REASON)
configure-indep: debian/stamp/do-configure-indep
	$(REASON)
configure: debian/stamp/do-configure-arch debian/stamp/do-configure-indep
	$(REASON)

#######################################################################
#######################################################################
###############			Build			###############
#######################################################################
#######################################################################
# tasks common to arch and arch indep packages go here
debian/stamp/pre-build-common:
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

# Arch specific and arch independent tasks go here
debian/stamp/pre-build-arch:  debian/stamp/do-configure-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
debian/stamp/do-pre-build-arch:  debian/stamp/do-configure-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@test -e debian/stamp/pre-build-common || $(MAKE) -f debian/rules debian/stamp/pre-build-common
	$(MAKE) -f debian/rules debian/stamp/pre-build-arch
	@echo done > $@

debian/stamp/pre-build-indep: debian/stamp/do-configure-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
debian/stamp/do-pre-build-indep: debian/stamp/do-configure-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@test -e debian/stamp/pre-build-common || $(MAKE) -f debian/rules debian/stamp/pre-build-common
	$(MAKE) -f debian/rules debian/stamp/pre-build-indep
	@echo done > $@

# Per package work happens  as an added dependency of this rule.
$(patsubst %,debian/stamp/BUILD/%,$(DEB_ARCH_PACKAGES))	 : debian/stamp/BUILD/% : debian/stamp/do-pre-build-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp/BUILD || mkdir -p debian/stamp/BUILD
	@echo done > $@

$(patsubst %,debian/stamp/BUILD/%,$(DEB_INDEP_PACKAGES)) : debian/stamp/BUILD/% : debian/stamp/do-pre-build-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp/BUILD || mkdir -p debian/stamp/BUILD
	@echo done > $@

# These do targeta make sure all the per package configuration is
# done, but is not in the direct line of dependencies. This makes sure
# that pre-config targets are all up to date before any of the per
# package target dependencies are run.
debian/stamp/dep-build-arch: debian/stamp/do-pre-build-arch $(patsubst %,debian/stamp/BUILD/%,$(DEB_ARCH_PACKAGES)) 
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

debian/stamp/dep-build-indep: debian/stamp/do-pre-build-indep $(patsubst %,debian/stamp/BUILD/%,$(DEB_INDEP_PACKAGES))
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

debian/stamp/do-build-arch: debian/stamp/do-pre-build-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/dep-build-arch
	@echo done > $@
debian/stamp/do-build-indep: debian/stamp/do-pre-build-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/dep-build-indep
	@echo done > $@

# required
build-arch: debian/stamp/do-build-arch
	$(REASON)
build-indep: debian/stamp/do-build-indep
	$(REASON)
build: debian/stamp/do-build-arch debian/stamp/do-build-indep
	$(REASON)

# Work here
debian/stamp/post-build-arch: debian/stamp/do-build-arch
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
debian/stamp/do-post-build-arch: debian/stamp/do-build-arch
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/post-build-arch
	@echo done > $@

debian/stamp/post-build-indep: debian/stamp/do-build-indep
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
debian/stamp/do-post-build-indep: debian/stamp/do-build-indep
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/post-build-indep
	@echo done > $@

#######################################################################
#######################################################################
###############		       Install			###############
#######################################################################
#######################################################################
# tasks common to arch and arch indep packages go here
debian/stamp/pre-inst-common:
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

# Arch specific and arch independent tasks go here
debian/stamp/pre-inst-arch:  debian/stamp/do-post-build-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
debian/stamp/do-pre-inst-arch:  debian/stamp/do-post-build-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@test -e debian/stamp/INST-common || $(MAKE) -f debian/rules debian/stamp/pre-inst-common
	$(MAKE) -f debian/rules debian/stamp/pre-inst-arch
	@echo done > $@

debian/stamp/pre-inst-indep: debian/stamp/do-post-build-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
debian/stamp/do-pre-inst-indep: debian/stamp/do-post-build-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@test -e debian/stamp/INST-common || $(MAKE) -f debian/rules debian/stamp/pre-inst-common
	$(MAKE) -f debian/rules debian/stamp/pre-inst-indep
	@echo done > $@


# Per package work happens as an added dependency of this rule
$(patsubst %,debian/stamp/INST/%,$(DEB_ARCH_PACKAGES))	: debian/stamp/INST/% : debian/stamp/do-pre-inst-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp/INST || mkdir -p debian/stamp/INST
	@echo done > $@
$(patsubst %,debian/stamp/INST/%,$(DEB_INDEP_PACKAGES)) : debian/stamp/INST/% : debian/stamp/do-pre-inst-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp/INST || mkdir -p debian/stamp/INST
	@echo done > $@

# These do targeta make sure all the per package configuration is
# done, but is not in the direct line of dependencies. This makes sure
# that pre-config targets are all up to date before any of the per
# package target dependencies are run.
debian/stamp/dep-install-arch: debian/stamp/do-pre-inst-arch $(patsubst %,debian/stamp/INST/%,$(DEB_ARCH_PACKAGES))
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

debian/stamp/dep-install-indep: debian/stamp/do-pre-inst-indep $(patsubst %,debian/stamp/INST/%,$(DEB_INDEP_PACKAGES))
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@


debian/stamp/do-install-arch: debian/stamp/do-pre-inst-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/dep-install-arch
	@echo done > $@
debian/stamp/do-install-indep:  debian/stamp/do-pre-inst-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/dep-install-indep
	@echo done > $@

#required
install-arch:  debian/stamp/do-install-arch
	$(REASON)
	$(TESTROOT)
install-indep: debian/stamp/do-install-indep
	$(REASON)
	$(TESTROOT)
install: debian/stamp/do-install-arch debian/stamp/do-install-indep
	$(REASON)
	$(TESTROOT)

#######################################################################
#######################################################################
###############		       Package			###############
#######################################################################
#######################################################################
# tasks common to arch and arch indep packages go here
debian/stamp/pre-bin-common:
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

# Arch specific and arch independent tasks go here
debian/stamp/pre-bin-arch:  debian/stamp/do-install-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
debian/stamp/do-pre-bin-arch:  debian/stamp/do-install-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@test -e debian/stamp/BIN-common || $(MAKE) -f debian/rules debian/stamp/pre-bin-common
	$(MAKE) -f debian/rules debian/stamp/pre-bin-arch
	@echo done > $@

debian/stamp/pre-bin-indep: debian/stamp/do-install-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@
debian/stamp/do-pre-bin-indep: debian/stamp/do-install-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	@test -e debian/stamp/BIN-common || $(MAKE) -f debian/rules debian/stamp/pre-bin-common
	$(MAKE) -f debian/rules debian/stamp/pre-bin-indep
	@echo done > $@

# Per package work happens as an added dependency of this rule
$(patsubst %,debian/stamp/BIN/%,$(DEB_ARCH_PACKAGES))  : debian/stamp/BIN/% : debian/stamp/do-pre-bin-arch
	$(REASON)
	$(checkdir)
	@test -d debian/stamp/BIN || mkdir -p debian/stamp/BIN
	@echo done > $@

$(patsubst %,debian/stamp/BIN/%,$(DEB_INDEP_PACKAGES)) : debian/stamp/BIN/% : debian/stamp/do-pre-bin-indep
	$(REASON)
	$(checkdir)
	@test -d debian/stamp/BIN || mkdir -p debian/stamp/BIN
	@echo done > $@

# These do targeta make sure all the per package work is done, but is
# not in the direct line of dependencies. This makes sure that
# pre-config targets are all up to date before any of the per package
# target dependencies are run.
debian/stamp/dep-binary-arch: debian/stamp/pre-bin-arch $(patsubst %,debian/stamp/BIN/%,$(DEB_ARCH_PACKAGES))
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

debian/stamp/dep-binary-indep: debian/stamp/pre-bin-indep $(patsubst %,debian/stamp/BIN/%,$(DEB_INDEP_PACKAGES))
	$(REASON)
	@test -d debian/stamp || mkdir -p debian/stamp
	@echo done > $@

debian/stamp/do-binary-arch:  debian/stamp/do-pre-bin-arch 
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/dep-binary-arch
	@echo done > $@
debian/stamp/do-binary-indep: debian/stamp/do-pre-bin-indep 
	$(REASON)
	$(checkdir)
	@test -d debian/stamp || mkdir -p debian/stamp
	$(MAKE) -f debian/rules debian/stamp/dep-binary-indep
	@echo done > $@
# required
binary-arch:  debian/stamp/do-binary-arch
	$(REASON)
	$(TESTROOT)
binary-indep: debian/stamp/do-binary-indep
	$(REASON)
	$(TESTROOT)
binary: debian/stamp/do-binary-arch debian/stamp/do-binary-indep
	$(REASON)
	$(TESTROOT)
	@echo arch package   = $(DEB_ARCH_PACKAGES)
	@echo indep packages = $(DEB_INDEP_PACKAGES)

#######################################################################
#######################################################################
###############			Clean			###############
#######################################################################
#######################################################################
# Work here
CLN-common:: 
	$(REASON)
	$(checkdir)

# sync Work here
CLN-arch::  CLN-common
	$(REASON)
	$(checkdir)
CLN-indep:: CLN-common
	$(REASON)
	$(checkdir)
# Work here
$(patsubst %,CLEAN/%,$(DEB_ARCH_PACKAGES))  :: CLEAN/% : CLN-arch
	$(REASON)
	$(checkdir)
$(patsubst %,CLEAN/%,$(DEB_INDEP_PACKAGES)) :: CLEAN/% : CLN-indep
	$(REASON)
	$(checkdir)

clean-arch: CLN-arch $(patsubst %,CLEAN/%,$(DEB_ARCH_PACKAGES))
	$(REASON)
clean-indep: CLN-indep $(patsubst %,CLEAN/%,$(DEB_INDEP_PACKAGES))
	$(REASON)
clean: clean-indep clean-arch
	$(REASON)
	-test -f Makefile && $(MAKE) distclean
	-rm -f	$(FILES_TO_CLEAN) $(STAMPS_TO_CLEAN)
	-rm -rf $(DIRS_TO_CLEAN) debian/stamp
	-rm -f core TAGS						     \
	       `find . ! -regex '.*/\.git/.*' ! -regex '.*/\{arch\}/.*'	     \
		       ! -regex '.*/CVS/.*'   ! -regex '.*/\.arch-ids/.*'    \
		       ! -regex '.*/\.svn/.*'				     \
		   \( -name '*.orig' -o -name '*.rej' -o -name '*~'	  -o \
		      -name '*.bak'  -o -name '#*#'   -o -name '.*.orig'  -o \
		      -name '.*.rej' -o -name '.SUMS' \)		     \
		-print`


#######################################################################
#######################################################################
###############						###############
#######################################################################
#######################################################################
.PHONY: configure-arch	configure-indep	 configure     \
	build-arch	build-indep	 build	       \
	install-arch	install-indep	 install       \
	binary-arch	binary-indep	 binary	       \
	CLN-common     CLN-indep     CLN-arch	  clean-arch	  clean-indep	   clean	 \
	$(patsubst %,CLEAN/%, $(DEB_INDEP_PACKAGES)) $(patsubst %,CLEAN/%, $(DEB_ARCH_PACKAGES)) \
	implode explode prebuild checkpo


#Local variables:
#mode: makefile
#End:
