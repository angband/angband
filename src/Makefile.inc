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
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h attack.h cave.h cmds.h object/slays.h \
 object/list-slays.h object/tvalsval.h target.h
./birth.o: birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cmds.h files.h game-event.h history.h object/inventory.h \
 object/tvalsval.h squelch.h ui-menu.h
./button.o: button.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h button.h
./cave.o: cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h game-event.h object/tvalsval.h squelch.h
./cmd0.o: cmd0.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h cmds.h files.h keymap.h ui-menu.h wizard.h
./cmd1.o: cmd1.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h attack.h cave.h cmds.h game-event.h generate.h history.h \
 object/inventory.h object/tvalsval.h squelch.h trap.h
./cmd2.o: cmd2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h attack.h cave.h cmds.h files.h game-event.h generate.h \
 object/tvalsval.h squelch.h trap.h
./cmd3.o: cmd3.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h button.h cave.h cmds.h object/inventory.h \
 object/tvalsval.h squelch.h ui-menu.h target.h
./cmd4.o: cmd4.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h buildid.h cave.h cmds.h files.h history.h \
 object/tvalsval.h prefs.h squelch.h ui-menu.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h cmds.h wizard.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h cmds.h effects.h list-effects.h \
 object/inventory.h object/tvalsval.h target.h
./death.o: death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cmds.h files.h history.h savefile.h ui-menu.h wizard.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h debug.h
./dungeon.o: dungeon.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h button.h cave.h cmds.h files.h game-event.h generate.h \
 init.h monster/mon-spell.h angband.h monster/list-spell-effects.h \
 object/tvalsval.h prefs.h savefile.h target.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h effects.h list-effects.h monster/mon-spell.h \
 angband.h monster/list-spell-effects.h
./files.o: files.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h buildid.h cave.h cmds.h history.h object/tvalsval.h \
 object/pval.h angband.h savefile.h ui-menu.h
./game-cmd.o: game-cmd.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cmds.h target.h
./game-event.o: game-event.c z-virt.h h-basic.h game-event.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h files.h generate.h monster/mon-spell.h angband.h \
 monster/list-spell-effects.h object/tvalsval.h trap.h z-queue.h
./guid.o: guid.c guid.h
./history.o: history.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h history.h
./init2.o: init2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h buildid.h button.h cave.h cmds.h game-event.h generate.h \
 keymap.h init.h monster/mon-power.h monster/monster.h \
 monster/mon-spell.h angband.h monster/list-spell-effects.h \
 object/slays.h object/list-slays.h object/tvalsval.h prefs.h randname.h \
 squelch.h object/list-object-flags.h list-effects.h \
 monster/list-mon-flags.h monster/list-mon-spells.h \
 monster/list-blow-methods.h monster/list-blow-effects.h
./keymap.o: keymap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h keymap.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h history.h monster/mon-spell.h angband.h \
 monster/list-spell-effects.h object/tvalsval.h savefile.h squelch.h
./monster/melee1.o: monster/melee1.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h attack.h monster/monster.h \
 object/tvalsval.h spells.h
./monster/melee2.o: monster/melee2.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h monster/mon-spell.h \
 monster/list-spell-effects.h object/slays.h object/list-slays.h \
 object/tvalsval.h spells.h squelch.h
./monster/monster1.o: monster/monster1.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h monster/mon-spell.h \
 monster/list-spell-effects.h object/tvalsval.h
./monster/monster2.o: monster/monster2.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h generate.h history.h \
 monster/mon-spell.h monster/list-spell-effects.h object/tvalsval.h \
 target.h z-term.h
./monster/mon-power.o: monster/mon-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h monster/mon-power.h monster/monster.h \
 monster/mon-spell.h monster/list-spell-effects.h
./monster/mon-spell.o: monster/mon-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h spells.h effects.h list-effects.h \
 monster/mon-spell.h monster/list-spell-effects.h \
 monster/list-mon-spells.h monster/list-spell-effects.h
./object/identify.o: object/identify.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h game-event.h history.h object/slays.h \
 object/list-slays.h object/tvalsval.h object/pval.h spells.h squelch.h
./object/obj-desc.o: object/obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h squelch.h object/tvalsval.h \
 object/pval.h
./object/obj-flag.o: object/obj-flag.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h object/list-object-flags.h
./object/obj-info.o: object/obj-info.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h attack.h effects.h list-effects.h \
 cmds.h object/tvalsval.h object/slays.h object/list-slays.h \
 object/pval.h
./object/obj-make.o: object/obj-make.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h object/tvalsval.h object/pval.h
./object/obj-power.o: object/obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h object/slays.h object/list-slays.h \
 object/tvalsval.h object/pval.h init.h effects.h list-effects.h \
 monster/mon-power.h monster/monster.h
./object/obj-ui.o: object/obj-ui.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h button.h object/tvalsval.h cmds.h
./object/obj-util.o: object/obj-util.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h effects.h list-effects.h generate.h \
 history.h object/inventory.h prefs.h spells.h squelch.h randname.h \
 object/tvalsval.h z-queue.h
./object/pval.o: object/pval.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
 game-cmd.h cave.h z-type.h object/list-object-flags.h object/object.h \
 monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h object/pval.h object/tvalsval.h
./object/randart.o: object/randart.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h \
 z-bitflag.h game-cmd.h cave.h z-type.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h object/slays.h object/list-slays.h \
 object/tvalsval.h object/pval.h init.h effects.h list-effects.h \
 randname.h
./object/slays.o: object/slays.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
 game-cmd.h cave.h z-type.h object/list-object-flags.h object/object.h \
 monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h object/slays.h object/list-slays.h \
 object/list-slays.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h
./parser.o: parser.c parser.h h-basic.h z-rand.h defines.h \
 list-player-flags.h z-util.h z-virt.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h randname.h
./pathfind.o: pathfind.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h squelch.h
./prefs.o: prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h keymap.h prefs.h squelch.h
./player/calcs.o: player/calcs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
 game-cmd.h cave.h z-type.h object/list-object-flags.h object/object.h \
 monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h files.h game-event.h \
 object/tvalsval.h object/pval.h spells.h squelch.h
./player/class.o: player/class.c externs.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h list-player-flags.h \
 h-basic.h z-bitflag.h h-basic.h z-form.h z-virt.h defines.h z-rand.h \
 cave.h types.h z-term.h ui-event.h z-type.h player/types.h \
 object/obj-flag.h z-file.h z-textblock.h z-file.h z-quark.h game-cmd.h \
 object/list-object-flags.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h object/object.h player/types.h store.h \
 object/obj-flag.h parser.h z-rand.h x-char.h z-msg.h spells.h \
 list-gf-types.h player/player.h guid.h
./player/player.o: player/player.c externs.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h list-player-flags.h \
 h-basic.h z-bitflag.h h-basic.h z-form.h z-virt.h defines.h z-rand.h \
 cave.h types.h z-term.h ui-event.h z-type.h player/types.h \
 object/obj-flag.h z-file.h z-textblock.h z-file.h z-quark.h game-cmd.h \
 object/list-object-flags.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h object/object.h player/types.h store.h \
 object/obj-flag.h parser.h z-rand.h x-char.h z-msg.h spells.h \
 list-gf-types.h history.h player/player.h guid.h
./player/race.o: player/race.c externs.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h list-player-flags.h \
 h-basic.h z-bitflag.h h-basic.h z-form.h z-virt.h defines.h z-rand.h \
 cave.h types.h z-term.h ui-event.h z-type.h player/types.h \
 object/obj-flag.h z-file.h z-textblock.h z-file.h z-quark.h game-cmd.h \
 object/list-object-flags.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h object/object.h player/types.h store.h \
 object/obj-flag.h parser.h z-rand.h x-char.h z-msg.h spells.h \
 list-gf-types.h player/player.h guid.h
./player/spell.o: player/spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
 game-cmd.h cave.h z-type.h object/list-object-flags.h object/object.h \
 monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h object/tvalsval.h spells.h
./player/timed.o: player/timed.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
 game-cmd.h cave.h z-type.h object/list-object-flags.h object/object.h \
 monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h
./player/util.o: player/util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-quark.h z-bitflag.h \
 game-cmd.h cave.h z-type.h object/list-object-flags.h object/object.h \
 monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 externs.h spells.h list-gf-types.h object/tvalsval.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h buildid.h
./signals.o: signals.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h files.h savefile.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h history.h savefile.h squelch.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h savefile.h
./spells1.o: spells1.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h generate.h object/tvalsval.h monster/mon-spell.h \
 angband.h monster/list-spell-effects.h squelch.h trap.h
./spells2.o: spells2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h generate.h history.h object/slays.h \
 object/list-slays.h object/tvalsval.h squelch.h target.h trap.h
./squelch.o: squelch.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cmds.h ui-menu.h object/tvalsval.h object/pval.h \
 angband.h squelch.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h cmds.h game-event.h history.h init.h \
 object/inventory.h object/tvalsval.h squelch.h target.h textui.h \
 ui-menu.h z-debug.h
./tables.o: tables.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h object/tvalsval.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h squelch.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h attack.h cave.h
./ui.o: ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h defines.h \
 list-player-flags.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-quark.h z-msg.h config.h option.h types.h game-cmd.h object/obj-flag.h \
 z-rand.h z-file.h z-textblock.h z-file.h defines.h z-quark.h z-bitflag.h \
 game-cmd.h cave.h types.h z-type.h h-basic.h object/list-object-flags.h \
 object/object.h monster/constants.h monster/list-blow-methods.h \
 monster/list-blow-effects.h monster/list-mon-flags.h monster/monster.h \
 defines.h h-basic.h player/types.h object/obj-flag.h object/object.h \
 option.h ui-event.h monster/list-mon-spells.h player/types.h \
 player/player.h guid.h store.h parser.h x-char.h ui.h z-textblock.h \
 z-type.h externs.h spells.h list-gf-types.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h button.h cmds.h files.h game-event.h object/tvalsval.h \
 ui-birth.h ui-menu.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h \
 defines.h z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h \
 h-basic.h object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h history.h object/tvalsval.h squelch.h ui-menu.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h ui-menu.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h \
 defines.h z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h \
 h-basic.h object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cmds.h keymap.h squelch.h prefs.h object/tvalsval.h \
 ui-menu.h files.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h object/tvalsval.h ui-menu.h
./util.o: util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h button.h cmds.h game-event.h randname.h
./variable.o: variable.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h buildid.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h \
 defines.h z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h \
 h-basic.h object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h buildid.h cmds.h object/tvalsval.h ui-menu.h wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h defines.h list-player-flags.h z-file.h z-util.h z-rand.h \
 z-term.h ui-event.h z-quark.h z-msg.h config.h option.h types.h \
 game-cmd.h object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h \
 defines.h z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h \
 h-basic.h object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h cmds.h wizard.h object/tvalsval.h effects.h \
 list-effects.h generate.h
./wizard.o: wizard.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h cmds.h files.h object/tvalsval.h ui-menu.h \
 target.h wizard.h
./x-char.o: x-char.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h
./x-spell.o: x-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cave.h effects.h list-effects.h object/tvalsval.h
./xtra2.o: xtra2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h cmds.h keymap.h history.h object/tvalsval.h target.h
./xtra3.o: xtra3.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h buildid.h button.h cave.h files.h game-event.h \
 object/tvalsval.h textui.h ui-birth.h
./buildid.o: buildid.c buildid.h
./z-bitflag.o: z-bitflag.c z-bitflag.h h-basic.h z-form.h z-virt.h \
 defines.h list-player-flags.h
./z-file.o: z-file.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h
./z-form.o: z-form.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h
./z-msg.o: z-msg.c z-virt.h h-basic.h z-term.h ui-event.h z-msg.h
./z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h
./z-queue.o: z-queue.c z-queue.h
./z-rand.o: z-rand.c z-rand.h h-basic.h defines.h list-player-flags.h
./z-term.o: z-term.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 defines.h list-player-flags.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-quark.h z-msg.h config.h option.h types.h game-cmd.h \
 object/obj-flag.h z-rand.h z-file.h z-textblock.h z-file.h defines.h \
 z-quark.h z-bitflag.h game-cmd.h cave.h types.h z-type.h h-basic.h \
 object/list-object-flags.h object/object.h monster/constants.h \
 monster/list-blow-methods.h monster/list-blow-effects.h \
 monster/list-mon-flags.h monster/monster.h defines.h h-basic.h \
 player/types.h object/obj-flag.h object/object.h option.h ui-event.h \
 monster/list-mon-spells.h player/types.h player/player.h guid.h store.h \
 parser.h x-char.h ui.h z-textblock.h z-type.h externs.h spells.h \
 list-gf-types.h
./z-type.o: z-type.c h-basic.h z-form.h z-term.h ui-event.h ui.h \
 z-textblock.h z-file.h defines.h list-player-flags.h z-type.h
./z-util.o: z-util.c z-util.h h-basic.h
./z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
./z-textblock.o: z-textblock.c z-term.h h-basic.h ui-event.h z-textblock.h \
 z-file.h defines.h list-player-flags.h z-virt.h z-form.h
