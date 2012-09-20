## Makefile.am -- Process this file with automake to process Makefile.in

angband_files = \
  a_info.txt b_info.txt e_info.txt f_info.txt \
  g_info.txt h_info.txt k_info.txt p_info.txt \
  r_info.txt v_info.txt z_info.txt

EXTRA_DIST = \
  $(angband_files)

if SET_GID
angbanddir = @DEFAULT_PATH@edit

angband_DATA = \
  $(angband_files)
endif

