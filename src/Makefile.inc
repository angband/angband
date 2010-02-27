#
# File: Makefile.inc
#
# Includes Makefile.src and contains dependency info
#

include Makefile.src

BASEOBJS := $(ANGFILES) $(ZFILES)

depgen:
	head -n 19 Makefile.inc > Makefile.new
	@for i in $(BASEOBJS:.o=.c); do \
		DIR=`echo ./$$i | sed 's|/[^/]*$$||'`; \
			gcc -MM -I. $$i | \
			sed "s|^\\([^ ]\\)|$$DIR/\\1|" >> Makefile.new; \
	done;

# Dependencies
./attack.o: attack.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h game-cmd.h cmds.h
./birth.o: birth.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h cmds.h game-cmd.h \
  game-event.h ui-menu.h
./button.o: button.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./cave.o: cave.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h object/tvalsval.h game-event.h
./cmd0.o: cmd0.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h wizard.h cmds.h game-cmd.h ui-menu.h
./cmd1.o: cmd1.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h object/tvalsval.h cmds.h game-cmd.h
./cmd2.o: cmd2.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h object/tvalsval.h cmds.h game-cmd.h
./cmd3.o: cmd3.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h object/tvalsval.h cmds.h game-cmd.h
./cmd4.o: cmd4.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h object/tvalsval.h cmds.h game-cmd.h ui-menu.h
./cmd5.o: cmd5.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h object/tvalsval.h game-cmd.h
./cmd-know.o: cmd-know.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h ui-menu.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h game-cmd.h cmds.h effects.h \
  list-effects.h
./death.o: death.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h ui-menu.h cmds.h game-cmd.h wizard.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h debug.h
./dungeon.o: dungeon.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h cmds.h game-cmd.h \
  game-event.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h effects.h list-effects.h
./files.o: files.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h ui-menu.h game-cmd.h cmds.h
./game-cmd.o: game-cmd.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h game-cmd.h cmds.h
./game-event.o: game-event.c z-virt.h h-basic.h game-event.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./history.o: history.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./init1.o: init1.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h effects.h list-effects.h \
  monster/constants.h init.h
./init2.o: init2.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h init.h cmds.h game-cmd.h game-event.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h object/tvalsval.h savefile.h
./load-old.o: load-old.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./monster/melee1.o: monster/melee1.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./monster/melee2.o: monster/melee2.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h monster/constants.h
./monster/monster1.o: monster/monster1.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h monster/constants.h
./monster/monster2.o: monster/monster2.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./object/identify.o: object/identify.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h game-event.h \
  list-object-flags.h
./object/obj-desc.o: object/obj-desc.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./object/obj-info.o: object/obj-info.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h effects.h list-effects.h cmds.h game-cmd.h \
  object/tvalsval.h
./object/obj-make.o: object/obj-make.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./object/obj-power.o: object/obj-power.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h init.h effects.h \
  list-effects.h
./object/obj-ui.o: object/obj-ui.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./object/obj-util.o: object/obj-util.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h defines.h object/tvalsval.h effects.h \
  list-effects.h game-cmd.h
./object/randart.o: object/randart.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h init.h effects.h \
  list-effects.h list-object-flags.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./pathfind.o: pathfind.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./prefs.o: prefs.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./player/calcs.o: player/calcs.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h game-event.h object/tvalsval.h
./player/timed.o: player/timed.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./player/util.o: player/util.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./signals.o: signals.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h savefile.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h savefile.h
./spells1.o: spells1.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h monster/constants.h
./spells2.o: spells2.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./squelch.o: squelch.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h cmds.h game-cmd.h ui-menu.h object/tvalsval.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h cmds.h game-cmd.h ui-menu.h game-event.h \
  object/tvalsval.h
./tables.o: tables.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h object/tvalsval.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h game-cmd.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h
./ui.o: ui.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h ui-menu.h ui-birth.h game-event.h game-cmd.h \
  cmds.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h ui-menu.h
./util.o: util.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h z-form.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h object/constants.h object/types.h object/object.h \
  monster/types.h player/types.h player/player.h store.h ui.h z-type.h \
  externs.h game-event.h
./variable.o: variable.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h wizard.h cmds.h game-cmd.h object/tvalsval.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-virt.h \
  z-file.h z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h cmds.h game-cmd.h wizard.h object/tvalsval.h
./wizard.o: wizard.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h wizard.h cmds.h game-cmd.h object/tvalsval.h
./x-spell.o: x-spell.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h effects.h list-effects.h object/tvalsval.h
./xtra2.o: xtra2.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h cmds.h game-cmd.h object/tvalsval.h
./xtra3.o: xtra3.c angband.h h-basic.h z-bitflag.h z-virt.h z-file.h \
  z-form.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h object/constants.h object/types.h \
  object/object.h monster/types.h player/types.h player/player.h store.h \
  ui.h z-type.h externs.h game-event.h game-cmd.h textui.h ui-birth.h \
  object/tvalsval.h
./z-bitflag.o: z-bitflag.c z-bitflag.h h-basic.h z-virt.h
./z-file.o: z-file.c z-file.h h-basic.h z-virt.h z-util.h z-form.h
./z-form.o: z-form.c z-form.h h-basic.h z-type.h z-util.h z-virt.h
./z-msg.o: z-msg.c z-virt.h h-basic.h z-term.h ui-event.h z-msg.h
./z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h
./z-rand.o: z-rand.c z-rand.h h-basic.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h
./z-term.o: z-term.c z-term.h h-basic.h ui-event.h z-virt.h
./z-type.o: z-type.c h-basic.h z-form.h z-term.h ui-event.h ui.h z-type.h
./z-util.o: z-util.c z-util.h h-basic.h
./z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
