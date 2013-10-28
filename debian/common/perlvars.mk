############################ -*- Mode: Makefile -*- ###########################
## perlvars.mk --- 
## Author           : Manoj Srivastava ( srivasta@glaurung.green-gryphon.com ) 
## Created On       : Sat Nov 15 02:55:47 2003
## Created On Node  : glaurung.green-gryphon.com
## Last Modified By : Manoj Srivastava
## Last Modified On : Sat Dec 13 13:50:58 2003
## Last Machine Used: glaurung.green-gryphon.com
## Update Count     : 3
## Status           : Unknown, Use with caution!
## HISTORY          : 
## Description      : 
## 
## arch-tag: a97a01ba-d08d-404d-aa81-572717c03e6c
## 
###############################################################################

# Perl variables
PERL = /usr/bin/perl

INSTALLPRIVLIB  = $(TMPTOP)/$(shell \
                 perl -e 'use Config; print "$$Config{'installprivlib'}\n";')
INSTALLARCHLIB  = $(TMPTOP)/$(shell \
                 perl -e 'use Config; print "$$Config{'installarchlib'}\n";')
INSTALLVENDORLIB =$(TMPTOP)/$(shell \
                 perl -e 'use Config; print "$$Config{'vendorlibexp'}\n";')
CONFIG           = INSTALLDIRS=vendor 
