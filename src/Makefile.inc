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
./attack.o: attack.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  cmds.h monster/monster.h monster/types.h object/tvalsval.h spells.h \
  target.h
./birth.o: birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cmds.h \
  files.h game-event.h history.h object/inventory.h object/tvalsval.h \
  squelch.h ui-menu.h
./button.o: button.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  button.h
./cave.o: cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h game-event.h \
  object/tvalsval.h squelch.h
./cmd0.o: cmd0.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h cave.h cmds.h files.h \
  macro.h monster/monster.h monster/types.h ui-menu.h wizard.h
./cmd1.o: cmd1.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h attack.h cave.h cmds.h \
  generate.h history.h monster/monster.h monster/types.h \
  object/inventory.h object/tvalsval.h squelch.h trap.h
./cmd2.o: cmd2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h attack.h cave.h cmds.h \
  files.h generate.h monster/monster.h monster/types.h object/tvalsval.h \
  spells.h squelch.h trap.h
./cmd3.o: cmd3.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h button.h cave.h cmds.h \
  monster/monster.h monster/types.h object/inventory.h object/tvalsval.h \
  squelch.h target.h
./cmd4.o: cmd4.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h cave.h cmds.h files.h \
  history.h macro.h object/tvalsval.h prefs.h squelch.h ui-menu.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  cmds.h monster/monster.h monster/types.h wizard.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  cmds.h effects.h list-effects.h object/tvalsval.h spells.h target.h
./death.o: death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cmds.h \
  files.h history.h ui-menu.h wizard.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h debug.h
./dungeon.o: dungeon.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  button.h cave.h cmds.h files.h game-event.h generate.h init.h \
  monster/monster.h monster/types.h object/tvalsval.h prefs.h spells.h \
  target.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  effects.h list-effects.h monster/monster.h monster/types.h spells.h
./files.o: files.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  cmds.h history.h object/tvalsval.h ui-menu.h
./game-cmd.o: game-cmd.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cmds.h \
  spells.h target.h
./game-event.o: game-event.c z-virt.h h-basic.h game-event.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  files.h monster/monster.h monster/types.h object/tvalsval.h trap.h
./history.o: history.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  history.h
./init2.o: init2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  button.h cave.h cmds.h game-event.h init.h macro.h monster/constants.h \
  object/tvalsval.h prefs.h squelch.h list-effects.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h cave.h history.h \
  monster/monster.h monster/types.h object/tvalsval.h savefile.h \
  squelch.h
./macro.o: macro.c defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h macro.h h-basic.h z-util.h z-virt.h
./monster/melee1.o: monster/melee1.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h attack.h cave.h monster/monster.h \
  monster/types.h object/tvalsval.h spells.h
./monster/melee2.o: monster/melee2.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h cave.h monster/constants.h \
  monster/monster.h monster/types.h object/tvalsval.h spells.h squelch.h
./monster/monster1.o: monster/monster1.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h monster/constants.h \
  monster/monster.h monster/types.h object/tvalsval.h
./monster/monster2.o: monster/monster2.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h cave.h generate.h history.h \
  object/tvalsval.h object/object.h target.h
./object/identify.o: object/identify.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h cave.h game-event.h history.h \
  object/tvalsval.h spells.h squelch.h list-object-flags.h
./object/obj-desc.o: object/obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h squelch.h object/tvalsval.h
./object/obj-info.o: object/obj-info.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h attack.h effects.h list-effects.h \
  cmds.h object/tvalsval.h z-textblock.h
./object/obj-make.o: object/obj-make.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h object/tvalsval.h
./object/obj-power.o: object/obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h object/tvalsval.h init.h \
  effects.h list-effects.h
./object/obj-ui.o: object/obj-ui.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h button.h object/tvalsval.h cmds.h
./object/obj-util.o: object/obj-util.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h cave.h defines.h effects.h \
  list-effects.h game-cmd.h generate.h history.h object/inventory.h \
  prefs.h spells.h squelch.h object/tvalsval.h
./object/randart.o: object/randart.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h object/tvalsval.h init.h \
  effects.h list-effects.h list-object-flags.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h
./parser.o: parser.c parser.h h-basic.h z-rand.h defines.h \
  list-blow-methods.h list-blow-effects.h list-object-flags.h \
  list-player-flags.h list-mon-flags.h list-mon-spells.h z-util.h \
  z-virt.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h
./pathfind.o: pathfind.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  squelch.h
./prefs.o: prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h macro.h \
  prefs.h squelch.h
./player/calcs.o: player/calcs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h cave.h files.h game-event.h \
  monster/monster.h monster/types.h object/tvalsval.h spells.h squelch.h
./player/player.o: player/player.c player/player.h player/types.h object/types.h \
  z-bitflag.h h-basic.h z-form.h z-virt.h z-quark.h z-rand.h defines.h \
  list-blow-methods.h list-blow-effects.h list-object-flags.h \
  list-player-flags.h list-mon-flags.h list-mon-spells.h option.h \
  ui-event.h
./player/spell.o: player/spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h cave.h object/tvalsval.h \
  game-cmd.h spells.h
./player/timed.o: player/timed.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h cave.h
./player/util.o: player/util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h monster/types.h player/types.h object/types.h option.h \
  ui-event.h player/player.h player/types.h store.h parser.h x-char.h \
  ui.h z-textblock.h z-type.h externs.h cave.h object/tvalsval.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h
./signals.o: signals.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h files.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h history.h \
  monster/monster.h monster/types.h savefile.h squelch.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  savefile.h
./spells1.o: spells1.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  generate.h object/tvalsval.h monster/constants.h monster/monster.h \
  monster/types.h squelch.h trap.h
./spells2.o: spells2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  generate.h history.h monster/monster.h monster/types.h \
  object/tvalsval.h spells.h squelch.h target.h trap.h
./squelch.o: squelch.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cmds.h \
  ui-menu.h object/tvalsval.h squelch.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  cmds.h game-event.h history.h init.h object/inventory.h \
  object/tvalsval.h spells.h squelch.h target.h textui.h ui-menu.h \
  z-debug.h
./tables.o: tables.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  object/tvalsval.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  monster/monster.h monster/types.h squelch.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h attack.h cave.h \
  monster/monster.h monster/types.h spells.h
./ui.o: ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  button.h cmds.h files.h game-event.h ui-birth.h ui-menu.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  history.h monster/monster.h monster/types.h object/tvalsval.h squelch.h \
  ui-menu.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  ui-menu.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cmds.h \
  macro.h squelch.h prefs.h object/tvalsval.h ui-menu.h files.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  object/tvalsval.h spells.h ui-menu.h
./util.o: util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-file.h \
  z-util.h z-rand.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-term.h ui-event.h z-quark.h z-msg.h config.h \
  option.h types.h game-cmd.h object/constants.h object/types.h \
  z-bitflag.h z-quark.h z-rand.h object/object.h angband.h z-textblock.h \
  z-file.h defines.h monster/types.h player/types.h object/types.h \
  option.h ui-event.h player/player.h player/types.h store.h parser.h \
  x-char.h ui.h z-textblock.h z-type.h externs.h button.h game-event.h \
  macro.h cmds.h
./variable.o: variable.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cmds.h \
  monster/monster.h monster/types.h object/tvalsval.h ui-menu.h wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cmds.h \
  wizard.h object/tvalsval.h
./wizard.o: wizard.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  cmds.h files.h monster/monster.h monster/types.h object/tvalsval.h \
  ui-menu.h spells.h target.h wizard.h
./x-char.o: x-char.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h
./x-spell.o: x-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cave.h \
  effects.h list-effects.h object/tvalsval.h spells.h
./xtra2.o: xtra2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h cmds.h \
  history.h macro.h object/tvalsval.h target.h
./xtra3.o: xtra3.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h \
  button.h cave.h files.h game-event.h monster/monster.h monster/types.h \
  object/tvalsval.h textui.h ui-birth.h
./z-bitflag.o: z-bitflag.c z-bitflag.h h-basic.h z-form.h z-virt.h
./z-file.o: z-file.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h
./z-form.o: z-form.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h
./z-msg.o: z-msg.c z-virt.h h-basic.h z-term.h ui-event.h z-msg.h
./z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h
./z-rand.o: z-rand.c z-rand.h h-basic.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h
./z-term.o: z-term.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  z-file.h z-util.h z-rand.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-term.h ui-event.h z-quark.h \
  z-msg.h config.h option.h types.h game-cmd.h object/constants.h \
  object/types.h z-bitflag.h z-quark.h z-rand.h object/object.h angband.h \
  z-textblock.h z-file.h defines.h monster/types.h player/types.h \
  object/types.h option.h ui-event.h player/player.h player/types.h \
  store.h parser.h x-char.h ui.h z-textblock.h z-type.h externs.h
./z-type.o: z-type.c h-basic.h z-form.h z-term.h ui-event.h ui.h \
  z-textblock.h z-file.h defines.h list-blow-methods.h \
  list-blow-effects.h list-object-flags.h list-player-flags.h \
  list-mon-flags.h list-mon-spells.h z-type.h
./z-util.o: z-util.c z-util.h h-basic.h
./z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
./z-textblock.o: z-textblock.c z-term.h h-basic.h ui-event.h z-textblock.h \
  z-file.h defines.h list-blow-methods.h list-blow-effects.h \
  list-object-flags.h list-player-flags.h list-mon-flags.h \
  list-mon-spells.h z-virt.h z-form.h
