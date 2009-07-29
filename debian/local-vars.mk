############################ -*- Mode: Makefile -*- ###########################
## local-vars.mk --- 
## Author           : Manoj Srivastava ( srivasta@glaurung.green-gryphon.com ) 
## Created On       : Sat Nov 15 10:43:00 2003
## Created On Node  : glaurung.green-gryphon.com
## Last Modified By : Manoj Srivastava
## Last Modified On : Tue Feb 17 09:41:30 2004
## Last Machine Used: glaurung.internal.golden-gryphon.com
## Update Count     : 15
## Status           : Unknown, Use with caution!
## HISTORY          : 
## Description      : 
## 
## arch-tag: 1a76a87e-7af5-424a-a30d-61660c8f243e
## 
###############################################################################

FILES_TO_CLEAN  = debian/files debian/buildinfo debian/substvars angband \
                  config.status config.log
STAMPS_TO_CLEAN = 
DIRS_TO_CLEAN   = $(TMPTOP) autom4te.cache debian/stamp

PREFIX = /usr
R_BINDIR = $(PREFIX)/bin
R_MANDIR = $(PREFIX)/share/man
R_INFODIR= $(PREFIX)/share/info
R_LIBDIR = /var/games/angband

# Location of the source dir
SRCTOP    := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
TMPTOP     = $(SRCTOP)/debian/$(package)
LINTIANDIR = $(TMPTOP)/usr/share/lintian/overrides
DOCBASEDIR = $(TMPTOP)/usr/share/doc-base

BINDIR  = $(TMPTOP)$(PREFIX)/bin
LIBDIR  = $(TMPTOP)$(PREFIX)/lib
DATADIR = $(TMPTOP)/var/games
GAMEDIR = $(DATADIR)/$(package)

# Man Pages
MANDIR    = $(TMPTOP)/usr/share/man
MAN1DIR   = $(MANDIR)/man1
MAN3DIR   = $(MANDIR)/man3
MAN5DIR   = $(MANDIR)/man5
MAN6DIR   = $(MANDIR)/man6
MAN7DIR   = $(MANDIR)/man7
MAN8DIR   = $(MANDIR)/man8

INFODIR = $(TMPTOP)/usr/share/info
DOCTOP  = $(TMPTOP)/usr/share/doc
DOCDIR  = $(DOCTOP)/$(package)

MENUDIR    = $(TMPTOP)/usr/share/menu/
INCLUDEDIR = $(TMPTOP)$(PREFIX)/include

define checkdir
	@test -f debian/rules -a -f src/angband.h || \
          (echo Not in correct source directory; exit 1)
endef

define checkroot
	@test $$(id -u) = 0 || (echo need root priviledges; exit 1)
endef
