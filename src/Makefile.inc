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
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h \
 game-world.h init.h parser.h list-parser-errors.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-ignore.h list-ignore-types.h \
 obj-pile.h obj-tval.h list-tvals.h obj-util.h player-timed.h \
 list-player-timed.h trap.h list-trap-flags.h
./cave-map.o: cave-map.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h init.h parser.h \
 list-parser-errors.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-ignore.h list-ignore-types.h obj-pile.h obj-tval.h \
 list-tvals.h obj-util.h player-calcs.h player-timed.h \
 list-player-timed.h trap.h list-trap-flags.h
./cave-square.o: cave-square.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h init.h parser.h list-parser-errors.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-util.h obj-pile.h trap.h \
 list-trap-flags.h
./cave-view.o: cave-view.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h init.h parser.h list-parser-errors.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h player-calcs.h player-timed.h \
 list-player-timed.h
./cmd-cave.o: cmd-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmd-core.h cmds.h \
 game-input.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h parser.h list-parser-errors.h mon-desc.h \
 mon-lore.h z-textblock.h mon-util.h obj-chest.h obj-ignore.h \
 list-ignore-types.h obj-pile.h obj-util.h player-attack.h player-calcs.h \
 player-path.h player-timed.h list-player-timed.h player-util.h store.h \
 trap.h list-trap-flags.h
./cmd-core.o: cmd-core.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 game-input.h obj-chest.h obj-desc.h obj-tval.h list-tvals.h obj-util.h \
 player-attack.h player-birth.h player-calcs.h player-spell.h store.h \
 parser.h list-parser-errors.h target.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 buildid.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 cmds.h game-input.h init.h parser.h list-parser-errors.h mon-lore.h \
 z-textblock.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 player-calcs.h player-history.h list-history-types.h obj-util.h target.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmd-core.h cmds.h \
 effects.h list-effects.h game-input.h init.h parser.h \
 list-parser-errors.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-info.h z-textblock.h \
 obj-knowledge.h obj-make.h obj-pile.h obj-tval.h list-tvals.h obj-util.h \
 player-attack.h player-calcs.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h target.h trap.h list-trap-flags.h
./cmd-pickup.o: cmd-pickup.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h game-input.h generate.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h init.h parser.h \
 list-parser-errors.h mon-lore.h z-textblock.h mon-util.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-ignore.h list-ignore-types.h \
 obj-pile.h obj-tval.h list-tvals.h obj-util.h player-attack.h \
 player-calcs.h player-history.h list-history-types.h player-util.h \
 trap.h list-trap-flags.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 debug.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h effects.h list-effects.h \
 game-input.h cmd-core.h generate.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h init.h parser.h \
 list-parser-errors.h mon-desc.h mon-lore.h z-textblock.h mon-make.h \
 mon-spell.h mon-summon.h list-summon-types.h mon-util.h obj-chest.h \
 obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-knowledge.h obj-make.h obj-pile.h obj-power.h \
 obj-tval.h list-tvals.h obj-util.h player-calcs.h player-history.h \
 list-history-types.h player-timed.h list-player-timed.h player-util.h \
 project.h list-project-environs.h list-project-monsters.h target.h \
 trap.h list-trap-flags.h
./game-event.o: game-event.c game-event.h z-type.h h-basic.h object.h \
 z-rand.h z-quark.h z-bitflag.h z-form.h z-virt.h z-dice.h z-expression.h \
 obj-properties.h z-file.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h list-elements.h list-origins.h
./game-input.o: game-input.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cmd-core.h
./game-world.o: game-world.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h effects.h list-effects.h game-world.h \
 init.h parser.h list-parser-errors.h mon-make.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-move.h mon-util.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-knowledge.h obj-tval.h list-tvals.h \
 obj-util.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h target.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h game-input.h cmd-core.h \
 game-world.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h parser.h list-parser-errors.h mon-make.h \
 mon-spell.h obj-tval.h list-tvals.h obj-util.h player-history.h \
 list-history-types.h trap.h list-trap-flags.h z-queue.h \
 list-dun-profiles.h list-rooms.h
./gen-cave.o: gen-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h game-world.h generate.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h list-parser-errors.h mon-make.h mon-spell.h player-util.h \
 store.h cmd-core.h trap.h list-trap-flags.h z-queue.h
./gen-chunk.o: gen-chunk.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 generate.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h list-parser-errors.h mon-make.h obj-util.h trap.h \
 list-trap-flags.h
./gen-monster.o: gen-monster.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 generate.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h list-parser-errors.h mon-make.h mon-spell.h
./gen-room.o: gen-room.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h generate.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h init.h parser.h \
 list-parser-errors.h mon-make.h mon-spell.h obj-tval.h list-tvals.h \
 trap.h list-trap-flags.h z-queue.h
./gen-util.o: gen-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h generate.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h init.h parser.h \
 list-parser-errors.h mon-make.h mon-spell.h obj-make.h obj-pile.h \
 obj-tval.h list-tvals.h obj-util.h player-util.h trap.h \
 list-trap-flags.h z-queue.h
./grafmode.o: grafmode.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 grafmode.h init.h parser.h list-parser-errors.h
./guid.o: guid.c guid.h
./init.o: init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 buildid.h cave.h list-square-flags.h list-terrain-flags.h cmds.h \
 cmd-core.h effects.h list-effects.h generate.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h hint.h init.h parser.h \
 list-parser-errors.h mon-init.h mon-list.h mon-lore.h z-textblock.h \
 mon-msg.h list-mon-message.h mon-util.h obj-ignore.h list-ignore-types.h \
 obj-list.h obj-make.h obj-randart.h obj-slays.h obj-tval.h list-tvals.h \
 obj-util.h player-history.h list-history-types.h player-quest.h \
 player-spell.h player-timed.h list-player-timed.h project.h \
 list-project-environs.h list-project-monsters.h randname.h store.h \
 trap.h list-trap-flags.h list-equip-slots.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h effects.h list-effects.h \
 game-world.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h parser.h list-parser-errors.h mon-lore.h \
 z-textblock.h mon-make.h mon-spell.h mon-util.h obj-gear.h \
 list-equip-slots.h obj-ignore.h list-ignore-types.h obj-make.h \
 obj-pile.h obj-randart.h obj-slays.h obj-util.h player-calcs.h \
 player-history.h list-history-types.h player-quest.h player-spell.h \
 player-timed.h list-player-timed.h player-util.h savefile.h store.h \
 cmd-core.h trap.h list-trap-flags.h
./message.o: message.c z-virt.h h-basic.h z-color.h z-util.h message.h \
 list-message.h game-event.h z-type.h option.h z-file.h list-options.h \
 init.h z-bitflag.h z-form.h z-rand.h parser.h z-dice.h z-expression.h \
 list-parser-errors.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h
./mon-attack.o: mon-attack.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h list-effects.h init.h parser.h list-parser-errors.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h monster.h \
 mon-timed.h list-mon-timed.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h list-blow-effects.h mon-desc.h mon-lore.h \
 z-textblock.h mon-spell.h mon-util.h obj-knowledge.h player-attack.h \
 cmd-core.h player-timed.h list-player-timed.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h
./mon-blow-effects.o: mon-blow-effects.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h effects.h list-effects.h init.h parser.h \
 list-parser-errors.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 mon-attack.h mon-lore.h z-textblock.h mon-util.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-make.h obj-pile.h obj-tval.h list-tvals.h \
 obj-util.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h project.h list-project-environs.h list-project-monsters.h
./mon-blow-methods.o: mon-blow-methods.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h
./mon-desc.o: mon-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 game-input.h cmd-core.h mon-desc.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h
./mon-init.o: mon-init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 effects.h list-effects.h init.h parser.h list-parser-errors.h mon-init.h \
 mon-lore.h z-textblock.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-msg.h \
 list-mon-message.h mon-power.h mon-spell.h mon-util.h obj-tval.h \
 list-tvals.h obj-util.h player-spell.h
./mon-list.o: mon-list.c game-world.h cave.h z-type.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h list-square-flags.h list-terrain-flags.h mon-desc.h \
 monster.h z-rand.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h player.h guid.h obj-properties.h \
 z-file.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h list-magic-realms.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-list.h \
 angband.h z-color.h z-util.h config.h game-event.h message.h \
 list-message.h project.h list-project-environs.h list-project-monsters.h
./mon-lore.o: mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 init.h parser.h list-parser-errors.h mon-blow-effects.h \
 mon-blow-methods.h list-blow-methods.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h list-mon-timed.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 list-blow-effects.h mon-init.h mon-lore.h z-textblock.h mon-make.h \
 mon-spell.h mon-util.h obj-gear.h list-equip-slots.h obj-tval.h \
 list-tvals.h obj-util.h player-attack.h cmd-core.h player-calcs.h \
 player-timed.h list-player-timed.h
./mon-make.o: mon-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 alloc.h game-world.h cave.h list-square-flags.h list-terrain-flags.h \
 init.h parser.h list-parser-errors.h mon-desc.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-lore.h z-textblock.h \
 mon-make.h mon-util.h obj-make.h obj-pile.h obj-tval.h list-tvals.h \
 obj-util.h player-calcs.h player-history.h list-history-types.h \
 player-quest.h target.h
./mon-move.o: mon-move.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h game-world.h init.h \
 parser.h list-parser-errors.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-attack.h mon-desc.h mon-lore.h z-textblock.h \
 mon-make.h mon-spell.h mon-util.h obj-desc.h obj-ignore.h \
 list-ignore-types.h obj-pile.h obj-slays.h obj-tval.h list-tvals.h \
 obj-util.h player-calcs.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h trap.h list-trap-flags.h
./mon-msg.o: mon-msg.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 init.h parser.h list-parser-errors.h mon-desc.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h mon-msg.h list-mon-message.h \
 mon-util.h player-calcs.h
./mon-power.o: mon-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h parser.h list-parser-errors.h mon-init.h \
 mon-power.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 mon-spell.h obj-tval.h list-tvals.h obj-util.h z-textblock.h
./mon-spell.o: mon-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h effects.h list-effects.h mon-attack.h mon-desc.h \
 monster.h cave.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-lore.h z-textblock.h \
 mon-make.h mon-spell.h mon-util.h player-timed.h list-player-timed.h \
 player-util.h project.h list-project-environs.h list-project-monsters.h
./mon-summon.o: mon-summon.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h mon-make.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 mon-summon.h list-summon-types.h mon-util.h
./mon-timed.o: mon-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h mon-desc.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-lore.h \
 z-textblock.h mon-msg.h list-mon-message.h mon-spell.h mon-util.h \
 player-calcs.h
./mon-util.o: mon-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 init.h parser.h list-parser-errors.h mon-lore.h z-textblock.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-make.h mon-msg.h \
 list-mon-message.h mon-spell.h mon-list.h mon-util.h obj-desc.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-pile.h obj-util.h \
 player-calcs.h player-timed.h list-player-timed.h player-util.h
./obj-chest.o: obj-chest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h list-effects.h mon-lore.h z-textblock.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-chest.h obj-ignore.h \
 list-ignore-types.h obj-make.h obj-pile.h obj-tval.h list-tvals.h \
 obj-util.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h
./obj-desc.o: obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-knowledge.h obj-tval.h list-tvals.h obj-util.h
./obj-gear.o: obj-gear.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cmd-core.h init.h parser.h list-parser-errors.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-ignore.h list-ignore-types.h obj-knowledge.h \
 obj-pile.h cave.h list-square-flags.h list-terrain-flags.h obj-tval.h \
 list-tvals.h obj-util.h player-calcs.h player-util.h
./obj-ignore.o: obj-ignore.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h init.h parser.h list-parser-errors.h \
 obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-knowledge.h obj-pile.h obj-tval.h list-tvals.h \
 obj-util.h player-calcs.h
./obj-info.o: obj-info.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 effects.h list-effects.h game-world.h init.h parser.h \
 list-parser-errors.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-summon.h list-summon-types.h obj-gear.h \
 list-equip-slots.h obj-info.h z-textblock.h obj-knowledge.h obj-make.h \
 obj-pile.h obj-slays.h obj-tval.h list-tvals.h obj-util.h \
 player-attack.h player-calcs.h player-timed.h list-player-timed.h \
 project.h list-project-environs.h list-project-monsters.h
./obj-knowledge.o: obj-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-slays.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-tval.h list-tvals.h \
 obj-util.h player-calcs.h player-history.h list-history-types.h store.h \
 cmd-core.h parser.h list-parser-errors.h
./obj-list.o: obj-list.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 game-world.h cave.h list-square-flags.h list-terrain-flags.h obj-desc.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-list.h obj-pile.h \
 obj-tval.h list-tvals.h obj-util.h project.h list-project-environs.h \
 list-project-monsters.h
./obj-make.o: obj-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 alloc.h cave.h list-square-flags.h list-terrain-flags.h effects.h \
 list-effects.h init.h parser.h list-parser-errors.h obj-gear.h \
 list-equip-slots.h obj-pile.h obj-make.h obj-power.h obj-slays.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h
./obj-pile.o: obj-pile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h effects.h list-effects.h \
 cmd-core.h game-input.h generate.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h grafmode.h init.h parser.h \
 list-parser-errors.h mon-make.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-info.h z-textblock.h \
 obj-knowledge.h obj-make.h obj-pile.h obj-slays.h obj-tval.h \
 list-tvals.h obj-util.h player-calcs.h player-history.h \
 list-history-types.h player-spell.h player-util.h randname.h z-queue.h
./obj-power.o: obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h obj-gear.h list-equip-slots.h obj-knowledge.h \
 obj-power.h obj-slays.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h init.h parser.h list-parser-errors.h effects.h \
 list-effects.h mon-power.h
./obj-properties.o: obj-properties.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h obj-gear.h list-equip-slots.h
./obj-randart.o: obj-randart.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h effects.h list-effects.h init.h parser.h \
 list-parser-errors.h obj-desc.h obj-make.h cave.h list-square-flags.h \
 list-terrain-flags.h obj-pile.h obj-power.h obj-randart.h obj-slays.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h randname.h
./obj-slays.o: obj-slays.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h init.h parser.h list-parser-errors.h mon-lore.h \
 z-textblock.h monster.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-desc.h obj-knowledge.h \
 obj-slays.h obj-util.h
./obj-tval.o: obj-tval.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h parser.h z-dice.h z-expression.h list-parser-errors.h \
 obj-tval.h object.h z-quark.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h \
 list-elements.h list-origins.h list-tvals.h z-type.h z-util.h
./obj-util.o: obj-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmd-core.h effects.h \
 list-effects.h game-input.h game-world.h generate.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h grafmode.h init.h parser.h \
 list-parser-errors.h mon-make.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-make.h obj-pile.h \
 obj-slays.h obj-tval.h list-tvals.h obj-util.h player-history.h \
 list-history-types.h player-spell.h player-util.h randname.h z-queue.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 init.h parser.h list-parser-errors.h
./parser.o: parser.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h parser.h z-dice.h z-expression.h list-parser-errors.h \
 game-event.h z-type.h message.h list-message.h mon-util.h monster.h \
 cave.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h player.h guid.h obj-properties.h list-stats.h \
 list-object-flags.h list-kind-flags.h list-object-modifiers.h object.h \
 z-quark.h list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h list-magic-realms.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h z-util.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 randname.h
./player-attack.o: player-attack.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h effects.h list-effects.h game-input.h init.h parser.h \
 list-parser-errors.h mon-desc.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h mon-make.h mon-msg.h \
 list-mon-message.h mon-util.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-knowledge.h obj-pile.h obj-slays.h obj-util.h player-attack.h \
 player-calcs.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h target.h
./player-birth.o: player-birth.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cmd-core.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h game-world.h init.h parser.h list-parser-errors.h \
 mon-lore.h z-textblock.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-knowledge.h obj-make.h obj-pile.h obj-power.h \
 obj-randart.h obj-tval.h list-tvals.h obj-util.h player-birth.h \
 player-calcs.h player-history.h list-history-types.h player-quest.h \
 player-spell.h player-timed.h list-player-timed.h player-util.h \
 savefile.h store.h
./player-calcs.o: player-calcs.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 game-input.h cmd-core.h game-world.h init.h parser.h \
 list-parser-errors.h mon-msg.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h list-mon-message.h mon-util.h obj-gear.h \
 list-equip-slots.h obj-ignore.h list-ignore-types.h obj-knowledge.h \
 obj-power.h obj-tval.h list-tvals.h obj-util.h player-calcs.h \
 player-spell.h player-timed.h list-player-timed.h player-util.h
./player-class.o: player-class.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-rand.h z-quark.h \
 z-dice.h z-expression.h list-elements.h list-origins.h option.h \
 list-options.h list-player-flags.h list-magic-realms.h
./player-history.o: player-history.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h obj-desc.h obj-make.h obj-pile.h obj-util.h \
 player-history.h list-history-types.h
./player-path.o: player-path.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h init.h parser.h list-parser-errors.h \
 mon-util.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 obj-ignore.h list-ignore-types.h obj-util.h player-calcs.h player-path.h \
 player-util.h
./player-quest.o: player-quest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h init.h parser.h list-parser-errors.h mon-util.h \
 monster.h cave.h list-square-flags.h list-terrain-flags.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-pile.h obj-util.h \
 player-calcs.h player-quest.h
./player-race.o: player-race.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-rand.h z-quark.h \
 z-dice.h z-expression.h list-elements.h list-origins.h option.h \
 list-options.h list-player-flags.h list-magic-realms.h
./player-spell.o: player-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h effects.h list-effects.h init.h parser.h list-parser-errors.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-tval.h \
 list-tvals.h obj-util.h player-calcs.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h
./player-timed.o: player-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-util.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 obj-knowledge.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h
./player-util.o: player-util.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h game-input.h game-world.h init.h parser.h \
 list-parser-errors.h obj-gear.h list-equip-slots.h obj-knowledge.h \
 obj-pile.h obj-tval.h list-tvals.h obj-util.h player-calcs.h \
 player-history.h list-history-types.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h score.h store.h target.h
./player.o: player.c effects.h object.h z-rand.h h-basic.h z-quark.h \
 z-bitflag.h z-form.h z-virt.h z-dice.h z-expression.h obj-properties.h \
 z-file.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h list-elements.h list-origins.h list-effects.h \
 init.h parser.h list-parser-errors.h obj-pile.h cave.h z-type.h \
 list-square-flags.h list-terrain-flags.h player.h guid.h option.h \
 list-options.h list-player-flags.h list-magic-realms.h obj-util.h \
 player-birth.h cmd-core.h player-calcs.h player-history.h angband.h \
 z-color.h z-util.h config.h game-event.h message.h list-message.h \
 list-history-types.h player-quest.h player-spell.h player-timed.h \
 list-player-timed.h
./project.o: project.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h game-input.h cmd-core.h \
 generate.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h list-parser-errors.h mon-util.h player-calcs.h player-timed.h \
 list-player-timed.h project.h list-project-environs.h \
 list-project-monsters.h
./project-feat.o: project-feat.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h generate.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-pile.h obj-util.h player-calcs.h player-timed.h \
 list-player-timed.h trap.h list-trap-flags.h list-project-environs.h \
 list-project-monsters.h
./project-mon.o: project-mon.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h list-effects.h mon-desc.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-lore.h z-textblock.h \
 mon-make.h mon-move.h mon-msg.h list-mon-message.h mon-spell.h \
 mon-util.h player-calcs.h project.h list-project-environs.h \
 list-project-monsters.h
./project-obj.o: project-obj.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-util.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-pile.h obj-tval.h list-tvals.h obj-util.h \
 player-calcs.h list-project-environs.h list-project-monsters.h
./project-player.o: project-player.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h option.h z-file.h list-options.h \
 player.h guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h list-effects.h init.h parser.h list-parser-errors.h mon-desc.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-gear.h \
 list-equip-slots.h obj-knowledge.h player-calcs.h player-timed.h \
 list-player-timed.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 buildid.h game-world.h cave.h list-square-flags.h list-terrain-flags.h \
 init.h parser.h list-parser-errors.h score.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h game-world.h init.h \
 parser.h list-parser-errors.h mon-lore.h z-textblock.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-make.h obj-pile.h obj-gear.h \
 list-equip-slots.h obj-ignore.h list-ignore-types.h savefile.h store.h \
 cmd-core.h obj-util.h player-history.h list-history-types.h \
 player-timed.h list-player-timed.h trap.h list-trap-flags.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 game-world.h cave.h list-square-flags.h list-terrain-flags.h init.h \
 parser.h list-parser-errors.h savefile.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h \
 game-world.h hint.h init.h parser.h list-parser-errors.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-ignore.h list-ignore-types.h obj-info.h \
 z-textblock.h obj-knowledge.h obj-make.h obj-pile.h obj-power.h \
 obj-slays.h obj-tval.h list-tvals.h obj-util.h player-calcs.h \
 player-history.h list-history-types.h store.h target.h debug.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmd-core.h game-input.h \
 mon-desc.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 obj-ignore.h list-ignore-types.h player-calcs.h player-timed.h \
 list-player-timed.h project.h list-project-environs.h \
 list-project-monsters.h target.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h effects.h list-effects.h \
 init.h parser.h list-parser-errors.h player-attack.h cmd-core.h \
 player-util.h trap.h list-trap-flags.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 game-input.h obj-tval.h list-tvals.h ui-birth.h ui-display.h ui-game.h \
 ui-help.h ui-input.h ui-event.h ui-term.h ui-menu.h ui-output.h \
 z-textblock.h ui-options.h obj-ignore.h list-ignore-types.h ui-player.h \
 ui-target.h
./ui-command.o: ui-command.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h cmds.h game-input.h game-world.h init.h \
 parser.h list-parser-errors.h monster.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-gear.h list-equip-slots.h obj-util.h \
 player-attack.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h store.h target.h ui-command.h ui-display.h ui-event.h \
 ui-help.h ui-input.h ui-term.h ui-keymap.h ui-map.h ui-menu.h \
 ui-output.h z-textblock.h ui-options.h obj-ignore.h list-ignore-types.h \
 ui-player.h ui-prefs.h ui-target.h wizard.h
./ui-context.o: ui-context.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h cmds.h game-input.h mon-desc.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-lore.h z-textblock.h \
 mon-util.h obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-info.h obj-tval.h list-tvals.h \
 obj-util.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h store.h parser.h list-parser-errors.h target.h \
 ui-context.h ui-input.h ui-event.h ui-term.h ui-game.h ui-keymap.h \
 ui-knowledge.h ui-menu.h ui-output.h ui-mon-lore.h ui-object.h \
 ui-player.h ui-spell.h ui-store.h ui-target.h wizard.h
./ui-death.o: ui-death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 game-input.h init.h parser.h list-parser-errors.h obj-desc.h obj-info.h \
 z-textblock.h savefile.h store.h ui-death.h ui-history.h ui-input.h \
 ui-event.h ui-term.h ui-knowledge.h ui-menu.h ui-output.h ui-object.h \
 ui-player.h ui-score.h wizard.h
./ui-display.o: ui-display.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-world.h grafmode.h hint.h init.h \
 parser.h list-parser-errors.h mon-lore.h z-textblock.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-util.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-pile.h obj-util.h player-calcs.h player-timed.h \
 list-player-timed.h player-util.h project.h list-project-environs.h \
 list-project-monsters.h savefile.h target.h ui-birth.h ui-game.h \
 ui-input.h ui-event.h ui-term.h ui-map.h ui-mon-list.h ui-mon-lore.h \
 ui-object.h ui-obj-list.h ui-output.h ui-player.h ui-prefs.h ui-store.h \
 wizard.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 ui-event.h
./ui-game.o: ui-game.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 game-world.h grafmode.h init.h parser.h list-parser-errors.h mon-lore.h \
 z-textblock.h monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-make.h \
 obj-util.h player-attack.h player-calcs.h player-path.h player-util.h \
 savefile.h ui-birth.h ui-command.h ui-context.h ui-input.h ui-event.h \
 ui-term.h ui-death.h ui-display.h ui-game.h ui-help.h ui-keymap.h \
 ui-knowledge.h ui-map.h ui-object.h ui-output.h ui-player.h ui-prefs.h \
 ui-spell.h ui-score.h ui-signals.h ui-store.h ui-target.h
./ui-help.o: ui-help.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 buildid.h init.h parser.h list-parser-errors.h ui-input.h cmd-core.h \
 ui-event.h ui-term.h ui-output.h z-textblock.h
./ui-history.o: ui-history.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h player-history.h list-history-types.h ui-input.h \
 cmd-core.h ui-event.h ui-term.h ui-output.h z-textblock.h
./ui-init.o: ui-init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 game-input.h cmd-core.h ui-display.h ui-game.h ui-input.h ui-event.h \
 ui-term.h ui-keymap.h ui-knowledge.h ui-options.h obj-ignore.h \
 list-ignore-types.h ui-output.h z-textblock.h ui-prefs.h cave.h \
 list-square-flags.h list-terrain-flags.h
./ui-input.o: ui-input.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cmds.h cave.h list-square-flags.h list-terrain-flags.h cmd-core.h \
 game-input.h game-world.h init.h parser.h list-parser-errors.h \
 obj-gear.h list-equip-slots.h obj-util.h player-calcs.h player-path.h \
 randname.h savefile.h target.h ui-command.h ui-context.h ui-input.h \
 ui-event.h ui-term.h ui-display.h ui-help.h ui-keymap.h ui-knowledge.h \
 ui-map.h ui-object.h ui-output.h z-textblock.h ui-player.h ui-prefs.h \
 ui-signals.h ui-spell.h ui-store.h ui-target.h
./ui-keymap.o: ui-keymap.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h ui-keymap.h ui-term.h ui-event.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h game-input.h grafmode.h init.h parser.h \
 list-parser-errors.h mon-lore.h z-textblock.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-util.h obj-desc.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-info.h obj-make.h \
 obj-pile.h obj-tval.h list-tvals.h obj-util.h player-calcs.h \
 player-history.h list-history-types.h store.h target.h trap.h \
 list-trap-flags.h ui-context.h ui-input.h ui-event.h ui-term.h \
 ui-history.h ui-menu.h ui-output.h ui-mon-list.h ui-mon-lore.h \
 ui-object.h ui-obj-list.h ui-options.h ui-prefs.h ui-score.h ui-store.h \
 ui-target.h wizard.h
./ui-map.o: ui-map.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h grafmode.h init.h \
 parser.h list-parser-errors.h mon-util.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-tval.h list-tvals.h \
 obj-util.h player-timed.h list-player-timed.h trap.h list-trap-flags.h \
 ui-input.h cmd-core.h ui-event.h ui-term.h ui-map.h ui-object.h \
 ui-output.h z-textblock.h ui-prefs.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h ui-target.h ui-event.h \
 ui-input.h cmd-core.h ui-term.h ui-menu.h ui-output.h z-textblock.h
./ui-mon-list.o: ui-mon-list.c mon-desc.h monster.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-rand.h cave.h z-type.h list-square-flags.h \
 list-terrain-flags.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h player.h guid.h obj-properties.h \
 z-file.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h list-magic-realms.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-list.h \
 angband.h z-color.h z-util.h config.h game-event.h message.h \
 list-message.h mon-lore.h z-textblock.h mon-util.h player-timed.h \
 list-player-timed.h ui-output.h ui-event.h ui-term.h ui-prefs.h
./ui-mon-lore.o: ui-mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h mon-lore.h z-textblock.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h mon-timed.h list-mon-timed.h \
 mon-blow-methods.h list-blow-methods.h mon-blow-effects.h \
 list-blow-effects.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h ui-mon-lore.h ui-output.h ui-event.h ui-term.h \
 ui-prefs.h
./ui-obj-list.o: ui-obj-list.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h init.h parser.h list-parser-errors.h obj-list.h \
 obj-util.h ui-object.h cmd-core.h ui-obj-list.h ui-output.h ui-event.h \
 ui-term.h z-textblock.h
./ui-object.o: ui-object.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h cmds.h effects.h list-effects.h game-input.h init.h parser.h \
 list-parser-errors.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-info.h z-textblock.h \
 obj-knowledge.h obj-make.h obj-pile.h obj-tval.h list-tvals.h obj-util.h \
 player-attack.h player-calcs.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h store.h ui-command.h ui-display.h \
 ui-game.h ui-input.h ui-event.h ui-term.h ui-keymap.h ui-menu.h \
 ui-output.h ui-object.h ui-options.h ui-prefs.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-input.h init.h parser.h \
 list-parser-errors.h obj-desc.h obj-ignore.h list-ignore-types.h \
 obj-tval.h list-tvals.h obj-util.h player-calcs.h ui-display.h ui-help.h \
 ui-input.h ui-event.h ui-term.h ui-keymap.h ui-knowledge.h ui-menu.h \
 ui-output.h z-textblock.h ui-options.h ui-prefs.h ui-target.h
./ui-output.o: ui-output.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 player-calcs.h ui-input.h cmd-core.h ui-event.h ui-term.h ui-output.h \
 z-textblock.h
./ui-player.o: ui-player.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h buildid.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h parser.h list-parser-errors.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-info.h z-textblock.h obj-knowledge.h \
 obj-util.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h store.h cmd-core.h ui-display.h ui-history.h ui-input.h \
 ui-event.h ui-term.h ui-menu.h ui-output.h ui-object.h ui-player.h
./ui-prefs.o: ui-prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h game-input.h cmd-core.h \
 grafmode.h init.h parser.h list-parser-errors.h mon-util.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-ignore.h list-ignore-types.h \
 obj-tval.h list-tvals.h obj-util.h project.h list-project-environs.h \
 list-project-monsters.h trap.h list-trap-flags.h ui-display.h \
 ui-keymap.h ui-prefs.h ui-term.h ui-event.h
./ui-score.o: ui-score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 buildid.h game-world.h cave.h list-square-flags.h list-terrain-flags.h \
 score.h ui-input.h cmd-core.h ui-event.h ui-term.h ui-output.h \
 z-textblock.h ui-score.h
./ui-signals.o: ui-signals.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h savefile.h ui-game.h cmd-core.h ui-signals.h \
 ui-term.h ui-event.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h \
 game-input.h obj-tval.h list-tvals.h obj-util.h player-calcs.h \
 player-spell.h ui-menu.h ui-output.h ui-event.h ui-term.h z-textblock.h
./ui-store.o: ui-store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h option.h z-file.h list-options.h player.h guid.h \
 obj-properties.h list-stats.h list-object-flags.h list-kind-flags.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h list-player-flags.h list-magic-realms.h \
 cave.h list-square-flags.h list-terrain-flags.h cmds.h cmd-core.h \
 game-input.h hint.h init.h parser.h list-parser-errors.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-ignore.h list-ignore-types.h obj-info.h \
 z-textblock.h obj-knowledge.h obj-make.h obj-pile.h obj-tval.h \
 list-tvals.h obj-util.h player-calcs.h player-history.h \
 list-history-types.h store.h target.h ui-display.h ui-input.h ui-event.h \
 ui-term.h ui-menu.h ui-output.h ui-object.h ui-options.h ui-knowledge.h \
 ui-player.h ui-spell.h ui-command.h ui-store.h z-debug.h
./ui-target.o: ui-target.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 game-input.h cmd-core.h init.h parser.h list-parser-errors.h mon-desc.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-lore.h \
 z-textblock.h obj-desc.h obj-pile.h obj-util.h player-attack.h \
 player-calcs.h player-timed.h list-player-timed.h project.h \
 list-project-environs.h list-project-monsters.h target.h trap.h \
 list-trap-flags.h ui-input.h ui-event.h ui-term.h ui-keymap.h ui-map.h \
 ui-mon-lore.h ui-object.h ui-output.h ui-target.h
./ui-term.o: ui-term.c buildid.h h-basic.h ui-term.h ui-event.h z-color.h \
 z-util.h z-virt.h
./wiz-debug.o: wiz-debug.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h effects.h list-effects.h game-input.h grafmode.h \
 init.h parser.h list-parser-errors.h mon-lore.h z-textblock.h monster.h \
 mon-timed.h list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-make.h mon-util.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-make.h obj-pile.h obj-power.h \
 obj-slays.h obj-tval.h list-tvals.h obj-util.h player-calcs.h \
 player-timed.h list-player-timed.h player-util.h project.h \
 list-project-environs.h list-project-monsters.h target.h ui-command.h \
 ui-event.h ui-display.h ui-help.h ui-input.h ui-term.h ui-map.h \
 ui-menu.h ui-output.h ui-prefs.h ui-target.h wizard.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h buildid.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-world.h init.h parser.h \
 list-parser-errors.h mon-lore.h z-textblock.h monster.h mon-timed.h \
 list-mon-timed.h mon-blow-methods.h list-blow-methods.h \
 mon-blow-effects.h list-blow-effects.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-desc.h obj-info.h obj-make.h \
 obj-pile.h obj-power.h obj-tval.h list-tvals.h obj-util.h ui-input.h \
 ui-event.h ui-term.h ui-knowledge.h ui-menu.h ui-output.h ui-mon-lore.h \
 wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h option.h z-file.h list-options.h player.h \
 guid.h obj-properties.h list-stats.h list-object-flags.h \
 list-kind-flags.h list-object-modifiers.h object.h z-quark.h z-dice.h \
 z-expression.h list-elements.h list-origins.h list-player-flags.h \
 list-magic-realms.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h effects.h list-effects.h game-input.h generate.h \
 monster.h mon-timed.h list-mon-timed.h mon-blow-methods.h \
 list-blow-methods.h mon-blow-effects.h list-blow-effects.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 parser.h list-parser-errors.h mon-make.h obj-pile.h obj-randart.h \
 obj-tval.h list-tvals.h obj-util.h ui-command.h wizard.h
./buildid.o: buildid.c buildid.h
./z-bitflag.o: z-bitflag.c z-bitflag.h h-basic.h z-form.h z-virt.h
./z-color.o: z-color.c h-basic.h z-color.h z-util.h
./z-dice.o: z-dice.c z-dice.h h-basic.h z-rand.h z-expression.h z-virt.h \
 z-util.h
./z-expression.o: z-expression.c z-expression.h h-basic.h z-virt.h z-util.h
./z-file.o: z-file.c h-basic.h z-file.h z-form.h z-util.h z-virt.h
./z-form.o: z-form.c z-form.h h-basic.h z-type.h z-util.h z-virt.h
./z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h init.h z-bitflag.h \
 z-form.h z-file.h z-rand.h parser.h z-dice.h z-expression.h \
 list-parser-errors.h
./z-queue.o: z-queue.c z-queue.h h-basic.h
./z-rand.o: z-rand.c z-rand.h h-basic.h
./z-set.o: z-set.c z-set.h h-basic.h z-rand.h z-virt.h
./z-textblock.o: z-textblock.c z-color.h h-basic.h z-textblock.h z-file.h \
 z-util.h z-virt.h z-form.h
./z-type.o: z-type.c z-type.h h-basic.h z-virt.h
./z-util.o: z-util.c z-util.h h-basic.h
./z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
