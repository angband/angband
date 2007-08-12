#
# File: Makefile.inc
#
# Includes Makefile.src and contains dependency info
#

include Makefile.src

BASEOBJS := $(ANGFILES) $(ZFILES)

depgen:
	head -n 15 Makefile.inc > Makefile.new
	gcc -MM $(BASEOBJS:.o=.c) >> Makefile.new

# Dependencies
attack.o: attack.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
birth.o: birth.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h cmds.h script.h
cave.o: cave.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
cmd0.o: cmd0.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h cmds.h
cmd1.o: cmd1.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h cmds.h
cmd2.o: cmd2.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
cmd3.o: cmd3.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
cmd4.o: cmd4.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
cmd5.o: cmd5.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h script.h
cmd6.o: cmd6.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h effects.h
dungeon.o: dungeon.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h cmds.h script.h
effects.o: effects.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h effects.h
files.o: files.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h cmds.h
generate.o: generate.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
init1.o: init1.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h effects.h init.h
init2.o: init2.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h init.h script.h cmds.h
load.o: load.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
loadsave.o: loadsave.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h z-blockfile.h z-smap.h
log.o: log.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h log.h
maid-x11.o: maid-x11.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
main.o: main.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h main.h
main-crb.o: main-crb.c
main-gcu.o: main-gcu.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
main-gtk.o: main-gtk.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
main-ros.o: main-ros.c
main-sdl.o: main-sdl.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h cmds.h
main-win.o: main-win.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
main-x11.o: main-x11.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
main-xaw.o: main-xaw.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
main-xxx.o: main-xxx.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
melee1.o: melee1.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
melee2.o: melee2.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
monster1.o: monster1.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
monster2.o: monster2.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
object1.o: object1.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h randname.h
object2.o: object2.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
obj-info.o: obj-info.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h effects.h cmds.h
option.o: option.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
pathfind.o: pathfind.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
randart.o: randart.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h init.h randname.h
randname.o: randname.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h randname.h
save.o: save.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
score.o: score.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
signals.o: signals.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
snd-sdl.o: snd-sdl.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
spells1.o: spells1.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h script.h
spells2.o: spells2.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
squelch.o: squelch.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h cmds.h
store.o: store.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h script.h cmds.h
tables.o: tables.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
trap.o: trap.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
ui.o: ui.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h
util.o: util.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h randname.h
variable.o: variable.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h
wizard1.o: wizard1.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h cmds.h
wizard2.o: wizard2.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h cmds.h script.h
x-spell.o: x-spell.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h externs.h script.h
xtra1.o: xtra1.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h script.h
xtra2.o: xtra2.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h z-quark.h z-msg.h config.h defines.h option.h types.h \
  ui.h z-type.h externs.h cmds.h
z-blockfile.o: z-blockfile.c z-virt.h h-basic.h z-blockfile.h z-file.h
z-file.o: z-file.c z-file.h h-basic.h z-virt.h z-util.h z-form.h
z-form.o: z-form.c z-form.h h-basic.h z-type.h z-util.h z-virt.h
z-msg.o: z-msg.c z-virt.h h-basic.h z-term.h z-msg.h
z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h
z-rand.o: z-rand.c z-rand.h h-basic.h
z-smap.o: z-smap.c z-virt.h h-basic.h z-file.h z-smap.h
z-term.o: z-term.c z-term.h h-basic.h z-virt.h
z-type.o: z-type.c h-basic.h z-form.h z-term.h ui.h z-type.h
z-util.o: z-util.c z-util.h h-basic.h
z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
