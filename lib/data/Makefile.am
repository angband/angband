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
	  touch "$(angbanddir)/a_info.raw" && \
	  touch "$(angbanddir)/e_info.raw" && \
	  touch "$(angbanddir)/g_info.raw" && \
	  touch "$(angbanddir)/k_info.raw" && \
	  touch "$(angbanddir)/r_info.raw" && \
	  touch "$(angbanddir)/z_info.raw" && \
	  touch "$(angbanddir)/b_info.raw" && \
	  touch "$(angbanddir)/f_info.raw" && \
	  touch "$(angbanddir)/h_info.raw" && \
	  touch "$(angbanddir)/p_info.raw" && \
	  touch "$(angbanddir)/v_info.raw" && \
	  chown -R root.@GAMEGROUP@ "$(angbanddir)" && \
	  chmod -R g+w "$(angbanddir)" ;\
	fi
endif

