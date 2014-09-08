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
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h attack.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h mon-make.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-msg.h mon-util.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-slays.h obj-ui.h obj-util.h \
 player-util.h project.h list-project-environs.h list-project-monsters.h \
 tables.h target.h ui-map.h
./birth.o: birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h birth.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h dungeon.h history.h init.h \
 parser.h monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-make.h obj-power.h \
 obj-randart.h obj-tval.h list-tvals.h obj-util.h player-spell.h \
 player-timed.h list-player-timed.h player-util.h store.h savefile.h \
 quest.h ui-menu.h ui-player.h
./cave.o: cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h dungeon.h init.h parser.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-ignore.h list-ignore-types.h obj-tval.h \
 list-tvals.h obj-util.h player-timed.h list-player-timed.h tables.h \
 trap.h list-trap-flags.h
./cave-map.o: cave-map.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-ignore.h list-ignore-types.h \
 obj-tval.h list-tvals.h obj-util.h player-timed.h list-player-timed.h \
 tables.h trap.h list-trap-flags.h
./cave-square.o: cave-square.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h dungeon.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-util.h trap.h list-trap-flags.h
./cave-view.o: cave-view.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h init.h parser.h \
 monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h player-timed.h list-player-timed.h
./cmd-cave.o: cmd-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h attack.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h dungeon.h generate.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-util.h obj-chest.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-util.h pathfind.h \
 player-timed.h list-player-timed.h player-util.h trap.h \
 list-trap-flags.h tables.h
./cmd-core.o: cmd-core.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h attack.h birth.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h obj-chest.h obj-desc.h \
 obj-tval.h list-tvals.h obj-ui.h obj-util.h player-spell.h target.h
./cmd-context.o: cmd-context.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h keymap.h mon-lore.h \
 monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-util.h obj-chest.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-info.h obj-tval.h list-tvals.h obj-ui.h \
 obj-util.h player-timed.h list-player-timed.h player-util.h store.h \
 parser.h target.h textui.h ui-menu.h ui-player.h wizard.h
./cmd-know.o: cmd-know.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h dungeon.h history.h init.h parser.h \
 mon-lore.h monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-list.h mon-util.h obj-ignore.h \
 list-ignore-types.h obj-list.h obj-tval.h list-tvals.h obj-ui.h \
 obj-util.h prefs.h target.h ui-map.h ui-menu.h ui-player.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h buildid.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h dungeon.h history.h \
 init.h parser.h mon-util.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h prefs.h obj-util.h target.h \
 ui-player.h wizard.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h attack.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-ignore.h list-ignore-types.h \
 obj-info.h obj-make.h obj-tval.h list-tvals.h obj-ui.h obj-util.h \
 player-spell.h player-timed.h list-player-timed.h player-util.h target.h \
 trap.h list-trap-flags.h ui-menu.h ui-options.h
./cmd-pickup.o: cmd-pickup.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h attack.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h generate.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h history.h mon-util.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-tval.h list-tvals.h obj-ui.h obj-util.h \
 player-util.h tables.h trap.h list-trap-flags.h
./cmd-process.o: cmd-process.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h attack.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h keymap.h \
 monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-gear.h list-equip-slots.h \
 obj-util.h player-timed.h list-player-timed.h player-util.h target.h \
 textui.h ui-game.h ui-help.h ui-map.h ui-menu.h ui-player.h wizard.h
./death.o: death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h death.h history.h init.h \
 parser.h obj-desc.h obj-identify.h obj-info.h obj-ui.h savefile.h \
 score.h store.h ui-menu.h ui-player.h wizard.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h debug.h
./dungeon.o: dungeon.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h birth.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h death.h dungeon.h \
 generate.h mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h grafmode.h init.h parser.h \
 mon-list.h mon-make.h mon-spell.h mon-util.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-randart.h obj-tval.h list-tvals.h \
 obj-util.h pathfind.h player-timed.h list-player-timed.h player-util.h \
 prefs.h savefile.h score.h signals.h store.h tables.h target.h \
 ui-birth.h ui-game.h ui-map.h ui-player.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h init.h parser.h mon-lore.h mon-make.h mon-spell.h \
 mon-util.h obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-make.h obj-power.h \
 obj-tval.h list-tvals.h obj-ui.h obj-util.h player-timed.h \
 list-player-timed.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h tables.h target.h trap.h list-trap-flags.h
./game-event.o: game-event.c z-virt.h h-basic.h game-event.h z-type.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h init.h parser.h mon-make.h mon-spell.h obj-util.h \
 trap.h list-trap-flags.h z-queue.h
./gen-cave.o: gen-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h init.h parser.h mon-make.h mon-spell.h store.h \
 tables.h trap.h list-trap-flags.h z-queue.h
./gen-chunk.o: gen-chunk.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h init.h parser.h mon-make.h obj-util.h trap.h \
 list-trap-flags.h
./gen-monster.o: gen-monster.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h init.h parser.h mon-make.h mon-spell.h
./gen-room.o: gen-room.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h generate.h mon-constants.h list-mon-flags.h \
 monster.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h init.h parser.h \
 mon-make.h mon-spell.h obj-tval.h list-tvals.h tables.h trap.h \
 list-trap-flags.h z-queue.h
./gen-util.o: gen-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h generate.h mon-constants.h list-mon-flags.h \
 monster.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h init.h parser.h \
 mon-make.h mon-spell.h obj-make.h obj-tval.h list-tvals.h obj-util.h \
 tables.h trap.h list-trap-flags.h z-queue.h
./grafmode.o: grafmode.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h grafmode.h init.h parser.h
./guid.o: guid.c guid.h
./history.o: history.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h dungeon.h history.h obj-desc.h \
 obj-util.h cave.h list-square-flags.h list-terrain-flags.h wizard.h
./init.o: init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h buildid.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h generate.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h history.h hint.h keymap.h init.h \
 parser.h mon-init.h mon-list.h mon-msg.h mon-util.h obj-ignore.h \
 list-ignore-types.h obj-list.h obj-make.h obj-randart.h obj-slays.h \
 obj-tval.h list-tvals.h obj-util.h player-spell.h player-timed.h \
 list-player-timed.h prefs.h project.h list-project-environs.h \
 list-project-monsters.h quest.h randname.h store.h trap.h \
 list-trap-flags.h list-equip-slots.h
./keymap.o: keymap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h keymap.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h history.h init.h parser.h mon-make.h mon-spell.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-make.h obj-slays.h obj-util.h player-spell.h \
 player-timed.h list-player-timed.h savefile.h store.h quest.h trap.h \
 list-trap-flags.h ui-game.h
./message.o: message.c z-virt.h h-basic.h z-color.h z-util.h message.h \
 message-list.h game-event.h z-type.h option.h z-file.h list-options.h \
 init.h z-bitflag.h z-form.h z-rand.h parser.h z-dice.h z-expression.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h effects.h \
 list-effects.h list-elements.h list-identify-flags.h list-origins.h \
 player-calcs.h list-player-flags.h
./mon-blow-effects.o: mon-blow-effects.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h \
 config.h game-event.h z-type.h message.h message-list.h option.h \
 z-file.h list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-util.h \
 obj-desc.h obj-gear.h list-equip-slots.h obj-make.h obj-tval.h \
 list-tvals.h obj-util.h player-timed.h list-player-timed.h player-util.h \
 project.h list-project-environs.h list-project-monsters.h
./mon-blow-methods.o: mon-blow-methods.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h \
 config.h game-event.h z-type.h message.h message-list.h option.h \
 z-file.h list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h
./mon-list.o: mon-list.c dungeon.h angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h mon-util.h \
 monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-list.h player-timed.h list-player-timed.h \
 project.h list-project-environs.h list-project-monsters.h
./mon-lore.o: mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h attack.h mon-lore.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-make.h \
 mon-spell.h mon-util.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-tval.h list-tvals.h obj-util.h player-timed.h list-player-timed.h
./mon-make.o: mon-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h alloc.h dungeon.h history.h \
 init.h parser.h target.h mon-lore.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-make.h mon-util.h \
 obj-identify.h obj-make.h obj-tval.h list-tvals.h obj-util.h quest.h \
 tables.h
./mon-melee2.o: mon-melee2.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h attack.h \
 cave.h list-square-flags.h list-terrain-flags.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-make.h mon-spell.h mon-util.h obj-desc.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-slays.h obj-tval.h \
 list-tvals.h obj-util.h player-timed.h list-player-timed.h player-util.h \
 project.h list-project-environs.h list-project-monsters.h tables.h
./mon-msg.o: mon-msg.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h mon-msg.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-util.h \
 init.h parser.h
./mon-power.o: mon-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h init.h \
 parser.h mon-power.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-spell.h prefs.h tables.h
./mon-spell.o: mon-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h mon-spell.h \
 monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-util.h player-timed.h list-player-timed.h \
 player-util.h project.h list-project-environs.h list-project-monsters.h
./mon-timed.o: mon-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h mon-msg.h \
 monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-spell.h mon-util.h
./mon-util.o: mon-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h init.h parser.h mon-make.h \
 monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-msg.h mon-spell.h mon-list.h mon-util.h obj-desc.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-util.h \
 player-timed.h list-player-timed.h player-util.h
./mon-init.o: mon-init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h init.h parser.h mon-init.h \
 mon-msg.h monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-power.h mon-spell.h mon-util.h obj-tval.h \
 list-tvals.h obj-util.h player-spell.h
./obj-chest.o: obj-chest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-util.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-chest.h obj-identify.h obj-make.h obj-tval.h \
 list-tvals.h obj-util.h player-timed.h list-player-timed.h player-util.h \
 tables.h
./obj-desc.o: obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h obj-chest.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-tval.h list-tvals.h obj-util.h cave.h \
 list-square-flags.h list-terrain-flags.h
./obj-gear.o: obj-gear.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-ignore.h list-ignore-types.h \
 obj-tval.h list-tvals.h obj-util.h cave.h list-square-flags.h \
 list-terrain-flags.h player-util.h
./obj-identify.o: obj-identify.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h dungeon.h history.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-slays.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h player-timed.h list-player-timed.h player-util.h
./obj-ignore.o: obj-ignore.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cmds.h \
 cave.h list-square-flags.h list-terrain-flags.h init.h parser.h \
 ui-menu.h obj-desc.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-ignore.h list-ignore-types.h obj-tval.h list-tvals.h obj-ui.h \
 obj-util.h
./obj-info.o: obj-info.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h attack.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h init.h parser.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-info.h obj-make.h obj-slays.h obj-tval.h list-tvals.h obj-util.h \
 project.h list-project-environs.h list-project-monsters.h tables.h
./obj-list.o: obj-list.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h dungeon.h init.h parser.h \
 obj-desc.h obj-identify.h obj-ignore.h list-ignore-types.h obj-list.h \
 obj-tval.h list-tvals.h obj-ui.h obj-util.h cave.h list-square-flags.h \
 list-terrain-flags.h
./obj-make.o: obj-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h alloc.h cave.h \
 list-square-flags.h list-terrain-flags.h init.h parser.h obj-gear.h \
 list-equip-slots.h obj-make.h obj-power.h obj-slays.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-tval.h list-tvals.h obj-util.h
./obj-power.o: obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-power.h obj-slays.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h init.h parser.h mon-power.h
./obj-properties.o: obj-properties.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h \
 config.h game-event.h z-type.h message.h message-list.h option.h \
 z-file.h list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h obj-gear.h \
 list-equip-slots.h
./obj-randart.o: obj-randart.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h init.h \
 parser.h obj-desc.h obj-power.h obj-randart.h obj-slays.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h randname.h wizard.h
./obj-slays.o: obj-slays.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h init.h \
 parser.h obj-desc.h obj-identify.h obj-slays.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h obj-util.h \
 mon-util.h
./obj-tval.o: obj-tval.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h parser.h z-dice.h z-expression.h obj-tval.h object.h \
 z-quark.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h list-tvals.h \
 z-type.h z-util.h
./obj-ui.o: obj-ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h keymap.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-tval.h list-tvals.h obj-ui.h \
 obj-util.h store.h parser.h ui-game.h
./obj-util.o: obj-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h generate.h mon-constants.h \
 list-mon-flags.h monster.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h grafmode.h history.h init.h parser.h mon-make.h \
 obj-desc.h obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-info.h obj-make.h obj-slays.h obj-tval.h \
 list-tvals.h obj-ui.h obj-util.h player-spell.h player-util.h prefs.h \
 randname.h z-queue.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h ui-game.h init.h parser.h
./parser.o: parser.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h parser.h z-dice.h z-expression.h mon-util.h monster.h \
 cave.h z-type.h list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h option.h list-options.h player-calcs.h \
 list-player-flags.h list-blow-effects.h list-mon-spells.h ui-input.h \
 angband.h z-color.h z-util.h z-term.h ui-event.h config.h game-event.h \
 message.h message-list.h ui.h z-textblock.h cmd-core.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h randname.h
./pathfind.o: pathfind.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h pathfind.h mon-util.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-ignore.h list-ignore-types.h obj-util.h \
 player-util.h tables.h
./prefs.o: prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h grafmode.h init.h parser.h keymap.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-ignore.h list-ignore-types.h obj-tval.h \
 list-tvals.h obj-util.h prefs.h project.h list-project-environs.h \
 list-project-monsters.h ui-game.h
./player-calcs.o: player-calcs.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h dungeon.h mon-msg.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-util.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-tval.h list-tvals.h \
 obj-util.h player-spell.h player-timed.h list-player-timed.h \
 player-util.h ui-player.h
./player-class.o: player-class.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-rand.h z-quark.h \
 z-dice.h z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h option.h list-options.h \
 player-calcs.h list-player-flags.h
./player-race.o: player-race.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-rand.h z-quark.h \
 z-dice.h z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h option.h list-options.h \
 player-calcs.h list-player-flags.h
./player-spell.o: player-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h
./player-timed.o: player-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-util.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-identify.h player-timed.h list-player-timed.h \
 player-util.h
./player-util.o: player-util.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h obj-gear.h list-equip-slots.h \
 obj-tval.h list-tvals.h obj-ui.h obj-util.h player-spell.h \
 player-timed.h list-player-timed.h player-util.h tables.h target.h
./player.o: player.c z-color.h h-basic.h z-util.h init.h z-bitflag.h \
 z-form.h z-virt.h z-file.h z-rand.h parser.h z-dice.h z-expression.h \
 history.h angband.h z-term.h ui-event.h config.h game-event.h z-type.h \
 message.h message-list.h option.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h \
 player-timed.h list-player-timed.h player-spell.h obj-util.h cave.h \
 list-square-flags.h list-terrain-flags.h birth.h
./project.o: project.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h generate.h mon-constants.h list-mon-flags.h \
 monster.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-util.h \
 player-timed.h list-player-timed.h project.h list-project-environs.h \
 list-project-monsters.h tables.h ui-map.h
./project-feat.o: project-feat.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h dungeon.h generate.h \
 mon-constants.h list-mon-flags.h monster.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-util.h player-timed.h \
 list-player-timed.h trap.h list-trap-flags.h list-project-environs.h \
 list-project-monsters.h
./project-mon.o: project-mon.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-make.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-msg.h mon-spell.h mon-util.h project.h \
 list-project-environs.h list-project-monsters.h
./project-obj.o: project-obj.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-util.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-tval.h list-tvals.h \
 obj-util.h list-project-environs.h list-project-monsters.h
./project-player.o: project-player.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h \
 config.h game-event.h z-type.h message.h message-list.h option.h \
 z-file.h list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-util.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-gear.h list-equip-slots.h obj-identify.h \
 player-timed.h list-player-timed.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h
./quest.o: quest.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h init.h parser.h \
 quest.h obj-util.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h buildid.h dungeon.h init.h \
 parser.h score.h
./signals.o: signals.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h dungeon.h savefile.h signals.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h history.h init.h parser.h mon-make.h \
 monster.h mon-constants.h list-mon-flags.h mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h quest.h savefile.h store.h obj-util.h \
 player-timed.h list-player-timed.h trap.h list-trap-flags.h ui-game.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h dungeon.h init.h parser.h \
 savefile.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h hint.h history.h init.h parser.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-info.h obj-make.h \
 obj-power.h obj-tval.h list-tvals.h obj-util.h store.h target.h debug.h
./tables.o: tables.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h tables.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h keymap.h mon-lore.h monster.h mon-constants.h \
 list-mon-flags.h mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-spells.h mon-util.h \
 obj-desc.h obj-ignore.h list-ignore-types.h obj-ui.h obj-util.h \
 player-timed.h list-player-timed.h project.h list-project-environs.h \
 list-project-monsters.h tables.h target.h trap.h list-trap-flags.h \
 ui-map.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h attack.h cave.h \
 list-square-flags.h list-terrain-flags.h dungeon.h init.h parser.h \
 player-util.h trap.h list-trap-flags.h
./ui.o: ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-color.h \
 z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h tables.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h obj-tval.h list-tvals.h \
 target.h ui-birth.h ui-game.h ui-help.h ui-menu.h ui-options.h \
 ui-player.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h
./ui-game.o: ui-game.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h buildid.h cave.h \
 list-square-flags.h list-terrain-flags.h grafmode.h hint.h init.h \
 parser.h mon-lore.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-list.h mon-util.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-list.h obj-ui.h obj-util.h \
 player-timed.h list-player-timed.h player-util.h prefs.h project.h \
 list-project-environs.h list-project-monsters.h textui.h ui-birth.h \
 ui-map.h ui-player.h
./ui-help.o: ui-help.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h buildid.h init.h parser.h
./ui-input.o: ui-input.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h dungeon.h target.h init.h \
 parser.h pathfind.h randname.h signals.h ui-player.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h history.h init.h \
 parser.h mon-lore.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h mon-util.h obj-desc.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-info.h obj-make.h \
 obj-tval.h list-tvals.h obj-util.h score.h store.h tables.h target.h \
 ui-menu.h ui-options.h grafmode.h wizard.h
./ui-map.o: ui-map.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h grafmode.h init.h parser.h mon-util.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-ui.h obj-util.h player-timed.h list-player-timed.h \
 prefs.h trap.h list-trap-flags.h ui-map.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h tables.h target.h ui-menu.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cmds.h \
 cave.h list-square-flags.h list-terrain-flags.h init.h parser.h keymap.h \
 prefs.h obj-desc.h obj-ignore.h list-ignore-types.h obj-tval.h \
 list-tvals.h obj-util.h tables.h target.h ui-game.h ui-help.h ui-menu.h \
 ui-options.h
./ui-player.o: ui-player.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h buildid.h \
 dungeon.h history.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-info.h obj-ui.h obj-util.h cave.h list-square-flags.h \
 list-terrain-flags.h player-timed.h list-player-timed.h player-util.h \
 store.h parser.h ui-game.h ui-menu.h ui-player.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h obj-tval.h list-tvals.h obj-ui.h obj-util.h \
 player-spell.h ui-menu.h
./ui-store.o: ui-store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h hint.h history.h init.h parser.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-info.h obj-make.h \
 obj-tval.h list-tvals.h obj-ui.h obj-util.h store.h target.h textui.h \
 ui-game.h ui-menu.h z-debug.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h buildid.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h dungeon.h init.h \
 parser.h mon-lore.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-desc.h obj-identify.h \
 obj-info.h obj-make.h obj-power.h obj-tval.h list-tvals.h obj-util.h \
 ui-menu.h wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h \
 game-event.h z-type.h message.h message-list.h option.h z-file.h \
 list-options.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h z-dice.h z-expression.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h player-calcs.h \
 list-player-flags.h ui.h z-textblock.h ui-input.h cmd-core.h cave.h \
 list-square-flags.h list-terrain-flags.h cmds.h dungeon.h init.h \
 parser.h wizard.h mon-make.h monster.h mon-constants.h list-mon-flags.h \
 mon-timed.h mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-spells.h obj-randart.h obj-tval.h \
 list-tvals.h obj-util.h generate.h tables.h
./wizard.o: wizard.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h z-term.h ui-event.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 ui.h z-textblock.h ui-input.h cmd-core.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h init.h parser.h mon-lore.h monster.h \
 mon-constants.h list-mon-flags.h mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-spells.h mon-make.h mon-util.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-make.h obj-power.h obj-tval.h \
 list-tvals.h obj-ui.h obj-util.h player-timed.h list-player-timed.h \
 project.h list-project-environs.h list-project-monsters.h target.h \
 ui-game.h ui-help.h ui-map.h ui-menu.h wizard.h
./buildid.o: buildid.c buildid.h
./z-bitflag.o: z-bitflag.c z-bitflag.h h-basic.h z-form.h z-virt.h
./z-color.o: z-color.c h-basic.h z-color.h z-util.h
./z-dice.o: z-dice.c z-dice.h h-basic.h z-rand.h z-expression.h z-virt.h \
 z-util.h
./z-expression.o: z-expression.c z-expression.h h-basic.h z-virt.h z-util.h
./z-file.o: z-file.c h-basic.h z-file.h z-form.h z-util.h z-virt.h
./z-form.o: z-form.c z-form.h h-basic.h z-type.h z-util.h z-virt.h
./z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h init.h z-bitflag.h \
 z-form.h z-file.h z-rand.h parser.h z-dice.h z-expression.h
./z-queue.o: z-queue.c z-queue.h h-basic.h
./z-rand.o: z-rand.c z-rand.h h-basic.h
./z-set.o: z-set.c z-set.h h-basic.h z-rand.h z-virt.h
./z-term.o: z-term.c buildid.h h-basic.h z-color.h z-term.h ui-event.h \
 z-util.h z-virt.h
./z-textblock.o: z-textblock.c z-color.h h-basic.h z-textblock.h z-file.h \
 z-util.h z-virt.h z-form.h
./z-type.o: z-type.c z-type.h h-basic.h z-virt.h
./z-util.o: z-util.c z-util.h h-basic.h
./z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
