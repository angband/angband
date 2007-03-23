## Makefile.am -- Process this file with automake to process Makefile.in

angband_files = \
  help.hlp attack.txt birth.txt command.txt \
  dungeon.txt general.txt option.txt playing.txt \
  version.txt

EXTRA_DIST = \
  $(angband_files)

if SET_GID
angbanddir = @DEFAULT_PATH@help

angband_DATA = \
  $(angband_files)
endif

