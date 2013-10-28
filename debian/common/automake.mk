############################ -*- Mode: Makefile -*- ###########################
## automake.mk --- 
## Author           : Manoj Srivastava ( srivasta@glaurung.green-gryphon.com ) 
## Created On       : Sat Nov 15 02:47:23 2003
## Created On Node  : glaurung.green-gryphon.com
## Last Modified By : Manoj Srivastava
## Last Modified On : Sat Nov 15 02:47:53 2003
## Last Machine Used: glaurung.green-gryphon.com
## Update Count     : 1
## Status           : Unknown, Use with caution!
## HISTORY          : 
## Description      : 
## 
## arch-tag: 1fabe69b-7cc8-4ecc-9411-bc5906b19857
## 
###############################################################################

AUTOCONF_VERSION:=$(shell if [ -e configure ]; then                       \
                       grep "Generated automatically using autoconf"      \
                       configure | sed -e 's/^.*autoconf version //g';    \
                      fi)
HAVE_NEW_AUTOMAKE:=$(shell if [ "X$(AUTOCONF_VERSION)" != "X2.13" ]; then \
                             echo 'YES' ; fi)

ifneq ($(strip $(HAVE_NEW_AUTOMAKE)),)
  ifeq ($(DEB_BUILD_GNU_TYPE), $(DEB_HOST_GNU_TYPE))
       confflags += --build $(DEB_BUILD_GNU_TYPE) 
  else
       confflags += --build $(DEB_BUILD_GNU_TYPE) --host $(DEB_HOST_GNU_TYPE)
  endif
else
  ifeq ($(DEB_BUILD_GNU_TYPE), $(DEB_HOST_GNU_TYPE))
       confflags += $(DEB_HOST_GNU_TYPE)
  else
       confflags += --build $(DEB_BUILD_GNU_TYPE) --host $(DEB_HOST_GNU_TYPE)
  endif
endif
