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
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h mon-make.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-msg.h \
 mon-util.h obj-desc.h obj-identify.h obj-slays.h list-slays.h obj-ui.h \
 obj-util.h project.h list-project-environs.h list-project-monsters.h \
 spells.h tables.h target.h
./birth.o: birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h birth.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h files.h game-event.h history.h init.h \
 parser.h monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-identify.h obj-make.h \
 obj-tval.h obj-util.h squelch.h store.h quest.h ui-menu.h
./cave.o: cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h dungeon.h game-event.h grafmode.h init.h parser.h mon-util.h \
 monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-tval.h obj-ui.h obj-util.h \
 prefs.h project.h list-project-environs.h list-project-monsters.h \
 squelch.h tables.h trap.h list-trap-flags.h
./cmd-cave.o: cmd-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h dungeon.h files.h game-event.h generate.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-util.h pathfind.h spells.h \
 squelch.h trap.h list-trap-flags.h obj-chest.h obj-identify.h obj-util.h \
 tables.h
./cmd-core.o: cmd-core.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h obj-chest.h obj-desc.h obj-tval.h obj-ui.h \
 obj-util.h spells.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h target.h
./cmd-context.o: cmd-context.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h files.h keymap.h textui.h ui-menu.h wizard.h target.h squelch.h \
 obj-chest.h obj-desc.h obj-identify.h obj-info.h obj-tvalsval.h \
 obj-tval.h obj-ui.h obj-util.h mon-lore.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-util.h \
 store.h parser.h
./cmd-know.o: cmd-know.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h dungeon.h files.h history.h init.h parser.h mon-lore.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-list.h mon-util.h obj-list.h obj-tval.h obj-ui.h \
 obj-util.h prefs.h squelch.h target.h ui-menu.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h init.h parser.h mon-util.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h wizard.h \
 target.h prefs.h files.h buildid.h history.h obj-util.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h effects.h list-effects.h obj-desc.h \
 obj-identify.h obj-info.h obj-tval.h obj-tvalsval.h obj-ui.h obj-util.h \
 spells.h monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h squelch.h target.h trap.h \
 list-trap-flags.h ui-menu.h ui-options.h
./cmd-pickup.o: cmd-pickup.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h game-event.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h history.h mon-util.h obj-desc.h obj-identify.h \
 obj-tval.h obj-tvalsval.h obj-ui.h obj-util.h squelch.h tables.h trap.h \
 list-trap-flags.h
./cmd-process.o: cmd-process.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h files.h keymap.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h textui.h \
 ui-menu.h wizard.h target.h obj-util.h ui-game.h
./death.o: death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h death.h files.h history.h init.h parser.h \
 savefile.h ui-menu.h wizard.h obj-desc.h obj-identify.h obj-info.h \
 obj-ui.h score.h store.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h debug.h
./dungeon.o: dungeon.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h birth.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h dungeon.h files.h game-event.h generate.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h grafmode.h init.h parser.h \
 mon-list.h mon-make.h mon-spell.h list-spell-effects.h mon-util.h \
 obj-desc.h obj-identify.h obj-tval.h obj-tvalsval.h obj-util.h \
 pathfind.h prefs.h savefile.h spells.h store.h tables.h target.h \
 ui-game.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h list-effects.h dungeon.h mon-spell.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h \
 list-spell-effects.h mon-util.h obj-identify.h obj-util.h project.h \
 list-project-environs.h list-project-monsters.h spells.h tables.h trap.h \
 list-trap-flags.h
./files.o: files.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h death.h dungeon.h files.h history.h init.h \
 parser.h obj-desc.h obj-identify.h obj-info.h obj-ui.h obj-util.h \
 savefile.h score.h store.h signals.h ui-game.h ui-menu.h
./game-event.o: game-event.c z-virt.h h-basic.h game-event.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h files.h game-event.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h init.h parser.h mon-make.h mon-spell.h \
 list-spell-effects.h obj-util.h trap.h list-trap-flags.h z-queue.h
./gen-cave.o: gen-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h files.h game-event.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h init.h parser.h mon-make.h mon-spell.h \
 list-spell-effects.h store.h tables.h trap.h list-trap-flags.h z-queue.h
./gen-chunk.o: gen-chunk.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 generate.h mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h init.h parser.h mon-make.h \
 obj-util.h trap.h list-trap-flags.h
./gen-monster.o: gen-monster.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 files.h game-event.h generate.h mon-constants.h list-mon-flags.h \
 monster.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h init.h parser.h \
 mon-make.h mon-spell.h list-spell-effects.h
./gen-room.o: gen-room.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 files.h game-event.h generate.h mon-constants.h list-mon-flags.h \
 monster.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h init.h parser.h \
 mon-make.h mon-spell.h list-spell-effects.h obj-tvalsval.h tables.h \
 trap.h list-trap-flags.h z-queue.h
./gen-util.o: gen-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 files.h game-event.h generate.h mon-constants.h list-mon-flags.h \
 monster.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h init.h parser.h \
 mon-make.h mon-spell.h list-spell-effects.h obj-make.h obj-tvalsval.h \
 obj-util.h tables.h trap.h list-trap-flags.h z-queue.h
./grafmode.o: grafmode.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h grafmode.h init.h parser.h
./guid.o: guid.c guid.h
./history.o: history.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h dungeon.h history.h obj-desc.h obj-util.h cave.h \
 list-square-flags.h list-terrain-flags.h wizard.h
./init.o: init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h game-event.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h history.h hint.h keymap.h init.h parser.h mon-init.h \
 mon-list.h mon-msg.h mon-util.h obj-list.h obj-make.h obj-slays.h \
 list-slays.h obj-tval.h obj-tvalsval.h obj-util.h prefs.h project.h \
 list-project-environs.h list-project-monsters.h quest.h randname.h \
 spells.h squelch.h store.h trap.h list-trap-flags.h list-effects.h
./keymap.o: keymap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h keymap.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h generate.h mon-constants.h list-mon-flags.h monster.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h history.h init.h parser.h \
 mon-make.h mon-spell.h list-spell-effects.h obj-make.h obj-util.h \
 savefile.h squelch.h store.h quest.h trap.h list-trap-flags.h ui-game.h
./mon-blow-effects.o: mon-blow-effects.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h config.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-util.h obj-desc.h obj-make.h \
 obj-tval.h obj-tvalsval.h obj-util.h project.h list-project-environs.h \
 list-project-monsters.h spells.h
./mon-blow-methods.o: mon-blow-methods.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h config.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h
./mon-list.o: mon-list.c dungeon.h angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h mon-util.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-list.h project.h \
 list-project-environs.h list-project-monsters.h
./mon-lore.o: mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h mon-lore.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-make.h mon-spell.h \
 list-spell-effects.h mon-util.h obj-identify.h obj-tval.h obj-util.h \
 attack.h
./mon-make.o: mon-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h alloc.h dungeon.h history.h init.h parser.h target.h \
 mon-lore.h monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-make.h mon-util.h obj-identify.h obj-make.h \
 obj-tval.h obj-tvalsval.h obj-util.h quest.h tables.h
./mon-melee2.o: mon-melee2.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h cave.h list-square-flags.h \
 list-terrain-flags.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-make.h mon-spell.h \
 list-spell-effects.h mon-util.h obj-desc.h obj-identify.h obj-slays.h \
 list-slays.h obj-tval.h obj-util.h project.h list-project-environs.h \
 list-project-monsters.h spells.h squelch.h tables.h
./mon-msg.o: mon-msg.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h mon-msg.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-util.h
./mon-power.o: mon-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h init.h parser.h mon-power.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-spell.h \
 list-spell-effects.h prefs.h tables.h
./mon-spell.o: mon-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h effects.h list-effects.h mon-spell.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h \
 list-spell-effects.h mon-util.h project.h list-project-environs.h \
 list-project-monsters.h spells.h
./mon-timed.o: mon-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h mon-msg.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-spell.h list-spell-effects.h \
 mon-util.h
./mon-util.o: mon-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h init.h parser.h mon-make.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-msg.h \
 mon-spell.h list-spell-effects.h mon-list.h mon-util.h squelch.h \
 obj-desc.h obj-identify.h obj-util.h
./mon-init.o: mon-init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h init.h parser.h mon-init.h mon-msg.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-power.h \
 mon-spell.h list-spell-effects.h mon-util.h obj-tval.h obj-util.h
./obj-chest.o: obj-chest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 obj-chest.h obj-identify.h obj-make.h obj-tval.h obj-tvalsval.h \
 obj-util.h mon-util.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h spells.h tables.h
./obj-desc.o: obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h squelch.h obj-chest.h obj-desc.h obj-identify.h \
 obj-tval.h obj-tvalsval.h obj-util.h cave.h list-square-flags.h \
 list-terrain-flags.h
./obj-identify.o: obj-identify.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h game-event.h history.h obj-desc.h obj-identify.h obj-slays.h \
 monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h list-slays.h obj-tval.h \
 obj-tvalsval.h obj-util.h spells.h squelch.h
./obj-info.o: obj-info.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h effects.h list-effects.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h init.h parser.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-identify.h obj-info.h obj-make.h obj-slays.h \
 list-slays.h obj-tval.h obj-tvalsval.h obj-util.h project.h \
 list-project-environs.h list-project-monsters.h tables.h
./obj-list.o: obj-list.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h dungeon.h init.h parser.h obj-desc.h obj-identify.h \
 obj-list.h obj-tval.h obj-tvalsval.h obj-ui.h obj-util.h cave.h \
 list-square-flags.h list-terrain-flags.h squelch.h
./obj-make.o: obj-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h alloc.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h parser.h obj-make.h obj-tval.h \
 obj-tvalsval.h obj-util.h
./obj-power.o: obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h obj-identify.h obj-slays.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h list-slays.h \
 obj-tval.h obj-tvalsval.h obj-util.h init.h parser.h effects.h \
 list-effects.h mon-power.h
./obj-properties.o: obj-properties.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h \
 z-msg.h z-msg-list.h config.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h
./obj-randart.o: obj-randart.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h effects.h list-effects.h init.h parser.h obj-desc.h \
 obj-slays.h monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h list-slays.h obj-tvalsval.h obj-util.h randname.h \
 wizard.h
./obj-slays.o: obj-slays.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h init.h parser.h obj-desc.h obj-identify.h \
 obj-slays.h monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h list-slays.h obj-util.h mon-util.h
./obj-tval.o: obj-tval.c obj-tval.h object.h z-rand.h h-basic.h z-quark.h \
 z-bitflag.h z-form.h z-virt.h z-dice.h z-expression.h obj-properties.h \
 z-file.h list-object-flags.h list-kind-flags.h list-object-modifiers.h \
 list-elements.h obj-tvalsval.h z-type.h z-util.h
./obj-ui.o: obj-ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h keymap.h obj-desc.h obj-identify.h obj-tval.h \
 obj-tvalsval.h obj-ui.h obj-util.h store.h parser.h ui-game.h
./obj-util.o: obj-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h effects.h list-effects.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h grafmode.h history.h init.h parser.h mon-make.h \
 obj-desc.h obj-identify.h obj-info.h obj-make.h obj-tval.h \
 obj-tvalsval.h obj-util.h prefs.h randname.h spells.h squelch.h \
 z-queue.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h ui-game.h
./parser.o: parser.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h parser.h z-dice.h z-expression.h ui-input.h angband.h \
 z-color.h z-util.h z-term.h ui-event.h z-msg.h z-msg-list.h config.h \
 option.h list-options.h player.h guid.h obj-properties.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h list-elements.h list-player-flags.h ui.h z-textblock.h \
 cmd-core.h z-type.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h randname.h
./pathfind.o: pathfind.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h pathfind.h squelch.h mon-util.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-util.h tables.h
./prefs.o: prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 grafmode.h init.h parser.h keymap.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h obj-tval.h \
 obj-util.h prefs.h project.h list-project-environs.h \
 list-project-monsters.h squelch.h spells.h ui-game.h
./player-calcs.o: player-calcs.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h files.h game-event.h mon-msg.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-util.h \
 obj-identify.h obj-tval.h obj-tvalsval.h obj-util.h spells.h squelch.h
./player-class.o: player-class.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-rand.h z-quark.h \
 z-dice.h z-expression.h list-elements.h option.h list-options.h \
 list-player-flags.h
./player-race.o: player-race.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-rand.h z-quark.h \
 z-dice.h z-expression.h list-elements.h option.h list-options.h \
 list-player-flags.h
./player-spell.o: player-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 obj-tvalsval.h obj-util.h spells.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h
./player-timed.o: player-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 obj-identify.h
./player-util.o: player-util.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 obj-tvalsval.h obj-util.h tables.h target.h
./player.o: player.c history.h angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h birth.h spells.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h
./project.o: project.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h generate.h mon-constants.h list-mon-flags.h monster.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h grafmode.h mon-make.h mon-msg.h \
 mon-spell.h list-spell-effects.h mon-util.h obj-chest.h obj-desc.h \
 obj-identify.h obj-tval.h obj-util.h prefs.h project.h \
 list-project-environs.h list-project-monsters.h spells.h squelch.h \
 tables.h trap.h list-trap-flags.h
./quest.o: quest.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h quest.h obj-util.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h buildid.h dungeon.h init.h parser.h score.h
./signals.o: signals.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h dungeon.h files.h savefile.h signals.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h history.h init.h parser.h mon-make.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h quest.h \
 savefile.h squelch.h store.h obj-util.h trap.h list-trap-flags.h \
 ui-game.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h dungeon.h init.h parser.h savefile.h
./spells.o: spells.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h generate.h mon-constants.h list-mon-flags.h monster.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h history.h init.h parser.h \
 mon-lore.h mon-make.h mon-util.h obj-chest.h obj-desc.h obj-identify.h \
 obj-make.h obj-slays.h list-slays.h obj-tval.h obj-tvalsval.h obj-ui.h \
 obj-util.h project.h list-project-environs.h list-project-monsters.h \
 spells.h squelch.h tables.h target.h trap.h list-trap-flags.h
./squelch.o: squelch.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h parser.h ui-menu.h obj-desc.h obj-identify.h \
 obj-tval.h obj-tvalsval.h obj-ui.h obj-util.h squelch.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h game-event.h hint.h history.h init.h parser.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-desc.h obj-identify.h obj-info.h obj-make.h \
 obj-tval.h obj-tvalsval.h obj-util.h spells.h squelch.h store.h target.h \
 z-debug.h
./tables.o: tables.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h tables.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 keymap.h mon-lore.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-util.h obj-desc.h obj-ui.h \
 obj-util.h project.h list-project-environs.h list-project-monsters.h \
 squelch.h tables.h target.h trap.h list-trap-flags.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h attack.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h effects.h list-effects.h init.h parser.h \
 spells.h monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h trap.h list-trap-flags.h
./ui.o: ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-color.h \
 z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h config.h \
 option.h z-file.h list-options.h player.h guid.h obj-properties.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h list-elements.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h z-type.h cave.h \
 list-square-flags.h list-terrain-flags.h tables.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h files.h game-event.h obj-tvalsval.h target.h \
 ui-birth.h ui-game.h ui-menu.h ui-options.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h
./ui-game.o: ui-game.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h files.h game-event.h grafmode.h hint.h init.h \
 parser.h mon-lore.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-list.h mon-util.h obj-desc.h \
 obj-list.h obj-ui.h obj-util.h prefs.h textui.h ui-birth.h
./ui-input.o: ui-input.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h target.h files.h game-event.h init.h \
 parser.h pathfind.h randname.h signals.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h history.h init.h parser.h mon-lore.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-util.h \
 obj-desc.h obj-identify.h obj-info.h obj-make.h obj-tvalsval.h \
 obj-util.h squelch.h score.h store.h tables.h target.h ui-menu.h \
 ui-options.h grafmode.h wizard.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h tables.h target.h ui-menu.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h parser.h keymap.h squelch.h prefs.h \
 obj-desc.h obj-tvalsval.h obj-tval.h obj-util.h tables.h target.h \
 ui-game.h ui-menu.h ui-options.h files.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h obj-tvalsval.h obj-ui.h obj-util.h spells.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h ui-menu.h
./ui-store.o: ui-store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h game-event.h hint.h history.h init.h parser.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-desc.h obj-identify.h obj-info.h obj-make.h \
 obj-tval.h obj-tvalsval.h obj-ui.h obj-util.h spells.h squelch.h store.h \
 target.h textui.h ui-game.h ui-menu.h z-debug.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h buildid.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h init.h parser.h mon-lore.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-desc.h obj-identify.h obj-info.h obj-make.h \
 obj-tvalsval.h obj-tval.h obj-util.h ui-menu.h wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h \
 z-msg-list.h config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h dungeon.h init.h parser.h wizard.h mon-make.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-tvalsval.h obj-util.h effects.h list-effects.h \
 generate.h tables.h
./wizard.o: wizard.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h effects.h list-effects.h files.h init.h parser.h mon-lore.h \
 monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-make.h mon-util.h obj-desc.h \
 obj-identify.h obj-make.h obj-tvalsval.h obj-tval.h obj-ui.h obj-util.h \
 project.h list-project-environs.h list-project-monsters.h spells.h \
 target.h ui-game.h ui-menu.h wizard.h
./x-spell.o: x-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h z-msg.h z-msg-list.h \
 config.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-player-flags.h ui.h z-textblock.h ui-input.h \
 cmd-core.h z-type.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h list-effects.h obj-tvalsval.h project.h \
 list-project-environs.h list-project-monsters.h spells.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h list-spells-arcane.h list-spells-prayer.h
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
