## Makefile.am -- Process this file with automake to process Makefile.in

noinst_LIBRARIES = liblua.a

noinst_PROGRAMS = tolua

liblua_a_SOURCES = \
  lapi.c ldebug.c lmem.c lstrlib.c lvm.c \
  tolua_lb.c lauxlib.c ldo.c lobject.c ltable.c \
  lzio.c tolua_rg.c lbaselib.c lfunc.c lparser.c \
  ltests.c tolua_bd.c tolua_tm.c lcode.c lgc.c \
  lstate.c ltm.c tolua_eh.c tolua_tt.c ldblib.c \
  llex.c lstring.c lundump.c tolua_gp.c \
  lapi.h lauxlib.h lcode.h ldebug.h ldo.h lfunc.h \
  lgc.h llex.h llimits.h lmem.h lobject.h lopcodes.h \
  lparser.h lstate.h lstring.h ltable.h ltm.h lua.h \
  luadebug.h lualib.h lundump.h lvm.h lzio.h \
  tolua.h tolua_eh.h tolua_rg.h tolua_tm.h \
  tolua_tt.h

tolua_SOURCES = \
  tolua.c tolualua.c liolib.c

EXTRA_tolua_SOURCES = \
  array.lua basic.lua class.lua clean.lua code.lua \
  container.lua declaration.lua define.lua doit.lua \
  enumerate.lua feature.lua function.lua lua2c.lua \
  module.lua operator.lua package.lua typedef.lua \
  variable.lua verbatim.lua

tolua_LDADD = liblua.a
