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
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h cmds.h mon-make.h mon-msg.h \
 mon-util.h obj-desc.h obj-identify.h obj-slays.h list-slays.h obj-ui.h \
 obj-util.h project.h target.h
./birth.o: birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h birth.h cmds.h files.h game-event.h \
 history.h obj-identify.h obj-make.h obj-tval.h obj-util.h squelch.h \
 quest.h ui-menu.h
./cave.o: cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h game-event.h mon-util.h obj-tval.h \
 obj-ui.h obj-util.h project.h squelch.h cmds.h grafmode.h
./cmd-cave.o: cmd-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h cmds.h dungeon.h files.h \
 game-event.h generate.h mon-util.h pathfind.h squelch.h obj-chest.h \
 obj-identify.h obj-util.h
./cmd-context.o: cmd-context.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h files.h keymap.h textui.h \
 ui-menu.h wizard.h target.h squelch.h obj-chest.h obj-desc.h \
 obj-identify.h obj-info.h obj-tvalsval.h obj-tval.h obj-ui.h obj-util.h \
 mon-lore.h mon-util.h
./cmd-know.o: cmd-know.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h files.h history.h mon-lore.h \
 mon-list.h mon-util.h obj-list.h obj-tval.h obj-ui.h obj-util.h prefs.h \
 squelch.h target.h ui-menu.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h mon-util.h wizard.h target.h \
 prefs.h files.h buildid.h history.h obj-util.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h cmds.h effects.h \
 list-effects.h obj-desc.h obj-identify.h obj-info.h obj-tval.h \
 obj-tvalsval.h obj-ui.h obj-util.h squelch.h target.h ui-menu.h \
 ui-options.h
./cmd-pickup.o: cmd-pickup.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h cmds.h game-event.h \
 generate.h history.h mon-util.h obj-desc.h obj-identify.h obj-tval.h \
 obj-tvalsval.h obj-ui.h obj-util.h squelch.h
./cmd-process.o: cmd-process.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h cmds.h files.h keymap.h \
 textui.h ui-menu.h wizard.h target.h obj-util.h
./death.o: death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h death.h files.h history.h \
 savefile.h ui-menu.h wizard.h obj-desc.h obj-identify.h obj-info.h \
 obj-ui.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h debug.h
./dungeon.o: dungeon.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h birth.h cmds.h dungeon.h files.h \
 game-event.h generate.h grafmode.h mon-list.h mon-make.h mon-spell.h \
 list-spell-effects.h mon-util.h obj-desc.h obj-identify.h obj-tval.h \
 obj-tvalsval.h obj-util.h pathfind.h prefs.h savefile.h target.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h effects.h list-effects.h dungeon.h \
 mon-spell.h list-spell-effects.h mon-util.h obj-identify.h obj-util.h \
 project.h
./files.o: files.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h buildid.h cmds.h death.h files.h \
 history.h obj-desc.h obj-identify.h obj-info.h obj-pval.h obj-ui.h \
 obj-util.h savefile.h ui-menu.h
./game-cmd.o: game-cmd.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h cmds.h obj-chest.h obj-desc.h \
 obj-tval.h obj-ui.h obj-util.h target.h
./game-event.o: game-event.c z-virt.h h-basic.h game-event.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h files.h game-event.h generate.h \
 mon-make.h mon-spell.h list-spell-effects.h obj-util.h z-queue.h \
 h-basic.h
./gen-cave.o: gen-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h dungeon.h files.h game-event.h \
 generate.h mon-make.h mon-spell.h list-spell-effects.h z-queue.h \
 h-basic.h
./gen-monster.o: gen-monster.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h files.h game-event.h generate.h \
 mon-make.h mon-spell.h list-spell-effects.h
./gen-room.o: gen-room.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h files.h game-event.h generate.h \
 mon-make.h mon-spell.h list-spell-effects.h z-queue.h h-basic.h
./gen-util.o: gen-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h files.h game-event.h generate.h \
 mon-make.h mon-spell.h list-spell-effects.h obj-make.h obj-tvalsval.h \
 obj-util.h z-queue.h h-basic.h
./grafmode.o: grafmode.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h grafmode.h
./guid.o: guid.c guid.h
./history.o: history.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h history.h obj-desc.h obj-util.h
./init.o: init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h buildid.h cmds.h game-event.h \
 generate.h history.h hint.h keymap.h mon-init.h mon-list.h mon-msg.h \
 mon-util.h obj-list.h obj-make.h obj-slays.h list-slays.h obj-tval.h \
 obj-tvalsval.h obj-util.h prefs.h project.h quest.h randname.h squelch.h \
 list-effects.h
./keymap.o: keymap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h keymap.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h dungeon.h history.h mon-make.h \
 mon-spell.h list-spell-effects.h obj-make.h obj-util.h savefile.h \
 squelch.h quest.h
./mon-blow-effects.o: mon-blow-effects.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h \
 z-type.h list-square-flags.h list-terrain-flags.h game-cmd.h init.h \
 parser.h mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h mon-util.h obj-desc.h obj-make.h \
 obj-tval.h obj-tvalsval.h obj-util.h project.h
./mon-blow-methods.o: mon-blow-methods.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h \
 z-type.h list-square-flags.h list-terrain-flags.h game-cmd.h init.h \
 parser.h mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h
./mon-list.o: mon-list.c mon-util.h angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h \
 ui-event.h z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h \
 z-type.h list-square-flags.h list-terrain-flags.h game-cmd.h init.h \
 parser.h mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h mon-list.h project.h
./mon-lore.o: mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h mon-lore.h mon-make.h mon-spell.h \
 list-spell-effects.h mon-util.h obj-identify.h obj-tval.h obj-util.h \
 attack.h
./mon-make.o: mon-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h alloc.h history.h target.h mon-lore.h \
 mon-make.h mon-util.h obj-identify.h obj-make.h obj-tval.h \
 obj-tvalsval.h obj-util.h quest.h
./mon-melee2.o: mon-melee2.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h mon-make.h mon-spell.h \
 list-spell-effects.h mon-util.h obj-desc.h obj-identify.h obj-slays.h \
 list-slays.h obj-tval.h obj-util.h project.h squelch.h
./mon-msg.o: mon-msg.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h mon-msg.h mon-util.h
./mon-power.o: mon-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h mon-power.h mon-spell.h \
 list-spell-effects.h
./mon-spell.o: mon-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h effects.h list-effects.h mon-spell.h \
 list-spell-effects.h mon-util.h project.h
./mon-timed.o: mon-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h mon-msg.h mon-spell.h \
 list-spell-effects.h mon-util.h
./mon-util.o: mon-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h mon-make.h mon-msg.h mon-spell.h \
 list-spell-effects.h mon-list.h mon-util.h squelch.h obj-desc.h \
 obj-identify.h obj-util.h
./mon-init.o: mon-init.c externs.h mon-constants.h list-mon-flags.h \
 monster.h h-basic.h z-bitflag.h z-form.h z-virt.h z-rand.h cave.h \
 z-type.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h z-file.h list-object-flags.h list-object-modifiers.h \
 object.h z-quark.h z-dice.h z-expression.h list-gf-types.h option.h \
 list-player-flags.h list-blow-effects.h list-mon-spells.h store.h \
 parser.h trap.h list-trap-flags.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h spells.h mon-init.h mon-msg.h angband.h z-color.h z-util.h \
 z-textblock.h config.h game-cmd.h init.h ui.h ui-input.h mon-power.h \
 mon-spell.h list-spell-effects.h mon-util.h obj-tval.h obj-util.h
./obj-chest.o: obj-chest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h obj-chest.h obj-identify.h obj-make.h \
 obj-tval.h obj-tvalsval.h obj-util.h mon-util.h
./obj-desc.o: obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h squelch.h obj-chest.h obj-desc.h \
 obj-identify.h obj-tval.h obj-tvalsval.h obj-pval.h obj-util.h
./obj-flag.o: obj-flag.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h
./obj-identify.o: obj-identify.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h game-event.h history.h obj-desc.h \
 obj-identify.h obj-pval.h obj-slays.h list-slays.h obj-tval.h \
 obj-tvalsval.h obj-util.h squelch.h
./obj-info.o: obj-info.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h effects.h list-effects.h \
 cmds.h obj-identify.h obj-info.h obj-make.h obj-pval.h obj-slays.h \
 list-slays.h obj-tval.h obj-tvalsval.h obj-util.h project.h
./obj-list.o: obj-list.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h obj-desc.h obj-identify.h obj-list.h \
 obj-tval.h obj-tvalsval.h obj-ui.h obj-util.h squelch.h
./obj-make.o: obj-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h alloc.h obj-make.h obj-pval.h \
 obj-tval.h obj-tvalsval.h obj-util.h
./obj-power.o: obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h obj-identify.h obj-pval.h obj-slays.h \
 list-slays.h obj-tval.h obj-tvalsval.h obj-util.h effects.h \
 list-effects.h mon-power.h
./obj-pval.o: obj-pval.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h obj-identify.h obj-tval.h obj-pval.h
./obj-randart.o: obj-randart.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h effects.h list-effects.h obj-desc.h \
 obj-slays.h list-slays.h obj-tvalsval.h obj-util.h randname.h
./obj-slays.o: obj-slays.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h obj-desc.h obj-identify.h obj-slays.h \
 list-slays.h obj-util.h mon-util.h
./obj-tval.o: obj-tval.c obj-tval.h object.h z-rand.h h-basic.h z-quark.h \
 z-bitflag.h z-form.h z-virt.h z-dice.h z-expression.h obj-flag.h \
 z-file.h list-object-flags.h list-object-modifiers.h list-gf-types.h \
 obj-tvalsval.h z-type.h z-util.h
./obj-ui.o: obj-ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h keymap.h obj-desc.h \
 obj-identify.h obj-tval.h obj-tvalsval.h obj-ui.h obj-util.h
./obj-util.o: obj-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h effects.h list-effects.h generate.h \
 grafmode.h history.h mon-make.h obj-desc.h obj-identify.h obj-info.h \
 obj-make.h obj-tval.h obj-tvalsval.h obj-util.h prefs.h randname.h \
 squelch.h z-queue.h h-basic.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h
./parser.o: parser.c externs.h mon-constants.h list-mon-flags.h monster.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h z-rand.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h player.h guid.h obj-flag.h \
 z-file.h list-object-flags.h list-object-modifiers.h object.h z-quark.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h parser.h trap.h \
 list-trap-flags.h z-term.h ui-event.h z-msg.h z-msg-list.h spells.h \
 ui-input.h z-util.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h randname.h
./pathfind.o: pathfind.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h pathfind.h squelch.h mon-util.h \
 obj-util.h
./prefs.o: prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h grafmode.h keymap.h obj-tval.h \
 obj-util.h prefs.h project.h squelch.h
./player-calcs.o: player-calcs.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h dungeon.h files.h game-event.h \
 mon-msg.h mon-util.h obj-identify.h obj-pval.h obj-tval.h obj-tvalsval.h \
 obj-util.h squelch.h
./player-class.o: player-class.c externs.h mon-constants.h list-mon-flags.h \
 monster.h h-basic.h z-bitflag.h z-form.h z-virt.h z-rand.h cave.h \
 z-type.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h z-file.h list-object-flags.h list-object-modifiers.h \
 object.h z-quark.h z-dice.h z-expression.h list-gf-types.h option.h \
 list-player-flags.h list-blow-effects.h list-mon-spells.h store.h \
 parser.h trap.h list-trap-flags.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h spells.h
./player-race.o: player-race.c externs.h mon-constants.h list-mon-flags.h \
 monster.h h-basic.h z-bitflag.h z-form.h z-virt.h z-rand.h cave.h \
 z-type.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h z-file.h list-object-flags.h list-object-modifiers.h \
 object.h z-quark.h z-dice.h z-expression.h list-gf-types.h option.h \
 list-player-flags.h list-blow-effects.h list-mon-spells.h store.h \
 parser.h trap.h list-trap-flags.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h spells.h
./player-spell.o: player-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h obj-tvalsval.h obj-util.h
./player-timed.o: player-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h obj-identify.h
./player-util.o: player-util.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h obj-tvalsval.h obj-util.h
./player.o: player.c externs.h mon-constants.h list-mon-flags.h monster.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h z-rand.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h player.h guid.h obj-flag.h \
 z-file.h list-object-flags.h list-object-modifiers.h object.h z-quark.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h parser.h trap.h \
 list-trap-flags.h z-term.h ui-event.h z-msg.h z-msg-list.h spells.h \
 history.h birth.h ui-input.h z-color.h z-util.h
./project.o: project.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h dungeon.h generate.h grafmode.h \
 mon-make.h mon-msg.h mon-spell.h list-spell-effects.h mon-util.h \
 obj-chest.h obj-desc.h obj-identify.h obj-tval.h obj-util.h project.h \
 squelch.h
./quest.o: quest.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h quest.h obj-util.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h buildid.h
./signals.o: signals.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h files.h savefile.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h dungeon.h history.h mon-make.h quest.h \
 savefile.h squelch.h obj-util.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h savefile.h
./spells.o: spells.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h dungeon.h generate.h history.h \
 mon-lore.h mon-make.h mon-util.h obj-chest.h obj-desc.h obj-identify.h \
 obj-make.h obj-slays.h list-slays.h obj-tval.h obj-tvalsval.h obj-ui.h \
 obj-util.h project.h squelch.h target.h
./squelch.o: squelch.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h ui-menu.h obj-desc.h \
 obj-identify.h obj-pval.h obj-tval.h obj-tvalsval.h obj-ui.h obj-util.h \
 squelch.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h game-event.h history.h \
 obj-desc.h obj-identify.h obj-info.h obj-make.h obj-tval.h \
 obj-tvalsval.h obj-ui.h obj-util.h squelch.h target.h textui.h ui-menu.h \
 z-debug.h
./tables.o: tables.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h mon-lore.h mon-util.h obj-desc.h \
 obj-ui.h obj-util.h project.h squelch.h target.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h attack.h effects.h list-effects.h
./ui.o: ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-color.h \
 z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 z-textblock.h z-quark.h config.h cave.h z-type.h list-square-flags.h \
 list-terrain-flags.h game-cmd.h init.h parser.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h player.h guid.h obj-flag.h \
 list-object-flags.h list-object-modifiers.h object.h z-dice.h \
 z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h files.h game-event.h \
 obj-tvalsval.h ui-birth.h ui-menu.h ui-options.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h
./ui-input.o: ui-input.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h target.h files.h game-event.h \
 randname.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h history.h mon-lore.h mon-util.h \
 obj-desc.h obj-identify.h obj-info.h obj-make.h obj-tvalsval.h \
 obj-util.h squelch.h ui-menu.h ui-options.h grafmode.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h ui-menu.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h keymap.h squelch.h prefs.h \
 obj-desc.h obj-tvalsval.h obj-tval.h obj-util.h ui-menu.h ui-options.h \
 files.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h obj-tvalsval.h obj-ui.h \
 obj-util.h ui-menu.h
./variable.o: variable.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h buildid.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h buildid.h cmds.h mon-lore.h obj-desc.h \
 obj-identify.h obj-info.h obj-make.h obj-tvalsval.h obj-tval.h \
 obj-util.h ui-menu.h wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h wizard.h mon-make.h effects.h \
 list-effects.h generate.h
./wizard.o: wizard.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h effects.h list-effects.h \
 files.h mon-lore.h mon-make.h mon-util.h obj-desc.h obj-identify.h \
 obj-make.h obj-tvalsval.h obj-tval.h obj-ui.h obj-util.h project.h \
 target.h ui-menu.h wizard.h
./x-spell.o: x-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h effects.h list-effects.h \
 obj-tvalsval.h project.h list-spells-arcane.h list-spells-prayer.h
./xtra2.o: xtra2.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h cmds.h keymap.h history.h pathfind.h \
 target.h
./xtra3.o: xtra3.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-file.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h z-textblock.h z-quark.h config.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h game-cmd.h init.h parser.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h player.h \
 guid.h obj-flag.h list-object-flags.h list-object-modifiers.h object.h \
 z-dice.h z-expression.h list-gf-types.h option.h list-player-flags.h \
 list-blow-effects.h list-mon-spells.h store.h ui.h ui-input.h externs.h \
 trap.h list-trap-flags.h spells.h buildid.h files.h game-event.h \
 grafmode.h hint.h mon-lore.h mon-list.h mon-util.h obj-desc.h obj-list.h \
 obj-ui.h obj-util.h textui.h ui-birth.h
./buildid.o: buildid.c buildid.h
./z-bitflag.o: z-bitflag.c z-bitflag.h h-basic.h z-form.h z-virt.h
./z-color.o: z-color.c h-basic.h z-color.h z-util.h
./z-dice.o: z-dice.c z-dice.h h-basic.h z-rand.h z-expression.h z-virt.h \
 z-util.h
./z-expression.o: z-expression.c z-expression.h h-basic.h z-virt.h z-util.h
./z-file.o: z-file.c h-basic.h z-file.h z-form.h z-util.h z-virt.h
./z-form.o: z-form.c z-form.h h-basic.h z-type.h z-util.h z-virt.h
./z-msg.o: z-msg.c z-virt.h h-basic.h z-color.h z-util.h z-msg.h \
 z-msg-list.h
./z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h
./z-queue.o: z-queue.c z-queue.h h-basic.h
./z-rand.o: z-rand.c z-rand.h h-basic.h
./z-set.o: z-set.c z-set.h h-basic.h z-rand.h z-virt.h
./z-term.o: z-term.c buildid.h h-basic.h z-color.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h z-util.h z-virt.h
./z-textblock.o: z-textblock.c z-color.h h-basic.h z-textblock.h z-file.h \
 z-util.h z-virt.h z-form.h
./z-type.o: z-type.c z-type.h h-basic.h z-virt.h
./z-util.o: z-util.c z-util.h h-basic.h
./z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
