############################ -*- Mode: Makefile -*- ###########################
## copt.mk --- 
## Author           : Manoj Srivastava ( srivasta@glaurung.green-gryphon.com ) 
## Created On       : Sat Nov 15 02:48:40 2003
## Created On Node  : glaurung.green-gryphon.com
## Last Modified By : Manoj Srivastava
## Last Modified On : Sat Nov 15 02:49:07 2003
## Last Machine Used: glaurung.green-gryphon.com
## Update Count     : 1
## Status           : Unknown, Use with caution!
## HISTORY          : 
## Description      : 
## 
## arch-tag: a0045c20-f1b3-4852-9a4b-1a33ebd7c1b8
## 
###############################################################################

PREFIX    := /usr
# set CC to $(DEB_HOST_GNU_TYPE)-gcc only if a cross-build is detected
ifneq ($(DEB_HOST_GNU_TYPE),$(DEB_BUILD_GNU_TYPE))
  CC=$(DEB_HOST_GNU_TYPE)-gcc
else
  CC = cc
endif

# Policy 10.1 says to make this the default
CFLAGS = -Wall -g

ifneq (,$(filter noopt,$(DEB_BUILD_OPTIONS)))
    CFLAGS += -O0
else
    CFLAGS += -O2
endif

## ifneq (,$(findstring debug,$(DEB_BUILD_OPTIONS)))
## endif

ifeq (,$(findstring nostrip,$(DEB_BUILD_OPTIONS)))
  STRIP   += -s
  LDFLAGS += -s
  INT_INSTALL_TARGET = install 
else
  INT_INSTALL_TARGET = install
endif
