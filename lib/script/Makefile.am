## Makefile.am -- Process this file with automake to process Makefile.in

angband_files = \
  init.lua object.lua spell.lua test.lua trace.lua

EXTRA_DIST = \
  $(angband_files)

if SET_GID
angbanddir = @DEFAULT_PATH@script

angband_DATA = \
  $(angband_files)
endif

