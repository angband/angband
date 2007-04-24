HEADERS = \
	h-basic.h  \
	h-config.h \
	angband.h  \
	config.h   \
	defines.h  \
	types.h    \
	externs.h  \
	z-term.h   \
	z-rand.h   \
	z-util.h   \
	z-virt.h   \
	z-form.h

SOURCES = \
	birth.c    \
	cave.c     \
	cmd1.c     \
	cmd2.c     \
	cmd3.c     \
	cmd4.c     \
	cmd5.c     \
	cmd6.c     \
	dungeon.c  \
	files.c    \
	generate.c \
	init1.c    \
	init2.c    \
	load.c     \
	maid-x11.c \
	main-cap.c \
	main-crb.c \
	main-dos.c \
	main-gcu.c \
	main-gtk.c \
	main-ibm.c \
	main-lsl.c \
	main-ros.c \
	main-sla.c \
	main-vcs.c \
	main-win.c \
	main-x11.c \
	main-xaw.c \
	main-xpj.c \
	main-xxx.c \
	main.c     \
	melee1.c   \
	melee2.c   \
	monster1.c \
	monster2.c \
	obj-info.c \
	object1.c  \
	object2.c  \
	randart.c  \
	randname.c \
	pathfind.c \
	save.c     \
	spells1.c  \
	spells2.c  \
	squelch.c  \
	store.c    \
	tables.c   \
	ui.c       \
	use-obj.c  \
	util.c     \
	variable.c \
	wizard1.c  \
	wizard2.c  \
	x-spell.c  \
	xtra1.c    \
	xtra2.c    \
	z-file.c   \
	z-form.c   \
	z-rand.c   \
	z-term.c   \
	z-util.c   \
	z-virt.c

OBJECTS = ${SOURCES:.c=.o}

inc-print:
	@echo ${OBJECTS}
