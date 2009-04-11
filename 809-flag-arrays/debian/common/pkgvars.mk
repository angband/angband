############################ -*- Mode: Makefile -*- ###########################
## pkgvars.mk --- 
## Author           : Manoj Srivastava ( srivasta@glaurung.green-gryphon.com ) 
## Created On       : Sat Nov 15 02:56:30 2003
## Created On Node  : glaurung.green-gryphon.com
## Last Modified By : Manoj Srivastava
## Last Modified On : Thu Jun 15 12:05:46 2006
## Last Machine Used: glaurung.internal.golden-gryphon.com
## Update Count     : 11
## Status           : Unknown, Use with caution!
## HISTORY          : 
## Description      : This is what allows us toseparate out the top level
##                    targets, by determining which packages needto be built.
## 
## arch-tag: 75fcc720-7389-4eaa-a7ac-c556d3eac331
## 
## 
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
##
###############################################################################

# The maintainer information.
maintainer := $(shell LC_ALL=C dpkg-parsechangelog | grep ^Maintainer: | \
                sed 's/^Maintainer: *//')
email := srivasta@debian.org

# Priority of this version (or urgency, as dchanges would call it)
urgency := $(shell LC_ALL=C dpkg-parsechangelog | grep ^Urgency: | \
             sed 's/^Urgency: *//')

# Common useful variables
DEB_SOURCE_PACKAGE := $(strip $(shell egrep '^Source: ' debian/control      |       \
                                      cut -f 2 -d ':'))
DEB_VERSION        := $(strip $(shell LC_ALL=C dpkg-parsechangelog          |       \
                                      egrep '^Version:' | cut -f 2 -d ' '))
DEB_ISNATIVE       := $(strip $(shell LC_ALL=C dpkg-parsechangelog          |       \
                       perl -ne 'print if (m/^Version:/g && ! m/^Version:.*\-/);'))
DEB_DISTRIBUTION  := $(strip $(shell LC_ALL=C dpkg-parsechangelog          |        \
                                      egrep '^Distribution:' | cut -f 2 -d ' '))

DEB_PACKAGES := $(shell perl -e '                                                    \
                  $$/="";                                                            \
                  while(<>){                                                         \
                     $$p=$$1 if m/^Package:\s*(\S+)/;                                \
                     die "duplicate package $$p" if $$seen{$$p};                     \
                     $$seen{$$p}++; print "$$p " if $$p;                             \
                  }' debian/control )

DEB_INDEP_PACKAGES := $(shell perl -e '                                              \
                         $$/="";                                                     \
                         while(<>){                                                  \
                            $$p=$$1 if m/^Package:\s*(\S+)/;                         \
                            die "duplicate package $$p" if $$seen{$$p};              \
                            $$seen{$$p}++;                                           \
                            $$a=$$1 if m/^Architecture:\s*(\S+)/m;                   \
                            next unless ($$a eq "all");                              \
                            print "$$p " if $$p;                                     \
                         }' debian/control )

DEB_ARCH_PACKAGES := $(shell perl -e '                                               \
                         $$/="";                                                     \
                         while(<>){                                                  \
                            $$p=$$1 if m/^Package:\s*(\S+)/;                         \
                            die "duplicate package $$p" if $$seen{$$p};              \
                            $$seen{$$p}++;                                           \
                            $$c="";                                                  \
	                    if (/^Architecture:\s*(.*?)\s*$$/sm) {                   \
                              @a = split /\s+/, $$1 };                               \
	                      for my $$b (@a) {                                      \
                                next unless ($$b eq "$(DEB_HOST_ARCH)" ||            \
                                             $$b eq "any");                          \
                                $$c="$$p";                                           \
                            }                                                        \
                            print "$$c " if $$c;                                     \
                         }' debian/control )

# This package is what we get after removing the psuedo dirs we use in rules
package = $(notdir $@)

#Local variables:
#mode: makefile
#End:
