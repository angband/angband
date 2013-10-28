############################ -*- Mode: Makefile -*- ###########################
## archvars.mk --- 
## Author           : Manoj Srivastava ( srivasta@golden-gryphon.com ) 
## Created On       : Sat Nov 15 02:40:56 2003
## Created On Node  : glaurung.green-gryphon.com
## Last Modified By : Manoj Srivastava
## Last Modified On : Tue Nov 16 23:36:15 2004
## Last Machine Used: glaurung.internal.golden-gryphon.com
## Update Count     : 5
## Status           : Unknown, Use with caution!
## HISTORY          : 
## Description      : calls dpkg-architecture and sets up various arch
##                    related variables
## 
## arch-tag: e16dd848-0fd6-4c0e-ae66-bef20d1f7c63
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


DPKG_ARCH := dpkg-architecture

ifeq ($(strip $(KPKG_ARCH)),um)
  MAKING_VIRTUAL_IMAGE:=YES
endif
ifeq ($(strip $(KPKG_ARCH)),xen)
  MAKING_VIRTUAL_IMAGE:=YES
endif

ifneq ($(strip $(CONFIG_UM)),)
  MAKING_VIRTUAL_IMAGE:=YES
  KPKG_ARCH=um
endif

ifneq ($(strip $(CONFIG_XEN)),)
  MAKING_VIRTUAL_IMAGE:=YES
  ifneq ($(strip $(CONFIG_X86_XEN)$(CONFIG_X86_64_XEN)),)
    KPKG_SUBARCH=xen
  else
    KPKG_ARCH=xen
    ifeq ($(strip $(CONFIG_XEN_PRIVILEGED_GUEST)),)
      KPKG_SUBARCH=xenu
    else
      KPKG_SUBARCH=xen0
    endif
  endif
endif

ifdef KPKG_ARCH
  ifeq ($(strip $(MAKING_VIRTUAL_IMAGE)),)
    ifneq ($(CROSS_COMPILE),-)
      ha:=-a$(KPKG_ARCH)
    endif
  endif
endif

# set the dpkg-architecture vars
export DEB_BUILD_ARCH      := $(shell $(DPKG_ARCH)       -qDEB_BUILD_ARCH)
export DEB_BUILD_GNU_CPU   := $(shell $(DPKG_ARCH)       -qDEB_BUILD_GNU_CPU)
export DEB_BUILD_GNU_SYSTEM:= $(shell $(DPKG_ARCH)       -qDEB_BUILD_GNU_SYSTEM)
export DEB_BUILD_GNU_TYPE  := $(shell $(DPKG_ARCH)       -qDEB_BUILD_GNU_TYPE)
export DEB_HOST_ARCH       := $(shell $(DPKG_ARCH) $(ha) -qDEB_HOST_ARCH)
export DEB_HOST_ARCH_OS    := $(shell $(DPKG_ARCH) $(ha) -qDEB_HOST_ARCH_OS      \
                                2>/dev/null|| true)
export DEB_HOST_ARCH_CPU   := $(shell $(DPKG_ARCH) $(ha) -qDEB_HOST_ARCH_CPU     \
                                2>/dev/null|| true)
export DEB_HOST_GNU_CPU    := $(shell $(DPKG_ARCH) $(ha) -qDEB_HOST_GNU_CPU)
export DEB_HOST_GNU_SYSTEM := $(shell $(DPKG_ARCH) $(ha) -qDEB_HOST_GNU_SYSTEM)
export DEB_HOST_GNU_TYPE   := $(shell $(DPKG_ARCH) $(ha) -qDEB_HOST_GNU_TYPE)

# arrgh. future proofing
ifeq ($(DEB_HOST_GNU_SYSTEM), linux)
  DEB_HOST_GNU_SYSTEM=linux-gnu
endif
ifeq ($(DEB_HOST_ARCH_OS),)
  ifeq ($(DEB_HOST_GNU_SYSTEM), linux-gnu)
    DEB_HOST_ARCH_OS := linux
  endif
  ifeq ($(DEB_HOST_GNU_SYSTEM), kfreebsd-gnu)
    DEB_HOST_ARCH_OS := kfreebsd
  endif
endif

REASON = @if [ -f $@ ]; then \
 echo "====== making $(notdir $@) because of $(notdir $?) ======";\
 else \
   echo "====== making target $@ [new prereqs: $(notdir $?)]======"; \
 fi

OLDREASON = @if [ -f $@ ]; then \
 echo "====== making $(notdir $@) because of $(notdir $?) ======";\
 else \
   echo "====== making (creating) $(notdir $@) ======"; \
 fi

LIBREASON = @echo "====== making $(notdir $@)($(notdir $%))because of $(notdir $?)======"


# macro outputing $(1) if DEBUG_DEBIAN_RULES is set, and resolving it
# in all cases usage $(call doit,some shell command)
doit = $(if $(DEBUG_DEBIAN_RULES),$(warning DEBUG: $(1)))$(shell $(1))

#Local variables:
#mode: makefile
#End:
