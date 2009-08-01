############################ -*- Mode: Makefile -*- ###########################
## local.mk --- 
## Author           : Manoj Srivastava ( srivasta@glaurung.green-gryphon.com ) 
## Created On       : Sat Nov 15 10:42:10 2003
## Created On Node  : glaurung.green-gryphon.com
## Last Modified By : Chris Carr
## Last Modified On : Thu Apr 16 21:59:17 2009
## Last Machine Used: xaphod.dbass.homelinux.com
## Update Count     : 27
## Status           : Unknown, Use with caution!
## HISTORY          : 
## Description      : 
## 
## arch-tag: b07b1015-30ba-4b46-915f-78c776a808f4
## 
###############################################################################

testdir:
	$(testdir)

debian/stamp/pre-config-common: debian/stamp/conf/angband
debian/stamp/BUILD/angband:     debian/stamp/build/angband
debian/stamp/INST/angband:      debian/stamp/install/angband
debian/stamp/BIN/angband:       debian/stamp/binary/angband


CLN-common::
	$(checkdir)
	$(REASON)
	-test ! -f Makefile || $(MAKE) distclean

CLEAN/angband::
	$(REASON)
	-rm -rf $(TMPTOP)

debian/stamp/conf/angband:
	$(checkdir)
	$(REASON)
	@test -d debian/stamp/conf || mkdir -p debian/stamp/conf
	./autogen.sh
	./configure  --verbose --prefix=$(PREFIX)                       \
                     --datadir=$(R_LIBDIR) --mandir=$(R_MANDIR)         \
                     --infodir=$(R_INFODIR) --sysconfdir=/etc           \
                     --with-setgid=games    --with-libpath=$(R_LIBDIR)/ \
		     --with-x --enable-sdl $(confflags) 
	@echo done > $@

debian/stamp/build/angband:
	$(checkdir)
	$(REASON)
	@test -d debian/stamp/build || mkdir -p debian/stamp/build
	$(MAKE)
	dpkg -l gcc 'libc*' binutils ldso make dpkg-dev | \
          awk '$$1 == "ii" { printf("%s-%s\n", $$2, $$3) }' > \
           debian/buildinfo
	@echo done > $@

debian/stamp/install/angband:
	$(checkdir)
	$(REASON)
	$(TESTROOT)
	rm -rf               $(TMPTOP)
	$(make_directory)    $(TMPTOP)
	$(make_directory)    $(TMPTOP)/etc/$(package)
	$(make_directory)    $(TMPTOP)/usr/games
	$(make_directory)    $(GAMEDIR)
	$(make_directory)    $(LIBDIR)/$(package)
	$(make_directory)    $(MAN6DIR)
	$(make_directory)    $(DOCDIR)
	$(make_directory)    $(MENUDIR)
	$(make_directory)    $(LINTIANDIR)
	echo '$(package): description-synopsis-might-not-be-phrased-properly'>> \
                              $(LINTIANDIR)/$(package)
	echo 'angband: non-standard-dir-perm var/games/angband/ 2755 != 0755' \
                             >> $(LINTIANDIR)/$(package)
	echo 'angband: non-standard-dir-perm var/games/angband/apex/ 0775 != 0755' \
                             >> $(LINTIANDIR)/$(package)
	echo 'angband: non-standard-dir-perm var/games/angband/data/ 0775 != 0755' \
                             >> $(LINTIANDIR)/$(package)
	echo 'angband: non-standard-dir-perm var/games/angband/bone/ 0070 != 0755' \
                             >> $(LINTIANDIR)/$(package)
	echo 'angband: non-standard-dir-perm var/games/angband/save/ 0070 != 0755' \
                             >> $(LINTIANDIR)/$(package)
	(DEFAULT_PATH=$(TMPTOP)/var/games/angband/; export DEFAULT_PATH;       \
         AM_MAKEFLAGS="DEFAULT_PATH=$(TMPTOP)/var/games/angband/"; export AM_MAKEFLAGS; \
         $(MAKE)              $(INT_INSTALL_TARGET)    prefix=$(TMPTOP)/usr \
		       	      infodir=$(INFODIR) mandir=$(MANDIR)       \
		              datadir=$(DATADIR) \
                             AM_MAKEFLAGS="DEFAULT_PATH=$(GAMEDIR)"       \
                             DEFAULT_PATH=$(GAMEDIR) DATA_PATH=$(GAMEDIR)) 
	$(install_file)      debian/changelog     $(DOCDIR)/changelog.Debian
	$(install_file)      debian/README.debian $(DOCDIR)/
	$(install_file)      debian/Xresources    $(DOCDIR)/
	$(install_file)      debian/buildinfo     $(DOCDIR)/buildinfo.Debian
	$(install_file)      changes.txt	  $(DOCDIR)/changelog
	$(install_file)      faq.txt		  $(DOCDIR)/faq
	$(install_file)      readme.txt           $(DOCDIR)/readme
	$(install_file)      thanks.txt		  $(DOCDIR)/thanks
	$(install_file)      src/angband.man      $(MAN6DIR)/angband.6
	gzip -9fqr           $(DOCDIR)
	gzip -9fqr           $(MANDIR)
	(cd $(GAMEDIR);   mv edit file pref $(TMPTOP)/etc/$(package);   \
                             ln -s /etc/$(package)/edit . ;                 \
                             ln -s /etc/$(package)/file . ;                 \
                             ln -s /etc/$(package)/pref . ;     )
	(cd $(GAMEDIR);   mv script help $(LIBDIR)/$(package);    \
#                             ln -s /usr/lib/$(package)/script . ;   \
                             ln -s /usr/lib/$(package)/help   . ;   )
	for i in bone/delete.me save/delete.me ; do \
            chmod 0644 $(GAMEDIR)/$$i;            \
        done
	rm -f                   $(GAMEDIR)/apex/scores.raw
	rm -f                   $(GAMEDIR)/xtra/font/copying.txt
#	rmdir			$(TMPTOP)/usr/include/angband
#	rmdir			$(TMPTOP)/usr/include
# Make sure the copyright file is not compressed
	$(install_file)         debian/copyright     $(DOCDIR)/
	$(install_file)         debian/menuentry     $(MENUDIR)/$(package)
	mv                      $(TMPTOP)/usr/bin/angband \
                                 $(TMPTOP)/usr/games/angband 
	rmdir			$(TMPTOP)/usr/bin
ifeq (,$(findstring nostrip,$(DEB_BUILD_OPTIONS)))
	strip $(STRIP)          --remove-section=.comment  \
                                --remove-section=.note \
                                $(TMPTOP)/usr/games/angband
endif
	@test -d debian/stamp/install || mkdir -p debian/stamp/install
	@echo done > $@

debian/stamp/binary/angband:
	$(checkdir)
	$(REASON)
	$(TESTROOT)
	$(make_directory)    $(TMPTOP)/DEBIAN
	$(install_file)      debian/conffiles       $(TMPTOP)/DEBIAN/conffiles
	$(install_program)   debian/preinst         $(TMPTOP)/DEBIAN/preinst
	$(install_program)   debian/postrm          $(TMPTOP)/DEBIAN/postrm
	$(install_script)    debian/postinst        $(TMPTOP)/DEBIAN/postinst
	$(install_script)    debian/prerm           $(TMPTOP)/DEBIAN/prerm
	dpkg-shlibdeps       $(TMPTOP)/usr/games/angband 
	dpkg-gencontrol      -p$(package) -isp      -P$(TMPTOP)
	$(create_md5sum)     $(TMPTOP)
	chown -R root        $(TMPTOP)
	chmod -R u+w,go=rX   $(TMPTOP)
	chown -R root:games  $(GAMEDIR) $(TMPTOP)/usr/games/angband 
	chmod u+rwx,g+rs     $(GAMEDIR) $(TMPTOP)/usr/games/angband 
	chmod -R 0664        $(GAMEDIR)/apex $(GAMEDIR)/data
	chmod 0775           $(GAMEDIR)/apex $(GAMEDIR)/data
	chmod 0070           $(GAMEDIR)/bone $(GAMEDIR)/save 
	dpkg --build         $(TMPTOP) ..
	@test -d debian/stamp/binary || mkdir -p debian/stamp/binary
	@echo done > $@
