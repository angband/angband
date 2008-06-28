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
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
birth.o: birth.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h cmds.h \
  game-event.h game-cmd.h ui-menu.h
button.o: button.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
cave.o: cave.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h \
  game-event.h
cmd0.o: cmd0.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h wizard.h cmds.h \
  ui-menu.h
cmd1.o: cmd1.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h cmds.h
cmd2.o: cmd2.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
cmd3.o: cmd3.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
cmd4.o: cmd4.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h ui-menu.h
cmd5.o: cmd5.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
cmd6.o: cmd6.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h cmds.h \
  effects.h list-effects.h
cmd-obj.o: cmd-obj.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h \
  cmds.h
death.o: death.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h ui-menu.h cmds.h
debug.o: debug.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h debug.h
dungeon.o: dungeon.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h \
  cmds.h game-event.h
effects.o: effects.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h effects.h \
  list-effects.h
files.o: files.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h ui-menu.h \
  cmds.h
game-cmd.o: game-cmd.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h game-cmd.h
game-event.o: game-event.c z-virt.h h-basic.h game-event.h
generate.o: generate.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
history.o: history.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
init1.o: init1.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h effects.h \
  list-effects.h init.h
init2.o: init2.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h init.h cmds.h \
  game-event.h game-cmd.h
load.o: load.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
melee1.o: melee1.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
melee2.o: melee2.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h bitflag.h \
  tvalsval.h
monster1.o: monster1.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
monster2.o: monster2.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
obj-desc.o: obj-desc.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
obj-info.o: obj-info.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h effects.h \
  list-effects.h cmds.h tvalsval.h
obj-make.o: obj-make.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
obj-ui.o: obj-ui.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
obj-util.o: obj-util.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h randname.h \
  tvalsval.h
option.o: option.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
randart.o: randart.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h \
  init.h randname.h
randname.o: randname.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h randname.h
pathfind.o: pathfind.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
score.o: score.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h
signals.o: signals.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
save.o: save.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h
spells1.o: spells1.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
spells2.o: spells2.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
squelch.o: squelch.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h cmds.h \
  ui-menu.h tvalsval.h
store.o: store.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h cmds.h ui-menu.h \
  game-event.h tvalsval.h
tables.o: tables.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
target.o: target.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h cmds.h
trap.o: trap.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h
ui.o: ui.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h
ui-birth.o: ui-birth.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h ui-menu.h \
  game-event.h game-cmd.h
ui-event.o: ui-event.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
ui-menu.o: ui-menu.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h ui-menu.h
util.o: util.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h randname.h
variable.o: variable.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h
wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h cmds.h \
  tvalsval.h
wiz-stats.o: wiz-stats.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h cmds.h \
  wizard.h tvalsval.h
wizard.o: wizard.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h wizard.h \
  cmds.h tvalsval.h
x-spell.o: x-spell.c angband.h h-basic.h z-file.h z-form.h z-util.h \
  z-virt.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  defines.h option.h types.h ui.h z-type.h object.h externs.h tvalsval.h
xtra1.o: xtra1.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h game-event.h \
  tvalsval.h
xtra2.o: xtra2.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h cmds.h tvalsval.h
xtra3.o: xtra3.c angband.h h-basic.h z-file.h z-form.h z-util.h z-virt.h \
  z-rand.h z-term.h ui-event.h z-quark.h z-msg.h config.h defines.h \
  option.h types.h ui.h z-type.h object.h externs.h game-event.h \
  ui-birth.h tvalsval.h
z-file.o: z-file.c z-file.h h-basic.h z-virt.h z-util.h z-form.h
z-form.o: z-form.c z-form.h h-basic.h z-type.h z-util.h z-virt.h
z-msg.o: z-msg.c z-virt.h h-basic.h z-term.h ui-event.h z-msg.h
z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h
z-rand.o: z-rand.c z-rand.h h-basic.h
z-term.o: z-term.c z-term.h h-basic.h ui-event.h z-virt.h
z-type.o: z-type.c h-basic.h z-form.h z-term.h ui-event.h ui.h z-type.h
z-util.o: z-util.c z-util.h h-basic.h
z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
