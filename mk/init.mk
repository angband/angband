# Shut up GNU make
.SILENT:

OBJECTIVE_DIRECTORIES = 
OBJECTIVE_LIBS = 
OBJECTIVE_LIBS_NOINST = 
OBJECTIVE_BINS = 
OBJECTIVE_DATA = 
SUBDIRS = 
HEADERS = 
V = 0
VERBOSE ?= $(V)
VERBOSITY = 0
SHOW_CFLAGS ?= $(VERBOSE)

LIBDIR = $(libdir)
BINDIR = $(bindir)
INCLUDEDIR = $(pkgincludedir)
CFLAGS += -DHAVE_CONFIG_H -I/usr/pkg/include -I/usr/pkg/xorg/include
CXXFLAGS += -DHAVE_CONFIG_H -I/usr/pkg/include -I/usr/pkg/xorg/include
