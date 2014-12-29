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
./cave.o: cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h \
 dungeon.h init.h parser.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-ignore.h list-ignore-types.h obj-pile.h obj-tval.h \
 list-tvals.h obj-util.h player-timed.h list-player-timed.h tables.h \
 trap.h list-trap-flags.h
./cave-map.o: cave-map.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h init.h parser.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 obj-ignore.h list-ignore-types.h obj-tval.h list-tvals.h obj-util.h \
 player-timed.h list-player-timed.h tables.h trap.h list-trap-flags.h
./cave-square.o: cave-square.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h init.h parser.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-util.h obj-pile.h trap.h list-trap-flags.h
./cave-view.o: cave-view.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h dungeon.h init.h parser.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h player-timed.h \
 list-player-timed.h
./cmd-cave.o: cmd-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmd-core.h cmds.h \
 dungeon.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h parser.h mon-desc.h mon-lore.h z-textblock.h \
 mon-util.h obj-chest.h obj-identify.h obj-ignore.h list-ignore-types.h \
 obj-util.h player-attack.h player-path.h player-timed.h \
 list-player-timed.h player-util.h store.h tables.h trap.h \
 list-trap-flags.h ui-input.h z-term.h ui-event.h ui-store.h
./cmd-core.o: cmd-core.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 obj-chest.h obj-desc.h obj-tval.h list-tvals.h obj-ui.h obj-util.h \
 player-attack.h player-birth.h player-spell.h store.h parser.h target.h \
 ui-event.h ui-target.h ui-spell.h ui-input.h z-term.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 buildid.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 cmds.h dungeon.h init.h parser.h mon-lore.h z-textblock.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-util.h player-history.h \
 list-history-types.h prefs.h obj-util.h target.h ui-event.h ui-input.h \
 z-term.h wizard.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmd-core.h cmds.h init.h \
 parser.h obj-desc.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-ignore.h list-ignore-types.h obj-info.h z-textblock.h obj-make.h \
 obj-pile.h obj-tval.h list-tvals.h obj-ui.h obj-util.h player-attack.h \
 player-spell.h player-timed.h list-player-timed.h player-util.h target.h \
 ui-event.h trap.h list-trap-flags.h ui-input.h z-term.h
./cmd-pickup.o: cmd-pickup.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h parser.h mon-lore.h z-textblock.h mon-util.h \
 obj-desc.h obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-pile.h obj-tval.h list-tvals.h obj-ui.h \
 obj-util.h player-attack.h player-history.h list-history-types.h \
 player-util.h tables.h trap.h list-trap-flags.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 debug.h
./dungeon.o: dungeon.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h \
 dungeon.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h grafmode.h init.h parser.h mon-list.h mon-lore.h \
 z-textblock.h mon-make.h mon-move.h mon-spell.h mon-util.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-pile.h obj-randart.h \
 obj-tval.h list-tvals.h obj-util.h player-birth.h player-history.h \
 list-history-types.h player-path.h player-timed.h list-player-timed.h \
 player-util.h prefs.h savefile.h score.h signals.h store.h tables.h \
 target.h ui-event.h ui-birth.h ui-death.h ui-display.h ui-input.h \
 z-term.h ui-knowledge.h ui-map.h ui-player.h ui.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h generate.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h mon-desc.h mon-lore.h z-textblock.h mon-make.h mon-spell.h \
 mon-summon.h list-summon-types.h mon-util.h obj-chest.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-make.h obj-pile.h obj-power.h obj-tval.h \
 list-tvals.h obj-ui.h cmd-core.h obj-util.h player-timed.h \
 list-player-timed.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h tables.h target.h ui-event.h trap.h \
 list-trap-flags.h ui-input.h z-term.h
./game-event.o: game-event.c z-virt.h h-basic.h game-event.h z-type.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h generate.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h mon-make.h mon-spell.h obj-util.h trap.h list-trap-flags.h \
 ui-input.h z-term.h ui-event.h cmd-core.h z-queue.h list-dun-profiles.h \
 list-rooms.h
./gen-cave.o: gen-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h generate.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h mon-make.h mon-spell.h store.h cmd-core.h tables.h trap.h \
 list-trap-flags.h z-queue.h
./gen-chunk.o: gen-chunk.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 generate.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h mon-make.h obj-util.h trap.h list-trap-flags.h
./gen-monster.o: gen-monster.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 generate.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h mon-make.h mon-spell.h
./gen-room.o: gen-room.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h generate.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h init.h parser.h mon-make.h \
 mon-spell.h obj-tval.h list-tvals.h tables.h trap.h list-trap-flags.h \
 z-queue.h
./gen-util.o: gen-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h generate.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h init.h parser.h mon-make.h \
 mon-spell.h obj-make.h obj-pile.h obj-tval.h list-tvals.h obj-util.h \
 tables.h trap.h list-trap-flags.h z-queue.h
./grafmode.o: grafmode.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 grafmode.h init.h parser.h ui-input.h z-term.h ui-event.h cmd-core.h
./guid.o: guid.c guid.h
./init.o: init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 buildid.h cave.h list-square-flags.h list-terrain-flags.h cmds.h \
 cmd-core.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h hint.h keymap.h init.h parser.h mon-init.h mon-list.h \
 mon-lore.h z-textblock.h mon-msg.h mon-util.h obj-ignore.h \
 list-ignore-types.h obj-list.h obj-make.h obj-randart.h obj-slays.h \
 obj-tval.h list-tvals.h obj-util.h player-history.h list-history-types.h \
 player-quest.h player-spell.h player-timed.h list-player-timed.h prefs.h \
 project.h list-project-environs.h list-project-monsters.h randname.h \
 store.h trap.h list-trap-flags.h list-equip-slots.h
./keymap.o: keymap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 keymap.h z-term.h ui-event.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h generate.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h mon-lore.h z-textblock.h mon-make.h mon-spell.h mon-util.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-make.h obj-pile.h obj-randart.h obj-slays.h \
 obj-util.h player-history.h list-history-types.h player-quest.h \
 player-spell.h player-timed.h list-player-timed.h savefile.h store.h \
 cmd-core.h trap.h list-trap-flags.h ui-display.h ui-input.h z-term.h \
 ui-event.h
./message.o: message.c z-virt.h h-basic.h z-color.h z-util.h message.h \
 message-list.h game-event.h z-type.h option.h z-file.h list-options.h \
 init.h z-bitflag.h z-form.h z-rand.h parser.h z-dice.h z-expression.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h effects.h \
 list-effects.h list-elements.h list-identify-flags.h list-origins.h \
 player-calcs.h list-player-flags.h list-magic-realms.h
./mon-attack.o: mon-attack.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 init.h parser.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h monster.h mon-timed.h list-mon-timed.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 list-blow-effects.h mon-desc.h mon-lore.h z-textblock.h mon-spell.h \
 mon-util.h obj-identify.h player-attack.h cmd-core.h player-timed.h \
 list-player-timed.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h
./mon-blow-effects.o: mon-blow-effects.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h init.h parser.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 mon-attack.h mon-lore.h z-textblock.h mon-util.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-make.h obj-pile.h obj-tval.h list-tvals.h \
 obj-util.h player-timed.h list-player-timed.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h
./mon-blow-methods.o: mon-blow-methods.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h
./mon-desc.o: mon-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 mon-desc.h monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h z-term.h ui-event.h
./mon-init.o: mon-init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 init.h parser.h mon-init.h mon-lore.h z-textblock.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-msg.h mon-power.h mon-spell.h mon-util.h \
 obj-tval.h list-tvals.h obj-util.h player-spell.h prefs.h
./mon-list.o: mon-list.c dungeon.h angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h mon-desc.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-list.h \
 mon-lore.h z-textblock.h mon-util.h player-timed.h list-player-timed.h \
 project.h list-project-environs.h list-project-monsters.h z-term.h \
 ui-event.h ui.h
./mon-lore.o: mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 init.h parser.h mon-blow-effects.h mon-blow-methods.h \
 list-blow-methods.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h list-blow-effects.h mon-init.h \
 mon-lore.h z-textblock.h mon-make.h mon-spell.h mon-util.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-tval.h list-tvals.h obj-util.h \
 player-attack.h cmd-core.h player-timed.h list-player-timed.h ui-input.h \
 z-term.h ui-event.h ui.h
./mon-make.o: mon-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 alloc.h dungeon.h init.h parser.h target.h ui-event.h mon-desc.h \
 monster.h cave.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-lore.h z-textblock.h \
 mon-make.h mon-util.h obj-identify.h obj-make.h obj-pile.h obj-tval.h \
 list-tvals.h obj-util.h player-history.h list-history-types.h \
 player-quest.h tables.h
./mon-move.o: mon-move.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h init.h \
 parser.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 mon-attack.h mon-desc.h mon-lore.h z-textblock.h mon-make.h mon-spell.h \
 mon-util.h obj-desc.h obj-ignore.h list-ignore-types.h obj-pile.h \
 obj-slays.h obj-tval.h list-tvals.h obj-util.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h tables.h trap.h \
 list-trap-flags.h
./mon-msg.o: mon-msg.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 init.h parser.h mon-desc.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-lore.h \
 z-textblock.h mon-msg.h mon-util.h
./mon-power.o: mon-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h init.h parser.h mon-init.h mon-power.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-spell.h obj-tval.h \
 list-tvals.h obj-util.h prefs.h tables.h z-textblock.h
./mon-spell.o: mon-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h mon-attack.h mon-desc.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h mon-make.h mon-spell.h \
 mon-util.h player-timed.h list-player-timed.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h
./mon-summon.o: mon-summon.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h mon-make.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 mon-summon.h list-summon-types.h mon-util.h
./mon-timed.o: mon-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h mon-desc.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-lore.h \
 z-textblock.h mon-msg.h mon-spell.h mon-util.h
./mon-util.o: mon-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 init.h parser.h mon-lore.h z-textblock.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-make.h mon-msg.h mon-spell.h mon-list.h mon-util.h \
 obj-desc.h obj-identify.h obj-ignore.h list-ignore-types.h obj-pile.h \
 obj-util.h player-timed.h list-player-timed.h player-util.h
./obj-chest.o: obj-chest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-lore.h z-textblock.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-chest.h obj-identify.h obj-make.h obj-pile.h \
 obj-tval.h list-tvals.h obj-util.h player-timed.h list-player-timed.h \
 player-util.h tables.h ui-input.h z-term.h ui-event.h cmd-core.h
./obj-desc.o: obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-ignore.h list-ignore-types.h obj-tval.h list-tvals.h obj-util.h
./obj-gear.o: obj-gear.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cmd-core.h init.h parser.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-pile.h cave.h \
 list-square-flags.h list-terrain-flags.h obj-tval.h list-tvals.h \
 obj-util.h player-util.h
./obj-identify.o: obj-identify.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h obj-desc.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-ignore.h list-ignore-types.h obj-slays.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-tval.h list-tvals.h \
 obj-util.h player-history.h list-history-types.h player-timed.h \
 list-player-timed.h player-util.h
./obj-ignore.o: obj-ignore.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h init.h parser.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-ignore.h list-ignore-types.h \
 obj-tval.h list-tvals.h obj-ui.h obj-util.h
./obj-info.o: obj-info.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h init.h \
 parser.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-info.h z-textblock.h obj-make.h \
 obj-slays.h obj-tval.h list-tvals.h obj-util.h player-attack.h project.h \
 list-project-environs.h list-project-monsters.h tables.h
./obj-list.o: obj-list.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 dungeon.h init.h parser.h obj-desc.h obj-identify.h obj-ignore.h \
 list-ignore-types.h obj-list.h obj-pile.h cave.h list-square-flags.h \
 list-terrain-flags.h obj-tval.h list-tvals.h obj-ui.h cmd-core.h \
 obj-util.h z-textblock.h ui.h z-term.h ui-event.h
./obj-make.o: obj-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 alloc.h cave.h list-square-flags.h list-terrain-flags.h init.h parser.h \
 obj-gear.h list-equip-slots.h obj-pile.h obj-make.h obj-power.h \
 obj-slays.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h
./obj-pile.o: obj-pile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h cmd-core.h \
 generate.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h grafmode.h \
 init.h parser.h mon-make.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-info.h z-textblock.h \
 obj-make.h obj-pile.h obj-slays.h obj-tval.h list-tvals.h obj-ui.h \
 obj-util.h player-history.h list-history-types.h player-spell.h \
 player-util.h prefs.h randname.h z-queue.h
./obj-power.o: obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-power.h obj-slays.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h init.h parser.h mon-power.h
./obj-properties.o: obj-properties.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h obj-gear.h list-equip-slots.h
./obj-randart.o: obj-randart.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h init.h parser.h obj-desc.h obj-power.h obj-randart.h \
 obj-slays.h monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-tval.h list-tvals.h \
 obj-util.h randname.h wizard.h
./obj-slays.o: obj-slays.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h init.h parser.h mon-lore.h z-textblock.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-desc.h obj-identify.h \
 obj-slays.h obj-util.h
./obj-tval.o: obj-tval.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h parser.h z-dice.h z-expression.h obj-tval.h object.h \
 z-quark.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h list-tvals.h \
 z-type.h z-util.h
./obj-ui.o: obj-ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cmd-core.h cmds.h cave.h list-square-flags.h list-terrain-flags.h init.h \
 parser.h keymap.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-info.h z-textblock.h obj-make.h obj-pile.h obj-tval.h \
 list-tvals.h obj-ui.h obj-util.h store.h ui-display.h ui-input.h \
 z-term.h ui-event.h ui.h
./obj-util.o: obj-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h cmd-core.h \
 generate.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h grafmode.h \
 init.h parser.h mon-make.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-make.h obj-pile.h \
 obj-slays.h obj-tval.h list-tvals.h obj-util.h player-history.h \
 list-history-types.h player-spell.h player-util.h prefs.h randname.h \
 z-queue.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 ui-display.h init.h parser.h z-term.h ui-event.h
./parser.o: parser.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h parser.h z-dice.h z-expression.h message.h \
 message-list.h mon-util.h monster.h cave.h z-type.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h player.h guid.h obj-properties.h \
 list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h option.h \
 list-options.h player-calcs.h list-player-flags.h list-magic-realms.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h ui-input.h z-term.h ui-event.h game-event.h cmd-core.h \
 z-util.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 randname.h
./prefs.o: prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h grafmode.h init.h \
 parser.h keymap.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-ignore.h list-ignore-types.h obj-tval.h \
 list-tvals.h obj-util.h prefs.h project.h list-project-environs.h \
 list-project-monsters.h ui-display.h z-term.h ui-event.h ui-input.h \
 cmd-core.h
./player-attack.o: player-attack.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h init.h parser.h mon-desc.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-lore.h z-textblock.h \
 mon-make.h mon-msg.h mon-util.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-pile.h obj-slays.h obj-ui.h obj-util.h \
 player-attack.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h tables.h target.h ui-event.h ui-input.h z-term.h \
 ui-map.h ui-target.h
./player-birth.o: player-birth.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cmd-core.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h dungeon.h init.h parser.h mon-lore.h z-textblock.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-ignore.h list-ignore-types.h \
 obj-make.h obj-pile.h obj-power.h obj-randart.h obj-tval.h list-tvals.h \
 obj-util.h player-birth.h player-history.h list-history-types.h \
 player-quest.h player-spell.h player-timed.h list-player-timed.h \
 player-util.h savefile.h store.h ui-input.h z-term.h ui-event.h
./player-calcs.o: player-calcs.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h init.h parser.h mon-msg.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-util.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-ignore.h list-ignore-types.h \
 obj-tval.h list-tvals.h obj-util.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h ui-player.h ui.h z-term.h ui-event.h \
 z-textblock.h
./player-class.o: player-class.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-rand.h z-quark.h \
 z-dice.h z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h option.h list-options.h \
 player-calcs.h list-player-flags.h list-magic-realms.h
./player-history.o: player-history.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h obj-desc.h obj-pile.h obj-util.h player-history.h \
 list-history-types.h wizard.h
./player-path.o: player-path.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h mon-util.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-ignore.h list-ignore-types.h \
 obj-util.h player-path.h player-util.h tables.h ui-input.h z-term.h \
 ui-event.h
./player-quest.o: player-quest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h init.h parser.h mon-util.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-pile.h obj-util.h player-quest.h
./player-race.o: player-race.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-rand.h z-quark.h \
 z-dice.h z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h option.h list-options.h \
 player-calcs.h list-player-flags.h list-magic-realms.h
./player-spell.o: player-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h init.h parser.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-tval.h list-tvals.h obj-util.h player-spell.h \
 player-timed.h list-player-timed.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h ui-input.h z-term.h \
 ui-event.h
./player-timed.o: player-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-util.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 obj-identify.h player-timed.h list-player-timed.h player-util.h
./player-util.o: player-util.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h init.h parser.h obj-gear.h list-equip-slots.h obj-tval.h \
 list-tvals.h obj-ui.h obj-util.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h tables.h target.h ui-event.h ui.h \
 z-term.h z-textblock.h ui-input.h
./player.o: player.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h parser.h z-dice.h z-expression.h obj-pile.h cave.h \
 z-type.h list-square-flags.h list-terrain-flags.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h effects.h list-effects.h \
 list-elements.h list-identify-flags.h list-origins.h option.h \
 list-options.h player-calcs.h list-player-flags.h list-magic-realms.h \
 obj-util.h player-birth.h cmd-core.h player-history.h angband.h \
 z-color.h z-util.h config.h game-event.h message.h message-list.h \
 list-history-types.h player-quest.h player-spell.h player-timed.h \
 list-player-timed.h ui-input.h z-term.h ui-event.h
./project.o: project.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h generate.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h init.h parser.h mon-util.h \
 player-timed.h list-player-timed.h project.h list-project-environs.h \
 list-project-monsters.h tables.h ui-map.h z-term.h ui-event.h
./project-feat.o: project-feat.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 dungeon.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-pile.h obj-util.h player-timed.h \
 list-player-timed.h trap.h list-trap-flags.h list-project-environs.h \
 list-project-monsters.h
./project-mon.o: project-mon.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-desc.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-lore.h \
 z-textblock.h mon-make.h mon-move.h mon-msg.h mon-spell.h mon-util.h \
 project.h list-project-environs.h list-project-monsters.h
./project-obj.o: project-obj.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-util.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-ignore.h list-ignore-types.h obj-pile.h obj-tval.h list-tvals.h \
 obj-util.h list-project-environs.h list-project-monsters.h
./project-player.o: project-player.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h message-list.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 init.h parser.h mon-desc.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-gear.h list-equip-slots.h obj-identify.h \
 player-timed.h list-player-timed.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 buildid.h dungeon.h init.h parser.h score.h ui-input.h z-term.h \
 ui-event.h cmd-core.h ui.h z-textblock.h
./signals.o: signals.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 dungeon.h savefile.h signals.h z-term.h ui-event.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h init.h \
 parser.h mon-lore.h z-textblock.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-make.h obj-pile.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h savefile.h store.h cmd-core.h \
 obj-util.h player-history.h list-history-types.h player-timed.h \
 list-player-timed.h trap.h list-trap-flags.h ui-display.h ui-input.h \
 z-term.h ui-event.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 dungeon.h init.h parser.h savefile.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h hint.h \
 init.h parser.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-info.h z-textblock.h \
 obj-make.h obj-pile.h obj-power.h obj-slays.h obj-tval.h list-tvals.h \
 obj-util.h player-history.h list-history-types.h store.h target.h \
 ui-event.h debug.h
./tables.o: tables.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 tables.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmd-core.h keymap.h \
 mon-desc.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 obj-ignore.h list-ignore-types.h player-timed.h list-player-timed.h \
 project.h list-project-environs.h list-project-monsters.h target.h \
 ui-event.h ui-target.h z-term.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h dungeon.h init.h \
 parser.h player-attack.h cmd-core.h player-util.h trap.h \
 list-trap-flags.h
./ui.o: ui.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h z-color.h \
 z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h tables.h ui-input.h \
 z-term.h ui-event.h cmd-core.h ui.h z-textblock.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 obj-tval.h list-tvals.h target.h ui-event.h ui-birth.h ui-display.h \
 ui-help.h ui-menu.h ui.h z-term.h z-textblock.h ui-options.h \
 obj-ignore.h list-ignore-types.h ui-player.h ui-input.h
./ui-command.o: ui-command.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h buildid.h dungeon.h init.h parser.h keymap.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-gear.h list-equip-slots.h \
 obj-util.h prefs.h player-attack.h player-timed.h list-player-timed.h \
 player-util.h target.h ui-event.h textui.h store.h ui.h z-term.h \
 z-textblock.h ui-display.h ui-help.h ui-input.h ui-map.h ui-menu.h \
 ui-options.h obj-ignore.h list-ignore-types.h ui-player.h ui-target.h \
 wizard.h
./ui-context.o: ui-context.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h cmds.h keymap.h mon-desc.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-lore.h z-textblock.h \
 mon-util.h obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-info.h obj-tval.h \
 list-tvals.h obj-ui.h obj-util.h player-timed.h list-player-timed.h \
 player-util.h store.h parser.h target.h ui-event.h textui.h ui-context.h \
 ui-input.h z-term.h ui-knowledge.h ui-menu.h ui.h ui-object.h \
 ui-player.h ui-spell.h ui-store.h ui-target.h wizard.h
./ui-death.o: ui-death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h init.h \
 parser.h obj-desc.h obj-identify.h obj-info.h z-textblock.h obj-ui.h \
 savefile.h score.h store.h ui-death.h ui-history.h ui-input.h z-term.h \
 ui-event.h ui-knowledge.h ui-menu.h ui.h ui-player.h wizard.h
./ui-display.o: ui-display.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h grafmode.h hint.h init.h parser.h \
 mon-list.h mon-lore.h z-textblock.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-util.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-list.h obj-pile.h obj-ui.h obj-util.h \
 player-timed.h list-player-timed.h player-util.h prefs.h project.h \
 list-project-environs.h list-project-monsters.h textui.h ui-birth.h \
 ui-input.h z-term.h ui-event.h ui-map.h ui-player.h ui.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 ui-event.h
./ui-help.o: ui-help.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 buildid.h init.h parser.h ui-input.h z-term.h ui-event.h cmd-core.h ui.h \
 z-textblock.h
./ui-history.o: ui-history.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h player-history.h list-history-types.h ui-input.h \
 z-term.h ui-event.h cmd-core.h ui.h z-textblock.h
./ui-input.o: ui-input.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 dungeon.h init.h parser.h keymap.h obj-gear.h list-equip-slots.h \
 obj-util.h player-attack.h player-path.h player-util.h randname.h \
 signals.h target.h ui-event.h ui-command.h ui-context.h ui-input.h \
 z-term.h ui-display.h ui-help.h ui-knowledge.h ui-map.h ui-object.h \
 ui-player.h ui-spell.h ui-store.h ui-target.h ui.h z-textblock.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h grafmode.h init.h parser.h mon-list.h mon-lore.h \
 z-textblock.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 obj-desc.h obj-identify.h obj-ignore.h list-ignore-types.h obj-info.h \
 obj-list.h obj-make.h obj-pile.h obj-tval.h list-tvals.h obj-ui.h \
 obj-util.h player-history.h list-history-types.h score.h store.h \
 tables.h target.h ui-event.h ui-context.h ui-input.h z-term.h \
 ui-history.h ui-menu.h ui.h ui-options.h ui-store.h ui-target.h wizard.h
./ui-map.o: ui-map.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h grafmode.h init.h \
 parser.h mon-util.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-ui.h cmd-core.h obj-util.h player-timed.h \
 list-player-timed.h prefs.h trap.h list-trap-flags.h ui-input.h z-term.h \
 ui-event.h ui-map.h ui.h z-textblock.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 tables.h target.h ui-event.h ui-menu.h ui.h z-term.h z-textblock.h \
 ui-input.h cmd-core.h
./ui-object.o: ui-object.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h init.h parser.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-identify.h obj-ignore.h list-ignore-types.h \
 obj-info.h z-textblock.h obj-make.h obj-pile.h obj-tval.h list-tvals.h \
 obj-ui.h obj-util.h player-attack.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h ui-command.h ui-menu.h ui.h z-term.h \
 ui-event.h ui-options.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h init.h parser.h keymap.h prefs.h \
 obj-desc.h obj-ignore.h list-ignore-types.h obj-tval.h list-tvals.h \
 obj-util.h tables.h target.h ui-event.h ui-display.h ui-help.h \
 ui-input.h z-term.h ui-menu.h ui.h z-textblock.h ui-options.h \
 ui-knowledge.h
./ui-player.o: ui-player.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h buildid.h dungeon.h init.h parser.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-identify.h obj-info.h z-textblock.h \
 obj-ui.h cmd-core.h obj-util.h player-timed.h list-player-timed.h \
 player-util.h store.h cave.h list-square-flags.h list-terrain-flags.h \
 ui-display.h ui-history.h ui-menu.h ui.h z-term.h ui-event.h ui-player.h \
 ui-input.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h \
 obj-tval.h list-tvals.h obj-ui.h obj-util.h player-spell.h ui.h z-term.h \
 ui-event.h z-textblock.h ui-menu.h
./ui-store.o: ui-store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 message-list.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 effects.h list-effects.h list-elements.h list-identify-flags.h \
 list-origins.h player-calcs.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h hint.h \
 init.h parser.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-identify.h obj-ignore.h list-ignore-types.h obj-info.h z-textblock.h \
 obj-make.h obj-pile.h obj-tval.h list-tvals.h obj-ui.h obj-util.h \
 player-history.h list-history-types.h store.h target.h ui-event.h \
 textui.h ui-display.h ui-input.h z-term.h ui-menu.h ui.h ui-object.h \
 ui-options.h ui-knowledge.h ui-player.h ui-spell.h ui-command.h \
 ui-store.h z-debug.h
./ui-target.o: ui-target.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 init.h parser.h mon-desc.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h obj-desc.h obj-pile.h \
 obj-ui.h cmd-core.h obj-util.h player-attack.h player-timed.h \
 list-player-timed.h project.h list-project-environs.h \
 list-project-monsters.h tables.h target.h ui-event.h trap.h \
 list-trap-flags.h ui.h z-term.h ui-map.h ui-target.h ui-input.h
./wiz-debug.o: wiz-debug.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h grafmode.h init.h parser.h mon-lore.h z-textblock.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-make.h \
 mon-util.h obj-desc.h obj-gear.h list-equip-slots.h obj-identify.h \
 obj-make.h obj-pile.h obj-power.h obj-slays.h obj-tval.h list-tvals.h \
 obj-ui.h obj-util.h player-timed.h list-player-timed.h prefs.h project.h \
 list-project-environs.h list-project-monsters.h target.h ui-event.h \
 ui-command.h ui-display.h ui-help.h ui-input.h z-term.h ui-map.h \
 ui-menu.h ui.h ui-target.h wizard.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h buildid.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h dungeon.h init.h parser.h mon-lore.h \
 z-textblock.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-desc.h \
 obj-identify.h obj-info.h obj-make.h obj-pile.h obj-power.h obj-tval.h \
 list-tvals.h obj-util.h ui-input.h z-term.h ui-event.h ui-knowledge.h \
 ui-menu.h ui.h wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h message-list.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h effects.h list-effects.h list-elements.h \
 list-identify-flags.h list-origins.h player-calcs.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h dungeon.h init.h parser.h wizard.h mon-make.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-pile.h \
 obj-randart.h obj-tval.h list-tvals.h obj-util.h generate.h tables.h \
 ui-command.h ui-input.h z-term.h ui-event.h
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
