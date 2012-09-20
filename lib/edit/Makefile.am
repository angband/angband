## Makefile.am -- Process this file with automake to process Makefile.in

angband_files = \
  artifact.txt \
  cost_adj.txt \
  ego_item.txt \
  limits.txt \
  monster.txt \
  object.txt \
  p_class.txt \
  p_hist.txt \
  p_race.txt \
  shop_own.txt \
  terrain.txt \
  vault.txt

EXTRA_DIST = \
  $(angband_files)

if SET_GID
angbanddir = @DEFAULT_PATH@edit

angband_DATA = \
  $(angband_files)
endif

