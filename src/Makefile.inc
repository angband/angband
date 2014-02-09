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
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h attack.h cmds.h monster/mon-make.h \
  angband.h monster/mon-msg.h monster/mon-timed.h monster/mon-util.h \
  object/slays.h object/list-slays.h object/tvalsval.h target.h
./birth.o: birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h birth.h cmds.h files.h game-event.h \
  history.h object/inventory.h object/tvalsval.h squelch.h quest.h \
  ui-menu.h
./cave.o: cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h game-event.h monster/mon-util.h \
  angband.h object/tvalsval.h squelch.h cmds.h grafmode.h
./cmd-cave.o: cmd-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h attack.h cmds.h dungeon.h files.h \
  game-event.h generate.h monster/mon-timed.h monster/mon-util.h \
  angband.h object/tvalsval.h pathfind.h squelch.h trap.h
./cmd-context.o: cmd-context.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h files.h keymap.h textui.h \
  ui-menu.h wizard.h target.h squelch.h object/tvalsval.h \
  monster/mon-lore.h angband.h monster/mon-util.h
./cmd-know.o: cmd-know.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h files.h history.h \
  object/tvalsval.h monster/mon-lore.h angband.h monster/mon-list.h \
  monster/mon-util.h object/obj-list.h angband.h prefs.h squelch.h \
  target.h ui-menu.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h monster/mon-util.h angband.h \
  wizard.h target.h prefs.h files.h buildid.h history.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h attack.h cmds.h effects.h \
  list-effects.h object/inventory.h object/tvalsval.h squelch.h target.h \
  ui-menu.h ui-options.h
./cmd-pickup.o: cmd-pickup.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h attack.h cmds.h game-event.h \
  generate.h history.h monster/mon-timed.h monster/mon-util.h angband.h \
  object/inventory.h object/tvalsval.h squelch.h trap.h
./cmd-process.o: cmd-process.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h attack.h cmds.h files.h keymap.h \
  textui.h ui-menu.h wizard.h target.h
./death.o: death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h death.h files.h history.h \
  savefile.h ui-menu.h wizard.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h debug.h
./dungeon.o: dungeon.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h birth.h borg/borg1.h angband.h \
  object/tvalsval.h cave.h borg/../effects.h borg/../list-effects.h \
  cmds.h dungeon.h files.h game-event.h generate.h grafmode.h \
  monster/mon-make.h angband.h monster/mon-spell.h \
  monster/list-spell-effects.h monster/mon-util.h object/tvalsval.h \
  pathfind.h prefs.h savefile.h target.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h effects.h list-effects.h dungeon.h \
  monster/mon-spell.h angband.h monster/list-spell-effects.h \
  monster/mon-util.h trap.h
./files.o: files.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h buildid.h cmds.h death.h files.h \
  history.h object/tvalsval.h object/pval.h angband.h savefile.h \
  ui-menu.h
./game-cmd.o: game-cmd.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h attack.h cmds.h object/tvalsval.h \
  target.h
./game-event.o: game-event.c z-virt.h h-basic.h game-event.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h files.h game-event.h generate.h \
  monster/mon-make.h angband.h monster/mon-spell.h \
  monster/list-spell-effects.h object/tvalsval.h trap.h z-queue.h \
  h-basic.h
./grafmode.o: grafmode.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h grafmode.h
./guid.o: guid.c guid.h
./history.o: history.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h history.h
./init.o: init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h buildid.h cmds.h game-event.h \
  generate.h history.h hint.h keymap.h monster/mon-init.h \
  monster/mon-list.h angband.h monster/mon-msg.h monster/mon-util.h \
  object/obj-list.h angband.h object/slays.h object/list-slays.h \
  object/tvalsval.h prefs.h quest.h randname.h squelch.h \
  object/list-object-flags.h list-effects.h list-player-flags.h
./keymap.o: keymap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h keymap.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h dungeon.h history.h \
  monster/mon-make.h angband.h monster/mon-spell.h \
  monster/list-spell-effects.h object/tvalsval.h savefile.h squelch.h \
  quest.h
./monster/mon-init.o: monster/mon-init.c externs.h monster/constants.h \
  monster/list-blow-methods.h monster/list-blow-effects.h \
  monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
  z-bitflag.h h-basic.h z-form.h z-virt.h defines.h z-rand.h cave.h \
  z-type.h monster/mon-timed.h object/obj-flag.h z-rand.h z-file.h \
  z-textblock.h z-file.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/list-object-flags.h monster/list-mon-spells.h object/object.h \
  object/obj-flag.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h object/obj-flag.h \
  parser.h z-bitflag.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
  spells.h list-gf-types.h monster/mon-init.h monster/mon-msg.h angband.h \
  z-util.h z-quark.h config.h cave.h game-cmd.h init.h option.h ui.h \
  z-textblock.h externs.h monster/mon-power.h monster/monster.h \
  monster/mon-spell.h monster/list-spell-effects.h monster/mon-util.h \
  monster/monster.h parser.h z-util.h z-virt.h monster/list-mon-flags.h \
  monster/list-mon-spells.h monster/list-blow-methods.h \
  monster/list-blow-effects.h
./monster/melee1.o: monster/melee1.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h attack.h cave.h monster/monster.h object/tvalsval.h \
  spells.h
./monster/melee2.o: monster/melee2.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h attack.h cave.h monster/monster.h monster/mon-make.h \
  angband.h monster/mon-spell.h monster/list-spell-effects.h \
  monster/mon-timed.h monster/mon-util.h object/slays.h \
  object/list-slays.h object/tvalsval.h spells.h squelch.h
./monster/mon-list.o: monster/mon-list.c monster/mon-util.h angband.h h-basic.h \
  z-bitflag.h z-form.h z-virt.h defines.h z-file.h z-util.h z-rand.h \
  z-term.h ui-event.h z-quark.h z-msg.h z-msg-list.h config.h cave.h \
  z-type.h game-cmd.h init.h parser.h monster/constants.h \
  monster/list-blow-methods.h monster/list-blow-effects.h \
  monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
  z-bitflag.h z-rand.h cave.h monster/mon-timed.h object/obj-flag.h \
  z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/list-object-flags.h monster/list-mon-spells.h object/obj-flag.h \
  object/object.h object/obj-flag.h option.h player/player.h guid.h \
  object/obj-flag.h object/object.h option.h list-player-flags.h store.h \
  ui.h z-textblock.h externs.h spells.h list-gf-types.h \
  monster/mon-list.h
./monster/mon-lore.o: monster/mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h monster/mon-lore.h angband.h monster/mon-make.h \
  monster/mon-spell.h monster/list-spell-effects.h monster/mon-util.h \
  object/tvalsval.h attack.h monster/list-mon-flags.h \
  monster/list-blow-methods.h monster/list-blow-effects.h
./monster/mon-make.o: monster/mon-make.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h alloc.h history.h init.h target.h monster/mon-lore.h \
  angband.h monster/mon-make.h monster/mon-timed.h monster/mon-util.h \
  object/tvalsval.h quest.h
./monster/mon-msg.o: monster/mon-msg.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h monster/mon-msg.h angband.h monster/mon-util.h
./monster/mon-power.o: monster/mon-power.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h monster/mon-power.h monster/monster.h \
  monster/mon-spell.h angband.h monster/list-spell-effects.h
./monster/mon-spell.o: monster/mon-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h spells.h effects.h list-effects.h monster/mon-spell.h \
  angband.h monster/list-spell-effects.h monster/mon-timed.h \
  monster/mon-util.h monster/list-mon-spells.h \
  monster/list-spell-effects.h
./monster/mon-timed.o: monster/mon-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h monster/mon-msg.h angband.h monster/mon-spell.h \
  monster/list-spell-effects.h monster/mon-timed.h monster/mon-util.h
./monster/mon-util.o: monster/mon-util.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h monster/mon-make.h angband.h monster/mon-msg.h \
  monster/mon-spell.h monster/list-spell-effects.h monster/mon-timed.h \
  monster/mon-list.h monster/mon-util.h squelch.h
./object/chest.o: object/chest.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h monster/mon-util.h angband.h
./object/identify.o: object/identify.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h cave.h game-event.h history.h object/slays.h \
  object/list-slays.h object/tvalsval.h object/pval.h angband.h spells.h \
  squelch.h
./object/obj-desc.o: object/obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h squelch.h object/tvalsval.h object/pval.h angband.h
./object/obj-flag.o: object/obj-flag.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/list-object-flags.h
./object/obj-info.o: object/obj-info.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h attack.h effects.h list-effects.h cmds.h \
  object/tvalsval.h z-textblock.h object/slays.h object/list-slays.h \
  object/pval.h angband.h
./object/obj-list.o: object/obj-list.c object/object.h z-rand.h h-basic.h \
  defines.h z-file.h z-textblock.h z-file.h z-quark.h z-bitflag.h \
  z-form.h z-virt.h game-cmd.h cave.h z-type.h object/obj-flag.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/list-object-flags.h object/tvalsval.h squelch.h \
  object/obj-list.h angband.h z-bitflag.h z-util.h z-rand.h z-term.h \
  ui-event.h z-quark.h z-msg.h z-msg-list.h config.h cave.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h monster/list-mon-spells.h object/obj-flag.h \
  object/object.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h
./object/obj-make.o: object/obj-make.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h alloc.h cave.h init.h object/tvalsval.h object/pval.h \
  angband.h object/slays.h object/list-slays.h
./object/obj-power.o: object/obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/slays.h object/list-slays.h object/tvalsval.h \
  object/pval.h angband.h init.h effects.h list-effects.h \
  monster/mon-power.h monster/monster.h
./object/obj-ui.o: object/obj-ui.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cmds.h game-cmd.h keymap.h
./object/obj-util.o: object/obj-util.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h cave.h effects.h list-effects.h game-cmd.h generate.h \
  history.h monster/mon-make.h angband.h object/inventory.h \
  object/tvalsval.h prefs.h randname.h spells.h squelch.h z-queue.h \
  h-basic.h grafmode.h
./object/pval.o: object/pval.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/pval.h angband.h object/tvalsval.h
./object/randart.o: object/randart.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/slays.h object/list-slays.h object/tvalsval.h \
  object/pval.h angband.h init.h effects.h list-effects.h randname.h
./object/slays.o: object/slays.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/slays.h object/list-slays.h monster/mon-util.h \
  angband.h object/list-slays.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h
./parser.o: parser.c externs.h monster/constants.h \
  monster/list-blow-methods.h monster/list-blow-effects.h \
  monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
  z-bitflag.h h-basic.h z-form.h z-virt.h defines.h z-rand.h cave.h \
  z-type.h monster/mon-timed.h object/obj-flag.h z-rand.h z-file.h \
  z-textblock.h z-file.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/list-object-flags.h monster/list-mon-spells.h object/object.h \
  z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h object/obj-flag.h \
  parser.h h-basic.h z-bitflag.h z-rand.h z-term.h ui-event.h z-file.h \
  z-msg.h z-msg-list.h spells.h list-gf-types.h z-form.h z-util.h \
  z-virt.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h randname.h
./pathfind.o: pathfind.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h pathfind.h squelch.h \
  monster/mon-util.h angband.h
./prefs.o: prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h keymap.h prefs.h squelch.h
./player/calcs.o: player/calcs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h cave.h files.h game-event.h monster/mon-msg.h angband.h \
  monster/mon-util.h object/tvalsval.h object/pval.h angband.h spells.h \
  squelch.h
./player/class.o: player/class.c externs.h monster/constants.h \
  monster/list-blow-methods.h monster/list-blow-effects.h \
  monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
  z-bitflag.h h-basic.h z-form.h z-virt.h defines.h z-rand.h cave.h \
  z-type.h monster/mon-timed.h object/obj-flag.h z-rand.h z-file.h \
  z-textblock.h z-file.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/list-object-flags.h monster/list-mon-spells.h object/object.h \
  object/obj-flag.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h object/obj-flag.h \
  parser.h z-bitflag.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
  spells.h list-gf-types.h player/player.h
./player/player.o: player/player.c externs.h monster/constants.h \
  monster/list-blow-methods.h monster/list-blow-effects.h \
  monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
  z-bitflag.h h-basic.h z-form.h z-virt.h defines.h z-rand.h cave.h \
  z-type.h monster/mon-timed.h object/obj-flag.h z-rand.h z-file.h \
  z-textblock.h z-file.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/list-object-flags.h monster/list-mon-spells.h object/object.h \
  object/obj-flag.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h object/obj-flag.h \
  parser.h z-bitflag.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
  spells.h list-gf-types.h history.h player/player.h birth.h z-util.h
./player/race.o: player/race.c externs.h monster/constants.h \
  monster/list-blow-methods.h monster/list-blow-effects.h \
  monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
  z-bitflag.h h-basic.h z-form.h z-virt.h defines.h z-rand.h cave.h \
  z-type.h monster/mon-timed.h object/obj-flag.h z-rand.h z-file.h \
  z-textblock.h z-file.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/list-object-flags.h monster/list-mon-spells.h object/object.h \
  object/obj-flag.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h object/obj-flag.h \
  parser.h z-bitflag.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
  spells.h list-gf-types.h player/player.h
./player/spell.o: player/spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h cave.h object/tvalsval.h game-cmd.h spells.h
./player/timed.o: player/timed.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h cave.h
./player/p-util.o: player/p-util.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h cave.h object/tvalsval.h
./quest.o: quest.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h quest.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h buildid.h
./signals.o: signals.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h files.h savefile.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h dungeon.h history.h \
  monster/mon-make.h angband.h quest.h savefile.h squelch.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h savefile.h
./spells1.o: spells1.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h dungeon.h generate.h grafmode.h \
  monster/mon-make.h angband.h monster/mon-msg.h monster/mon-spell.h \
  monster/list-spell-effects.h monster/mon-timed.h monster/mon-util.h \
  object/tvalsval.h squelch.h trap.h
./spells2.o: spells2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h generate.h history.h \
  monster/mon-lore.h angband.h monster/mon-make.h monster/mon-timed.h \
  monster/mon-util.h object/slays.h object/list-slays.h object/tvalsval.h \
  squelch.h target.h trap.h
./squelch.o: squelch.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h ui-menu.h object/tvalsval.h \
  object/pval.h angband.h squelch.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h game-event.h history.h \
  object/inventory.h object/tvalsval.h squelch.h target.h textui.h \
  ui-menu.h z-debug.h
./tables.o: tables.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h object/tvalsval.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h monster/mon-lore.h angband.h \
  monster/mon-util.h squelch.h target.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h attack.h effects.h list-effects.h \
  trap.h
./ui.o: ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h defines.h \
  z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h z-msg.h \
  z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h parser.h \
  monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h files.h game-event.h \
  object/tvalsval.h ui-birth.h ui-menu.h ui-options.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h history.h monster/mon-lore.h \
  angband.h monster/mon-util.h object/tvalsval.h squelch.h ui-menu.h \
  ui-options.h grafmode.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h ui-menu.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h keymap.h squelch.h prefs.h \
  object/tvalsval.h ui-menu.h ui-options.h files.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h object/tvalsval.h ui-menu.h
./util.o: util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h target.h files.h game-event.h \
  randname.h
./variable.o: variable.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h buildid.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h buildid.h cmds.h monster/mon-lore.h \
  angband.h object/tvalsval.h ui-menu.h wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
  z-virt.h defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
  z-quark.h z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h \
  init.h parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h wizard.h monster/mon-make.h \
  angband.h object/tvalsval.h effects.h list-effects.h generate.h
./wizard.o: wizard.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h files.h monster/mon-lore.h \
  angband.h monster/mon-make.h monster/mon-util.h object/tvalsval.h \
  ui-menu.h target.h wizard.h
./x-spell.o: x-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h effects.h list-effects.h \
  object/tvalsval.h
./xtra2.o: xtra2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h cmds.h keymap.h history.h \
  object/tvalsval.h pathfind.h target.h
./xtra3.o: xtra3.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h buildid.h files.h game-event.h \
  grafmode.h hint.h monster/mon-lore.h angband.h monster/mon-list.h \
  monster/mon-util.h object/obj-list.h angband.h object/tvalsval.h \
  textui.h ui-birth.h
./borg/borg1.o: borg/borg1.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cave.h borg/borg1.h borg/../effects.h \
  borg/../list-effects.h
./borg/borg2.o: borg/borg2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cave.h borg/borg1.h borg/../effects.h \
  borg/../list-effects.h borg/borg2.h
./borg/borg3.o: borg/borg3.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cave.h borg/borg1.h borg/../effects.h \
  borg/../list-effects.h borg/borg3.h
./borg/borg4.o: borg/borg4.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cave.h files.h borg/borg1.h \
  borg/../effects.h borg/../list-effects.h borg/borg2.h borg/borg3.h \
  borg/borg4.h
./borg/borg5.o: borg/borg5.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cave.h monster/mon-spell.h angband.h \
  monster/list-spell-effects.h borg/borg1.h borg/../effects.h \
  borg/../list-effects.h borg/borg2.h borg/borg3.h borg/borg4.h \
  borg/borg5.h borg/borg6.h
./borg/borg6.o: borg/borg6.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cave.h monster/mon-spell.h angband.h \
  monster/list-spell-effects.h borg/borg1.h borg/../effects.h \
  borg/../list-effects.h borg/borg2.h borg/borg3.h borg/borg4.h \
  borg/borg5.h borg/borg6.h borg/borg7.h
./borg/borg7.o: borg/borg7.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cave.h borg/borg1.h borg/../effects.h \
  borg/../list-effects.h borg/borg2.h borg/borg3.h borg/borg4.h \
  borg/borg5.h borg/borg6.h borg/borg7.h
./borg/borg8.o: borg/borg8.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h cave.h borg/borg1.h borg/../effects.h \
  borg/../list-effects.h borg/borg2.h borg/borg3.h borg/borg4.h \
  borg/borg5.h borg/borg6.h borg/borg7.h borg/borg8.h cmds.h
./borg/borg9.o: borg/borg9.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
  game-cmd.h cave.h object/list-object-flags.h monster/list-mon-spells.h \
  object/obj-flag.h object/object.h object/obj-flag.h option.h \
  player/player.h guid.h object/obj-flag.h object/object.h option.h \
  list-player-flags.h store.h ui.h z-textblock.h externs.h spells.h \
  list-gf-types.h object/tvalsval.h birth.h cave.h target.h spells.h \
  object/inventory.h borg/borg1.h borg/../effects.h \
  borg/../list-effects.h borg/borg2.h borg/borg3.h borg/borg4.h \
  borg/borg5.h borg/borg6.h borg/borg7.h borg/borg8.h cmds.h borg/borg9.h
./buildid.o: buildid.c buildid.h
./z-bitflag.o: z-bitflag.c z-bitflag.h h-basic.h z-form.h z-virt.h \
  defines.h
./z-file.o: z-file.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h
./z-form.o: z-form.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h
./z-msg.o: z-msg.c z-virt.h h-basic.h z-term.h ui-event.h z-util.h z-msg.h \
  z-msg-list.h
./z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h
./z-queue.o: z-queue.c z-queue.h h-basic.h
./z-rand.o: z-rand.c z-rand.h h-basic.h defines.h
./z-set.o: z-set.c z-set.h h-basic.h z-rand.h defines.h z-virt.h
./z-term.o: z-term.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
  defines.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-quark.h \
  z-msg.h z-msg-list.h config.h cave.h z-type.h game-cmd.h init.h \
  parser.h monster/constants.h monster/list-blow-methods.h \
  monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
  defines.h h-basic.h z-bitflag.h z-rand.h cave.h monster/mon-timed.h \
  object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
  z-quark.h z-bitflag.h game-cmd.h cave.h object/list-object-flags.h \
  monster/list-mon-spells.h object/obj-flag.h object/object.h z-rand.h \
  z-file.h z-textblock.h z-quark.h z-bitflag.h game-cmd.h cave.h \
  object/obj-flag.h option.h player/player.h guid.h object/obj-flag.h \
  object/object.h option.h list-player-flags.h store.h ui.h z-textblock.h \
  externs.h spells.h list-gf-types.h
./z-type.o: z-type.c h-basic.h z-form.h z-term.h ui-event.h ui.h \
  z-textblock.h z-file.h defines.h z-type.h z-virt.h
./z-util.o: z-util.c z-util.h h-basic.h
./z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
./z-textblock.o: z-textblock.c z-term.h h-basic.h ui-event.h z-textblock.h \
  z-file.h defines.h z-virt.h z-form.h
