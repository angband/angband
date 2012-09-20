## Makefile.am -- Process this file with automake to process Makefile.in

angband_files = \
  delete.me

EXTRA_DIST = \
  $(angband_files)

if SET_GID
angbanddir = @DEFAULT_PATH@data

angband_DATA = \
  $(angband_files)
endif


install-data-hook:
if SET_GID
	if test "@GAMEGROUP@" != ""; then \
	  touch "$(DESTDIR)$(angbanddir)/artifact.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/cost_adj.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/ego_item.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/flavor.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/limits.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/monster.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/object.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/p_class.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/p_hist.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/p_race.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/prices.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/shop_own.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/terrain.raw" && \
	  touch "$(DESTDIR)$(angbanddir)/vault.raw" && \
	  chown -R root:@GAMEGROUP@ "$(DESTDIR)$(angbanddir)" && \
	  chmod -R g+w "$(DESTDIR)$(angbanddir)" ;\
	fi
endif

