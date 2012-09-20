## Makefile.am -- Process this file with automake to process Makefile.in

noinst_LIBRARIES = liblua.a

noinst_PROGRAMS = tolua

liblua_a_SOURCES = \
  lapi.c ldebug.c lmem.c lstrlib.c lvm.c \
  lauxlib.c ldo.c lobject.c ltable.c lzio.c \
  lbaselib.c lfunc.c lparser.c lcode.c lgc.c \
  lopcodes.c lstate.c ltm.c  ldblib.c llex.c \
  lstring.c ldump.c lundump.c ltablib.c lapi.h \
  lauxlib.h lcode.h ldebug.h ldo.h lfunc.h \
  lgc.h llex.h llimits.h lmem.h lobject.h \
  lopcodes.h lparser.h lstate.h lstring.h ltable.h \
  ltm.h lua.h lualib.h lundump.h lvm.h \
  lzio.h \
  tolua_map.c tolua_is.c tolua_to.c tolua_push.c tolua_event.c \
  tolua.h tolua_event.h

tolua_SOURCES = \
  tolua.c liolib.c toluabind.c

EXTRA_tolua_SOURCES = \
  array.lua basic.lua class.lua clean.lua code.lua \
  container.lua declaration.lua define.lua doit.lua \
  enumerate.lua feature.lua function.lua \
  module.lua operator.lua package.lua typedef.lua \
  variable.lua verbatim.lua

tolua_LDADD = liblua.a
