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
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h game-world.h init.h datafile.h parser.h \
 list-parser-errors.h mon-group.h monster.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-ignore.h list-ignore-types.h \
 obj-pile.h obj-tval.h obj-util.h player-timed.h list-player-timed.h \
 trap.h list-trap-flags.h
./cave-map.o: cave-map.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 init.h datafile.h parser.h list-parser-errors.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 mon-msg.h list-mon-message.h obj-ignore.h list-ignore-types.h obj-pile.h \
 obj-tval.h obj-util.h player-calcs.h player-timed.h list-player-timed.h \
 trap.h list-trap-flags.h
./cave-square.o: cave-square.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h init.h datafile.h parser.h list-parser-errors.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 obj-knowledge.h obj-pile.h obj-util.h player-timed.h list-player-timed.h \
 trap.h list-trap-flags.h
./cave-view.o: cave-view.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h init.h datafile.h parser.h list-parser-errors.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h player-calcs.h player-timed.h list-player-timed.h \
 trap.h list-trap-flags.h
./cmd-cave.o: cmd-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h cmds.h game-input.h generate.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 datafile.h parser.h list-parser-errors.h mon-attack.h mon-desc.h \
 mon-lore.h z-textblock.h mon-spell.h mon-util.h mon-msg.h \
 list-mon-message.h obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-pile.h obj-util.h \
 player-attack.h player-calcs.h player-path.h player-timed.h \
 list-player-timed.h player-util.h project.h source.h list-projections.h \
 store.h trap.h list-trap-flags.h
./cmd-core.o: cmd-core.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-input.h obj-chest.h obj-desc.h \
 obj-tval.h obj-util.h player-attack.h player-birth.h player-calcs.h \
 player-spell.h player-timed.h list-player-timed.h player-util.h store.h \
 datafile.h parser.h list-parser-errors.h target.h mon-predicate.h
./cmd-misc.o: cmd-misc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h cmds.h game-input.h init.h datafile.h \
 parser.h list-parser-errors.h mon-lore.h z-textblock.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 mon-msg.h list-mon-message.h player-calcs.h player-history.h \
 list-history-types.h obj-util.h
./cmd-obj.o: cmd-obj.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h cmds.h effects.h source.h player-attack.h list-effects.h \
 game-input.h init.h datafile.h parser.h list-parser-errors.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-ignore.h list-ignore-types.h \
 obj-info.h z-textblock.h obj-knowledge.h obj-make.h obj-pile.h \
 obj-tval.h obj-util.h player-calcs.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h target.h mon-predicate.h trap.h \
 list-trap-flags.h
./cmd-pickup.o: cmd-pickup.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h game-input.h generate.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 datafile.h parser.h list-parser-errors.h mon-lore.h z-textblock.h \
 mon-util.h mon-msg.h list-mon-message.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-ignore.h list-ignore-types.h obj-pile.h \
 obj-tval.h obj-util.h player-attack.h player-calcs.h player-history.h \
 list-history-types.h player-util.h trap.h list-trap-flags.h
./datafile.o: datafile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h datafile.h parser.h list-parser-errors.h \
 game-world.h cave.h list-square-flags.h list-terrain-flags.h init.h
./debug.o: debug.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h debug.h
./effects.o: effects.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h list-effects.h \
 game-input.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h datafile.h parser.h list-parser-errors.h \
 mon-desc.h mon-lore.h z-textblock.h mon-make.h mon-msg.h \
 list-mon-message.h mon-spell.h mon-summon.h mon-util.h obj-chest.h \
 obj-curse.h obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-knowledge.h obj-make.h obj-pile.h obj-power.h \
 obj-tval.h obj-util.h player-calcs.h player-history.h \
 list-history-types.h player-spell.h player-timed.h list-player-timed.h \
 player-util.h project.h list-projections.h trap.h list-trap-flags.h
./effects-info.o: effects-info.c effects-info.h z-textblock.h z-file.h \
 h-basic.h effects.h source.h object.h z-type.h z-quark.h z-bitflag.h \
 z-form.h z-virt.h z-dice.h z-rand.h z-expression.h obj-properties.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h list-elements.h list-origins.h player-attack.h \
 cmd-core.h cmds.h cave.h list-square-flags.h list-terrain-flags.h \
 list-effects.h init.h datafile.h parser.h list-parser-errors.h message.h \
 list-message.h mon-summon.h monster.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h player.h guid.h option.h \
 list-options.h list-player-flags.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-info.h player-timed.h \
 list-player-timed.h project.h list-projections.h z-color.h z-util.h
./game-event.o: game-event.c game-event.h z-type.h h-basic.h object.h \
 z-quark.h z-bitflag.h z-form.h z-virt.h z-dice.h z-rand.h z-expression.h \
 obj-properties.h z-file.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h list-elements.h \
 list-origins.h
./game-input.o: game-input.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmd-core.h game-input.h
./game-world.o: game-world.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h effects.h source.h player-attack.h \
 list-effects.h game-world.h generate.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 datafile.h parser.h list-parser-errors.h mon-make.h mon-move.h \
 mon-util.h mon-msg.h list-mon-message.h obj-curse.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-knowledge.h obj-tval.h obj-util.h \
 player-calcs.h player-timed.h list-player-timed.h player-util.h trap.h \
 list-trap-flags.h z-queue.h
./generate.o: generate.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 datafile.h parser.h list-parser-errors.h game-input.h cmd-core.h \
 game-world.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h mon-make.h mon-move.h mon-spell.h obj-tval.h \
 obj-util.h player-history.h list-history-types.h player-util.h trap.h \
 list-trap-flags.h z-queue.h list-dun-profiles.h list-rooms.h
./gen-cave.o: gen-cave.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 datafile.h parser.h list-parser-errors.h game-world.h generate.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h mon-group.h mon-make.h mon-spell.h mon-util.h \
 mon-msg.h list-mon-message.h player-util.h store.h cmd-core.h trap.h \
 list-trap-flags.h z-queue.h
./gen-chunk.o: gen-chunk.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h datafile.h parser.h list-parser-errors.h \
 mon-group.h mon-make.h obj-util.h trap.h list-trap-flags.h
./gen-monster.o: gen-monster.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h datafile.h parser.h list-parser-errors.h \
 mon-make.h mon-spell.h
./gen-room.o: gen-room.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 datafile.h parser.h list-parser-errors.h generate.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 mon-group.h mon-make.h mon-spell.h obj-tval.h trap.h list-trap-flags.h \
 z-queue.h
./gen-util.o: gen-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 datafile.h parser.h list-parser-errors.h generate.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 mon-make.h mon-spell.h obj-make.h obj-pile.h obj-tval.h obj-util.h \
 player-util.h trap.h list-trap-flags.h z-queue.h
./grafmode.o: grafmode.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h datafile.h parser.h list-parser-errors.h grafmode.h \
 init.h
./guid.o: guid.c guid.h
./init.o: init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmds.h cmd-core.h datafile.h parser.h \
 list-parser-errors.h effects.h source.h player-attack.h list-effects.h \
 game-world.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h hint.h init.h mon-init.h mon-list.h mon-lore.h \
 z-textblock.h mon-msg.h list-mon-message.h mon-summon.h mon-util.h \
 obj-chest.h obj-ignore.h list-ignore-types.h obj-init.h obj-list.h \
 obj-make.h obj-pile.h obj-power.h obj-randart.h \
 list-randart-properties.h obj-slays.h obj-tval.h obj-util.h \
 player-history.h list-history-types.h player-quest.h player-spell.h \
 player-timed.h list-player-timed.h project.h list-projections.h \
 randname.h store.h trap.h list-trap-flags.h ui-entry.h ui-entry-init.h \
 ui-visuals.h list-equip-slots.h
./load.o: load.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h list-effects.h \
 game-world.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h datafile.h parser.h list-parser-errors.h \
 mon-group.h mon-lore.h z-textblock.h mon-make.h mon-spell.h mon-util.h \
 mon-msg.h list-mon-message.h obj-curse.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-init.h obj-knowledge.h obj-make.h \
 obj-pile.h obj-randart.h list-randart-properties.h obj-slays.h \
 obj-tval.h obj-util.h player-calcs.h player-history.h \
 list-history-types.h player-quest.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h savefile.h store.h trap.h \
 list-trap-flags.h ui-term.h ui-event.h
./message.o: message.c z-virt.h h-basic.h z-color.h z-util.h message.h \
 list-message.h game-event.h z-type.h option.h z-file.h list-options.h \
 init.h z-bitflag.h z-form.h z-rand.h datafile.h object.h z-quark.h \
 z-dice.h z-expression.h obj-properties.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h list-elements.h list-origins.h parser.h \
 list-parser-errors.h player.h guid.h list-player-flags.h
./mon-attack.o: mon-attack.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h list-effects.h \
 init.h datafile.h parser.h list-parser-errors.h mon-attack.h mon-blows.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-desc.h \
 mon-lore.h z-textblock.h mon-spell.h mon-util.h mon-msg.h \
 list-mon-message.h obj-knowledge.h player-timed.h list-player-timed.h \
 player-util.h project.h list-projections.h
./mon-blows.o: mon-blows.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h effects.h source.h player-attack.h cmd-core.h cmds.h \
 cave.h list-square-flags.h list-terrain-flags.h list-effects.h init.h \
 datafile.h parser.h list-parser-errors.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 mon-attack.h mon-desc.h mon-lore.h z-textblock.h mon-make.h mon-msg.h \
 list-mon-message.h mon-util.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-make.h obj-pile.h obj-slays.h obj-tval.h obj-util.h player-calcs.h \
 player-timed.h list-player-timed.h player-util.h project.h \
 list-projections.h
./mon-desc.o: mon-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h game-input.h cmd-core.h mon-desc.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h
./mon-group.o: mon-group.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 init.h datafile.h parser.h list-parser-errors.h mon-group.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-make.h \
 mon-util.h mon-msg.h list-mon-message.h
./mon-init.o: mon-init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h datafile.h parser.h list-parser-errors.h effects.h \
 source.h player-attack.h cmd-core.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h list-effects.h generate.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 mon-init.h mon-lore.h z-textblock.h mon-msg.h list-mon-message.h \
 mon-spell.h mon-util.h obj-tval.h obj-util.h player-spell.h project.h \
 list-projections.h ui-visuals.h
./mon-list.o: mon-list.c game-world.h cave.h z-type.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h list-square-flags.h list-terrain-flags.h mon-desc.h \
 monster.h z-rand.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 mon-blows.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-list.h angband.h z-color.h z-util.h config.h \
 game-event.h message.h list-message.h project.h source.h \
 list-projections.h
./mon-lore.o: mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h effects.h source.h player-attack.h cmd-core.h cmds.h \
 cave.h list-square-flags.h list-terrain-flags.h list-effects.h \
 game-world.h init.h datafile.h parser.h list-parser-errors.h mon-blows.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-init.h \
 mon-lore.h z-textblock.h mon-make.h mon-spell.h mon-util.h mon-msg.h \
 list-mon-message.h obj-gear.h list-equip-slots.h obj-tval.h obj-util.h \
 player-calcs.h player-timed.h list-player-timed.h project.h \
 list-projections.h
./mon-make.o: mon-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h alloc.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h datafile.h parser.h list-parser-errors.h \
 mon-group.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h mon-make.h mon-util.h \
 mon-msg.h list-mon-message.h obj-knowledge.h obj-make.h obj-pile.h \
 obj-tval.h obj-util.h player-calcs.h player-timed.h list-player-timed.h
./mon-move.o: mon-move.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h init.h datafile.h parser.h list-parser-errors.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 mon-attack.h mon-desc.h mon-group.h mon-lore.h z-textblock.h mon-make.h \
 mon-spell.h mon-util.h mon-msg.h list-mon-message.h obj-desc.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-pile.h obj-slays.h \
 obj-tval.h obj-util.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h project.h source.h list-projections.h trap.h \
 list-trap-flags.h
./mon-msg.o: mon-msg.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h mon-desc.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-msg.h list-mon-message.h mon-util.h game-input.h \
 cmd-core.h player-calcs.h
./mon-predicate.o: mon-predicate.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-group.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-spell.h mon-util.h mon-msg.h list-mon-message.h
./mon-spell.o: mon-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h effects.h source.h player-attack.h cmd-core.h cmds.h \
 cave.h list-square-flags.h list-terrain-flags.h list-effects.h init.h \
 datafile.h parser.h list-parser-errors.h mon-attack.h mon-desc.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h mon-make.h mon-spell.h \
 mon-util.h mon-msg.h list-mon-message.h obj-knowledge.h player-timed.h \
 list-player-timed.h player-util.h project.h list-projections.h
./mon-summon.o: mon-summon.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 datafile.h parser.h list-parser-errors.h mon-group.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-make.h \
 mon-summon.h mon-util.h mon-msg.h list-mon-message.h
./mon-timed.o: mon-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h mon-desc.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h mon-msg.h list-mon-message.h \
 mon-spell.h mon-util.h player-calcs.h
./mon-util.o: mon-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmd-core.h effects.h source.h player-attack.h cmds.h \
 cave.h list-square-flags.h list-terrain-flags.h list-effects.h \
 game-world.h init.h datafile.h parser.h list-parser-errors.h mon-desc.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-list.h mon-lore.h z-textblock.h mon-make.h \
 mon-msg.h list-mon-message.h mon-spell.h mon-summon.h mon-util.h \
 obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-knowledge.h obj-pile.h obj-slays.h obj-tval.h \
 obj-util.h player-calcs.h player-history.h list-history-types.h \
 player-quest.h player-timed.h list-player-timed.h player-util.h \
 project.h list-projections.h trap.h list-trap-flags.h z-set.h
./obj-chest.o: obj-chest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h list-effects.h \
 init.h datafile.h parser.h list-parser-errors.h mon-lore.h z-textblock.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-chest.h obj-ignore.h list-ignore-types.h \
 obj-knowledge.h obj-make.h obj-pile.h obj-tval.h obj-util.h \
 player-calcs.h player-spell.h player-timed.h list-player-timed.h \
 player-util.h
./obj-curse.o: obj-curse.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h effects.h source.h player-attack.h cmd-core.h cmds.h \
 cave.h list-square-flags.h list-terrain-flags.h list-effects.h init.h \
 datafile.h parser.h list-parser-errors.h obj-curse.h obj-gear.h \
 list-equip-slots.h obj-knowledge.h obj-pile.h obj-util.h player-timed.h \
 list-player-timed.h player-util.h
./obj-desc.o: obj-desc.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h obj-chest.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-tval.h obj-util.h
./obj-gear.o: obj-gear.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmd-core.h init.h datafile.h parser.h \
 list-parser-errors.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-pile.h cave.h \
 list-square-flags.h list-terrain-flags.h obj-tval.h obj-util.h \
 player-calcs.h player-util.h
./obj-ignore.o: obj-ignore.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h init.h datafile.h parser.h \
 list-parser-errors.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-ignore.h list-ignore-types.h obj-knowledge.h obj-pile.h obj-tval.h \
 obj-util.h player-calcs.h
./obj-info.o: obj-info.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h effects.h source.h player-attack.h \
 list-effects.h effects-info.h z-textblock.h game-world.h init.h \
 datafile.h parser.h list-parser-errors.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 mon-msg.h list-mon-message.h obj-curse.h obj-gear.h list-equip-slots.h \
 obj-info.h obj-knowledge.h obj-make.h obj-pile.h obj-slays.h obj-tval.h \
 obj-util.h player-calcs.h project.h list-projections.h
./obj-init.o: obj-init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h datafile.h parser.h list-parser-errors.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h list-effects.h init.h \
 mon-util.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-msg.h list-mon-message.h obj-curse.h obj-ignore.h \
 list-ignore-types.h obj-list.h obj-make.h obj-pile.h obj-power.h \
 obj-randart.h list-randart-properties.h obj-slays.h obj-tval.h \
 obj-util.h player-spell.h project.h list-projections.h ui-entry.h
./obj-knowledge.o: obj-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 init.h datafile.h parser.h list-parser-errors.h obj-curse.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-ignore.h list-ignore-types.h \
 obj-knowledge.h obj-pile.h obj-slays.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-tval.h \
 obj-util.h player-calcs.h player-history.h list-history-types.h \
 player-util.h project.h source.h list-projections.h store.h cmd-core.h
./obj-list.o: obj-list.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h obj-desc.h obj-ignore.h list-ignore-types.h \
 obj-knowledge.h obj-list.h obj-pile.h obj-tval.h obj-util.h project.h \
 source.h list-projections.h
./obj-make.o: obj-make.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h alloc.h cave.h list-square-flags.h \
 list-terrain-flags.h effects.h source.h player-attack.h cmd-core.h \
 cmds.h list-effects.h init.h datafile.h parser.h list-parser-errors.h \
 obj-chest.h obj-curse.h obj-gear.h list-equip-slots.h obj-knowledge.h \
 obj-make.h obj-pile.h obj-power.h obj-slays.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-tval.h \
 obj-util.h
./obj-pile.o: obj-pile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h list-effects.h \
 game-input.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h grafmode.h init.h datafile.h parser.h \
 list-parser-errors.h mon-make.h mon-util.h mon-msg.h list-mon-message.h \
 obj-curse.h obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-info.h z-textblock.h obj-knowledge.h obj-make.h \
 obj-pile.h obj-slays.h obj-tval.h obj-util.h player-calcs.h \
 player-history.h list-history-types.h player-spell.h player-util.h \
 randname.h trap.h list-trap-flags.h z-queue.h
./obj-power.o: obj-power.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h obj-curse.h obj-gear.h list-equip-slots.h \
 obj-knowledge.h obj-power.h obj-slays.h monster.h cave.h \
 list-square-flags.h list-terrain-flags.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-tval.h obj-util.h init.h \
 datafile.h parser.h list-parser-errors.h effects.h source.h \
 player-attack.h cmd-core.h cmds.h list-effects.h
./obj-properties.o: obj-properties.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h player.h guid.h obj-properties.h \
 z-file.h list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h init.h datafile.h parser.h list-parser-errors.h
./obj-randart.o: obj-randart.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h datafile.h parser.h list-parser-errors.h effects.h \
 source.h player-attack.h cmd-core.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h list-effects.h init.h obj-curse.h obj-desc.h \
 obj-make.h obj-pile.h obj-power.h obj-randart.h \
 list-randart-properties.h obj-slays.h monster.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h obj-tval.h obj-util.h project.h \
 list-projections.h randname.h z-textblock.h
./obj-slays.o: obj-slays.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h init.h datafile.h parser.h list-parser-errors.h \
 mon-lore.h z-textblock.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-desc.h obj-init.h obj-knowledge.h obj-slays.h \
 obj-util.h player-timed.h list-player-timed.h
./obj-tval.o: obj-tval.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h datafile.h object.h z-type.h z-quark.h z-dice.h \
 z-expression.h obj-properties.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h list-elements.h \
 list-origins.h parser.h list-parser-errors.h obj-tval.h z-util.h
./obj-util.o: obj-util.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h effects.h source.h player-attack.h cmds.h list-effects.h \
 game-input.h game-world.h generate.h monster.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h grafmode.h init.h datafile.h \
 parser.h list-parser-errors.h mon-make.h obj-curse.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-ignore.h list-ignore-types.h \
 obj-knowledge.h obj-make.h obj-pile.h obj-slays.h obj-tval.h obj-util.h \
 player-history.h list-history-types.h player-spell.h player-util.h \
 randname.h z-queue.h
./option.o: option.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h init.h datafile.h parser.h list-parser-errors.h
./parser.o: parser.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h datafile.h object.h z-type.h z-quark.h z-dice.h \
 z-expression.h obj-properties.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h list-elements.h \
 list-origins.h parser.h list-parser-errors.h game-event.h message.h \
 list-message.h mon-util.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h player.h guid.h option.h list-options.h \
 list-player-flags.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-msg.h list-mon-message.h z-util.h
./randname.o: randname.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h randname.h
./player-attack.o: player-attack.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h effects.h source.h player-attack.h list-effects.h \
 game-input.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h datafile.h parser.h list-parser-errors.h \
 mon-desc.h mon-lore.h z-textblock.h mon-make.h mon-msg.h \
 list-mon-message.h mon-util.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-knowledge.h obj-pile.h obj-slays.h obj-tval.h obj-util.h \
 player-calcs.h player-timed.h list-player-timed.h player-util.h \
 project.h list-projections.h
./player-birth.o: player-birth.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmd-core.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h game-world.h init.h datafile.h parser.h \
 list-parser-errors.h mon-lore.h z-textblock.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 obj-curse.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-init.h obj-knowledge.h obj-make.h obj-pile.h \
 obj-power.h obj-randart.h list-randart-properties.h obj-tval.h \
 obj-util.h player-birth.h player-calcs.h player-history.h \
 list-history-types.h player-quest.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h savefile.h store.h
./player-calcs.o: player-calcs.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-input.h cmd-core.h game-world.h init.h datafile.h parser.h \
 list-parser-errors.h mon-msg.h monster.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h list-mon-message.h mon-util.h \
 obj-curse.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-knowledge.h obj-power.h obj-tval.h obj-util.h \
 player-calcs.h player-spell.h player-timed.h list-player-timed.h \
 player-util.h
./player-class.o: player-class.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h object.h z-type.h \
 z-quark.h z-dice.h z-rand.h z-expression.h list-elements.h \
 list-origins.h option.h list-options.h list-player-flags.h
./player-history.o: player-history.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h player.h guid.h obj-properties.h \
 z-file.h list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h obj-desc.h obj-make.h obj-pile.h obj-util.h \
 player-history.h list-history-types.h
./player-path.o: player-path.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h init.h datafile.h parser.h list-parser-errors.h \
 mon-predicate.h obj-ignore.h list-ignore-types.h obj-util.h \
 player-calcs.h player-path.h player-timed.h list-player-timed.h \
 player-util.h
./player-properties.o: player-properties.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h player.h guid.h obj-properties.h \
 z-file.h list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h ui-input.h cmd-core.h ui-event.h ui-term.h ui-menu.h \
 ui-output.h z-textblock.h
./player-quest.o: player-quest.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h datafile.h parser.h list-parser-errors.h init.h \
 mon-util.h monster.h cave.h list-square-flags.h list-terrain-flags.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-msg.h \
 list-mon-message.h obj-pile.h obj-util.h player-calcs.h player-quest.h
./player-race.o: player-race.c player.h guid.h obj-properties.h z-file.h \
 h-basic.h z-bitflag.h z-form.h z-virt.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h object.h z-type.h \
 z-quark.h z-dice.h z-rand.h z-expression.h list-elements.h \
 list-origins.h option.h list-options.h list-player-flags.h
./player-spell.o: player-spell.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h effects.h source.h player-attack.h cmds.h list-effects.h \
 init.h datafile.h parser.h list-parser-errors.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-gear.h \
 list-equip-slots.h obj-tval.h obj-util.h player-calcs.h player-spell.h \
 player-timed.h list-player-timed.h player-util.h project.h \
 list-projections.h
./player-timed.o: player-timed.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 datafile.h parser.h list-parser-errors.h init.h mon-util.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-msg.h \
 list-mon-message.h obj-gear.h list-equip-slots.h obj-knowledge.h \
 obj-util.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h
./player-util.o: player-util.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h cmds.h game-input.h game-world.h generate.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h init.h \
 datafile.h parser.h list-parser-errors.h obj-chest.h obj-gear.h \
 list-equip-slots.h obj-knowledge.h obj-pile.h obj-tval.h obj-util.h \
 player-calcs.h player-history.h list-history-types.h player-spell.h \
 player-timed.h list-player-timed.h player-util.h project.h source.h \
 list-projections.h score.h store.h trap.h list-trap-flags.h
./player.o: player.c effects.h source.h object.h z-type.h h-basic.h \
 z-quark.h z-bitflag.h z-form.h z-virt.h z-dice.h z-rand.h z-expression.h \
 obj-properties.h z-file.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h list-elements.h \
 list-origins.h player-attack.h cmd-core.h cmds.h cave.h \
 list-square-flags.h list-terrain-flags.h list-effects.h init.h \
 datafile.h parser.h list-parser-errors.h obj-pile.h player.h guid.h \
 option.h list-options.h list-player-flags.h obj-util.h player-birth.h \
 player-calcs.h player-history.h angband.h z-color.h z-util.h config.h \
 game-event.h message.h list-message.h list-history-types.h \
 player-quest.h player-spell.h player-timed.h list-player-timed.h
./project.o: project.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-input.h cmd-core.h generate.h monster.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h init.h datafile.h parser.h \
 list-parser-errors.h mon-util.h mon-msg.h list-mon-message.h \
 player-calcs.h player-timed.h list-player-timed.h project.h source.h \
 list-projections.h trap.h list-trap-flags.h
./project-feat.o: project-feat.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-pile.h obj-util.h player-calcs.h player-timed.h \
 list-player-timed.h source.h trap.h list-trap-flags.h list-projections.h
./project-mon.o: project-mon.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h list-effects.h \
 generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-desc.h mon-lore.h z-textblock.h mon-make.h \
 mon-move.h mon-msg.h list-mon-message.h mon-spell.h mon-util.h \
 player-calcs.h project.h list-projections.h
./project-obj.o: project-obj.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 mon-util.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-msg.h list-mon-message.h obj-chest.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-ignore.h list-ignore-types.h \
 obj-pile.h obj-tval.h obj-util.h player-calcs.h source.h \
 list-projections.h
./project-player.o: project-player.c angband.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h \
 z-type.h message.h list-message.h player.h guid.h obj-properties.h \
 z-file.h list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h list-effects.h \
 init.h datafile.h parser.h list-parser-errors.h mon-desc.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 mon-msg.h list-mon-message.h obj-desc.h obj-gear.h list-equip-slots.h \
 obj-knowledge.h player-calcs.h player-timed.h list-player-timed.h \
 player-util.h project.h list-projections.h trap.h list-trap-flags.h
./score.o: score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h datafile.h parser.h list-parser-errors.h \
 score.h
./save.o: save.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-world.h init.h datafile.h parser.h list-parser-errors.h mon-group.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h mon-make.h obj-desc.h \
 obj-knowledge.h obj-pile.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-tval.h obj-util.h savefile.h store.h cmd-core.h \
 player-history.h list-history-types.h player-timed.h list-player-timed.h \
 trap.h list-trap-flags.h ui-term.h ui-event.h
./savefile.o: savefile.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h datafile.h parser.h list-parser-errors.h \
 savefile.h save-charoutput.h
./save-charoutput.o: save-charoutput.c init.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-file.h z-rand.h datafile.h object.h z-type.h \
 z-quark.h z-dice.h z-expression.h obj-properties.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h list-elements.h list-origins.h parser.h \
 list-parser-errors.h player.h guid.h option.h list-options.h \
 list-player-flags.h
./sound-core.o: sound-core.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h init.h datafile.h parser.h list-parser-errors.h \
 sound.h main.h ui-term.h ui-event.h ui-prefs.h cave.h \
 list-square-flags.h list-terrain-flags.h ui-keymap.h
./source.o: source.c source.h
./store.o: store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h game-world.h hint.h init.h datafile.h parser.h \
 list-parser-errors.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-info.h z-textblock.h obj-knowledge.h obj-make.h \
 obj-pile.h obj-power.h obj-slays.h obj-tval.h obj-util.h player-calcs.h \
 player-history.h list-history-types.h player-spell.h store.h debug.h
./target.o: target.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h game-input.h mon-desc.h monster.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-util.h mon-msg.h \
 list-mon-message.h obj-ignore.h list-ignore-types.h player-calcs.h \
 player-timed.h list-player-timed.h project.h source.h list-projections.h
./trap.o: trap.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 effects.h source.h player-attack.h cmd-core.h cmds.h list-effects.h \
 init.h datafile.h parser.h list-parser-errors.h mon-util.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-msg.h \
 list-mon-message.h obj-knowledge.h player-timed.h list-player-timed.h \
 player-util.h trap.h list-trap-flags.h
./ui-birth.o: ui-birth.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-input.h obj-tval.h player-spell.h \
 ui-birth.h ui-display.h ui-game.h ui-help.h ui-input.h ui-event.h \
 ui-term.h ui-menu.h ui-output.h z-textblock.h ui-options.h obj-ignore.h \
 list-ignore-types.h ui-player.h ui-prefs.h datafile.h parser.h \
 list-parser-errors.h ui-keymap.h ui-target.h
./ui-command.o: ui-command.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h cmds.h game-input.h game-world.h init.h \
 datafile.h parser.h list-parser-errors.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-gear.h \
 list-equip-slots.h obj-util.h player-attack.h player-calcs.h \
 player-timed.h list-player-timed.h player-util.h store.h ui-command.h \
 ui-display.h ui-event.h ui-help.h ui-input.h ui-term.h ui-keymap.h \
 ui-map.h ui-menu.h ui-output.h z-textblock.h ui-options.h obj-ignore.h \
 list-ignore-types.h ui-player.h ui-prefs.h ui-target.h wizard.h
./ui-context.o: ui-context.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h cmds.h game-input.h mon-desc.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-lore.h \
 z-textblock.h mon-util.h mon-msg.h list-mon-message.h obj-chest.h \
 obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-info.h obj-tval.h obj-util.h player-calcs.h \
 player-timed.h list-player-timed.h player-util.h store.h datafile.h \
 parser.h list-parser-errors.h ui-context.h ui-input.h ui-event.h \
 ui-term.h ui-game.h ui-keymap.h ui-knowledge.h ui-menu.h ui-output.h \
 ui-mon-lore.h ui-object.h ui-player.h ui-spell.h ui-store.h ui-target.h \
 wizard.h
./ui-curse.o: ui-curse.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h init.h datafile.h parser.h list-parser-errors.h \
 obj-curse.h obj-knowledge.h ui-menu.h ui-output.h ui-event.h ui-term.h \
 z-textblock.h
./ui-death.o: ui-death.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-input.h init.h datafile.h parser.h \
 list-parser-errors.h obj-desc.h obj-info.h z-textblock.h savefile.h \
 store.h ui-death.h ui-history.h ui-input.h ui-event.h ui-term.h \
 ui-knowledge.h ui-menu.h ui-output.h ui-object.h ui-player.h ui-score.h \
 wizard.h
./ui-display.o: ui-display.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-world.h grafmode.h hint.h init.h \
 datafile.h parser.h list-parser-errors.h mon-lore.h z-textblock.h \
 monster.h target.h mon-predicate.h mon-timed.h list-mon-timed.h \
 mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-util.h mon-msg.h list-mon-message.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-pile.h obj-util.h player-calcs.h \
 player-timed.h list-player-timed.h player-util.h project.h source.h \
 list-projections.h savefile.h trap.h list-trap-flags.h ui-birth.h \
 ui-display.h ui-game.h ui-input.h ui-event.h ui-term.h ui-map.h \
 ui-mon-list.h ui-mon-lore.h ui-object.h ui-obj-list.h ui-output.h \
 ui-player.h ui-prefs.h ui-keymap.h ui-store.h ui-visuals.h wizard.h
./ui-entry.o: ui-entry.c init.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-file.h z-rand.h datafile.h object.h z-type.h z-quark.h z-dice.h \
 z-expression.h obj-properties.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h list-elements.h \
 list-origins.h parser.h list-parser-errors.h obj-curse.h obj-gear.h \
 player.h guid.h option.h list-options.h list-player-flags.h \
 list-equip-slots.h obj-knowledge.h angband.h z-color.h z-util.h config.h \
 game-event.h message.h list-message.h obj-util.h player-timed.h \
 list-player-timed.h ui-entry.h ui-entry-combiner.h ui-entry-init.h \
 ui-entry-renderers.h
./ui-entry-combiner.o: ui-entry-combiner.c ui-entry-combiner.h h-basic.h \
 z-virt.h
./ui-entry-renderers.o: ui-entry-renderers.c ui-entry-init.h datafile.h \
 object.h z-type.h h-basic.h z-quark.h z-bitflag.h z-form.h z-virt.h \
 z-dice.h z-rand.h z-expression.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h list-elements.h list-origins.h parser.h \
 list-parser-errors.h ui-entry-renderers.h ui-entry-combiner.h ui-term.h \
 ui-event.h z-color.h z-util.h list-ui-entry-renderers.h
./ui-equip-cmp.o: ui-equip-cmp.c cave.h z-type.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h list-square-flags.h list-terrain-flags.h game-input.h \
 cmd-core.h object.h z-quark.h z-dice.h z-rand.h z-expression.h \
 obj-properties.h z-file.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h list-elements.h \
 list-origins.h init.h datafile.h parser.h list-parser-errors.h \
 obj-desc.h obj-gear.h player.h guid.h option.h list-options.h \
 list-player-flags.h list-equip-slots.h obj-ignore.h list-ignore-types.h \
 obj-info.h z-textblock.h obj-tval.h store.h ui-entry.h \
 ui-entry-renderers.h ui-entry-combiner.h ui-equip-cmp.h ui-event.h \
 ui-input.h game-event.h ui-term.h ui-object.h ui-output.h z-color.h \
 z-util.h
./ui-event.o: ui-event.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h ui-event.h
./ui-game.o: ui-game.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h datafile.h parser.h list-parser-errors.h \
 game-world.h grafmode.h init.h mon-lore.h z-textblock.h monster.h \
 target.h mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-make.h \
 obj-knowledge.h obj-util.h player-attack.h player-calcs.h player-path.h \
 player-properties.h player-util.h savefile.h ui-birth.h ui-command.h \
 ui-context.h ui-input.h ui-event.h ui-term.h ui-death.h ui-display.h \
 ui-game.h ui-help.h ui-init.h ui-keymap.h ui-knowledge.h ui-map.h \
 ui-object.h ui-output.h ui-player.h ui-prefs.h ui-spell.h ui-score.h \
 ui-signals.h ui-store.h ui-target.h
./ui-help.o: ui-help.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h init.h datafile.h parser.h \
 list-parser-errors.h ui-input.h cmd-core.h ui-event.h ui-term.h \
 ui-output.h z-textblock.h
./ui-history.o: ui-history.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h player-history.h list-history-types.h ui-input.h \
 cmd-core.h ui-event.h ui-term.h ui-output.h z-textblock.h
./ui-init.o: ui-init.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h game-input.h cmd-core.h init.h datafile.h parser.h \
 list-parser-errors.h ui-display.h ui-game.h ui-input.h ui-event.h \
 ui-term.h ui-keymap.h ui-knowledge.h ui-options.h obj-ignore.h \
 list-ignore-types.h ui-output.h z-textblock.h ui-prefs.h cave.h \
 list-square-flags.h list-terrain-flags.h
./ui-input.o: ui-input.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-input.h game-world.h init.h \
 datafile.h parser.h list-parser-errors.h obj-gear.h list-equip-slots.h \
 obj-util.h player-calcs.h player-path.h randname.h savefile.h target.h \
 mon-predicate.h ui-command.h ui-context.h ui-input.h ui-event.h \
 ui-term.h ui-curse.h ui-display.h ui-help.h ui-keymap.h ui-knowledge.h \
 ui-map.h ui-object.h ui-output.h z-textblock.h ui-player.h ui-prefs.h \
 ui-signals.h ui-spell.h ui-store.h ui-target.h
./ui-keymap.o: ui-keymap.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h ui-keymap.h ui-term.h ui-event.h
./ui-knowledge.o: ui-knowledge.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h effects.h source.h player-attack.h list-effects.h \
 effects-info.h z-textblock.h game-input.h grafmode.h init.h datafile.h \
 parser.h list-parser-errors.h mon-lore.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h mon-util.h \
 mon-msg.h list-mon-message.h obj-desc.h obj-ignore.h list-ignore-types.h \
 obj-knowledge.h obj-info.h obj-make.h obj-pile.h obj-tval.h obj-util.h \
 player-calcs.h player-history.h list-history-types.h player-util.h \
 project.h list-projections.h store.h trap.h list-trap-flags.h \
 ui-context.h ui-input.h ui-event.h ui-term.h ui-equip-cmp.h ui-history.h \
 ui-menu.h ui-output.h ui-mon-list.h ui-mon-lore.h ui-object.h \
 ui-obj-list.h ui-options.h ui-prefs.h ui-keymap.h ui-score.h ui-store.h \
 ui-target.h wizard.h
./ui-map.o: ui-map.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 grafmode.h init.h datafile.h parser.h list-parser-errors.h \
 mon-predicate.h mon-util.h monster.h target.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-msg.h list-mon-message.h obj-tval.h obj-util.h \
 player-timed.h list-player-timed.h trap.h list-trap-flags.h ui-input.h \
 cmd-core.h ui-event.h ui-term.h ui-map.h ui-object.h ui-output.h \
 z-textblock.h ui-prefs.h ui-keymap.h
./ui-menu.o: ui-menu.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 ui-target.h ui-event.h ui-input.h cmd-core.h ui-term.h ui-menu.h \
 ui-output.h z-textblock.h
./ui-mon-list.o: ui-mon-list.c mon-desc.h monster.h h-basic.h z-bitflag.h \
 z-form.h z-virt.h z-rand.h cave.h z-type.h list-square-flags.h \
 list-terrain-flags.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-list.h angband.h z-color.h z-util.h config.h \
 game-event.h message.h list-message.h mon-lore.h z-textblock.h \
 mon-util.h mon-msg.h list-mon-message.h player-timed.h \
 list-player-timed.h ui-output.h ui-event.h ui-term.h ui-prefs.h \
 datafile.h parser.h list-parser-errors.h ui-keymap.h
./ui-mon-lore.o: ui-mon-lore.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h init.h datafile.h parser.h list-parser-errors.h \
 mon-lore.h z-textblock.h monster.h cave.h list-square-flags.h \
 list-terrain-flags.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h ui-mon-lore.h ui-output.h ui-event.h ui-term.h \
 ui-prefs.h ui-keymap.h
./ui-obj-list.o: ui-obj-list.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h init.h datafile.h parser.h list-parser-errors.h \
 obj-list.h obj-util.h ui-object.h cmd-core.h ui-obj-list.h ui-output.h \
 ui-event.h ui-term.h z-textblock.h
./ui-object.o: ui-object.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmd-core.h cmds.h effects.h source.h player-attack.h list-effects.h \
 game-input.h init.h datafile.h parser.h list-parser-errors.h obj-desc.h \
 obj-gear.h list-equip-slots.h obj-ignore.h list-ignore-types.h \
 obj-info.h z-textblock.h obj-knowledge.h obj-make.h obj-pile.h \
 obj-tval.h obj-util.h player-calcs.h player-spell.h player-timed.h \
 list-player-timed.h player-util.h store.h ui-command.h ui-display.h \
 ui-game.h ui-input.h ui-event.h ui-term.h ui-keymap.h ui-menu.h \
 ui-output.h ui-object.h ui-options.h ui-prefs.h
./ui-options.o: ui-options.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-input.h init.h datafile.h parser.h \
 list-parser-errors.h obj-desc.h obj-ignore.h list-ignore-types.h \
 obj-tval.h obj-util.h player-calcs.h ui-display.h ui-help.h ui-input.h \
 ui-event.h ui-term.h ui-keymap.h ui-knowledge.h ui-menu.h ui-output.h \
 z-textblock.h ui-options.h ui-prefs.h ui-target.h
./ui-output.o: ui-output.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 player-calcs.h ui-input.h cmd-core.h ui-event.h ui-term.h ui-output.h \
 z-textblock.h
./ui-player.o: ui-player.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h init.h datafile.h parser.h list-parser-errors.h \
 obj-curse.h obj-desc.h obj-gear.h list-equip-slots.h obj-info.h \
 z-textblock.h obj-knowledge.h obj-util.h player-calcs.h player-timed.h \
 list-player-timed.h player-util.h store.h cmd-core.h ui-display.h \
 ui-entry.h ui-entry-renderers.h ui-entry-combiner.h ui-history.h \
 ui-input.h ui-event.h ui-term.h ui-menu.h ui-output.h ui-object.h \
 ui-player.h
./ui-prefs.o: ui-prefs.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-input.h cmd-core.h grafmode.h init.h datafile.h parser.h \
 list-parser-errors.h mon-util.h monster.h target.h mon-predicate.h \
 mon-timed.h list-mon-timed.h mon-blows.h list-mon-temp-flags.h \
 list-mon-race-flags.h list-mon-spells.h mon-msg.h list-mon-message.h \
 obj-ignore.h list-ignore-types.h obj-tval.h obj-util.h project.h \
 source.h list-projections.h trap.h list-trap-flags.h ui-display.h \
 ui-entry-renderers.h ui-entry-combiner.h ui-keymap.h ui-prefs.h \
 ui-term.h ui-event.h sound.h
./ui-score.o: ui-score.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h score.h ui-input.h cmd-core.h ui-event.h ui-term.h \
 ui-output.h z-textblock.h ui-score.h
./ui-signals.o: ui-signals.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h game-world.h cave.h list-square-flags.h \
 list-terrain-flags.h savefile.h ui-game.h cmd-core.h ui-signals.h \
 ui-term.h ui-event.h
./ui-spell.o: ui-spell.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h game-input.h obj-tval.h obj-util.h player-calcs.h \
 player-spell.h ui-menu.h ui-output.h ui-event.h ui-term.h z-textblock.h
./ui-store.o: ui-store.c angband.h h-basic.h z-bitflag.h z-form.h z-virt.h \
 z-color.h z-util.h z-rand.h config.h game-event.h z-type.h message.h \
 list-message.h player.h guid.h obj-properties.h z-file.h list-tvals.h \
 list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h game-input.h hint.h init.h datafile.h parser.h \
 list-parser-errors.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h obj-desc.h obj-gear.h list-equip-slots.h obj-ignore.h \
 list-ignore-types.h obj-info.h z-textblock.h obj-knowledge.h obj-make.h \
 obj-pile.h obj-tval.h obj-util.h player-calcs.h player-history.h \
 list-history-types.h player-util.h store.h ui-display.h ui-input.h \
 ui-event.h ui-term.h ui-menu.h ui-output.h ui-object.h ui-options.h \
 ui-knowledge.h ui-player.h ui-spell.h ui-command.h ui-store.h z-debug.h
./ui-target.o: ui-target.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 game-input.h cmd-core.h init.h datafile.h parser.h list-parser-errors.h \
 mon-desc.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h mon-lore.h z-textblock.h obj-desc.h obj-pile.h \
 obj-util.h player-attack.h player-calcs.h player-timed.h \
 list-player-timed.h project.h source.h list-projections.h trap.h \
 list-trap-flags.h ui-display.h ui-input.h ui-event.h ui-term.h \
 ui-keymap.h ui-map.h ui-mon-lore.h ui-object.h ui-output.h ui-target.h
./ui-term.o: ui-term.c buildid.h h-basic.h ui-term.h ui-event.h z-color.h \
 z-util.h z-virt.h
./ui-visuals.o: ui-visuals.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h init.h datafile.h parser.h list-parser-errors.h \
 monster.h cave.h list-square-flags.h list-terrain-flags.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h \
 ui-visuals.h
./wiz-debug.o: wiz-debug.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h effects.h source.h player-attack.h list-effects.h \
 game-input.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h grafmode.h init.h datafile.h parser.h \
 list-parser-errors.h mon-lore.h z-textblock.h mon-make.h mon-util.h \
 mon-msg.h list-mon-message.h obj-curse.h obj-desc.h obj-gear.h \
 list-equip-slots.h obj-knowledge.h obj-make.h obj-pile.h obj-power.h \
 obj-slays.h obj-tval.h obj-util.h player-calcs.h player-timed.h \
 list-player-timed.h player-util.h project.h list-projections.h trap.h \
 list-trap-flags.h ui-command.h ui-event.h ui-display.h ui-help.h \
 ui-input.h ui-term.h ui-map.h ui-menu.h ui-output.h ui-prefs.h \
 ui-keymap.h ui-target.h wizard.h
./wiz-spoil.o: wiz-spoil.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h buildid.h cmds.h cave.h list-square-flags.h \
 list-terrain-flags.h cmd-core.h game-world.h init.h datafile.h parser.h \
 list-parser-errors.h mon-lore.h z-textblock.h monster.h target.h \
 mon-predicate.h mon-timed.h list-mon-timed.h mon-blows.h \
 list-mon-temp-flags.h list-mon-race-flags.h list-mon-spells.h obj-desc.h \
 obj-info.h obj-make.h obj-pile.h obj-power.h obj-tval.h obj-util.h \
 ui-input.h ui-event.h ui-term.h ui-knowledge.h ui-menu.h ui-output.h \
 ui-mon-lore.h wizard.h
./wiz-stats.o: wiz-stats.c angband.h h-basic.h z-bitflag.h z-form.h \
 z-virt.h z-color.h z-util.h z-rand.h config.h game-event.h z-type.h \
 message.h list-message.h player.h guid.h obj-properties.h z-file.h \
 list-tvals.h list-object-flags.h list-kind-flags.h list-stats.h \
 list-object-modifiers.h object.h z-quark.h z-dice.h z-expression.h \
 list-elements.h list-origins.h option.h list-options.h \
 list-player-flags.h cave.h list-square-flags.h list-terrain-flags.h \
 cmds.h cmd-core.h effects.h source.h player-attack.h list-effects.h \
 game-input.h generate.h monster.h target.h mon-predicate.h mon-timed.h \
 list-mon-timed.h mon-blows.h list-mon-temp-flags.h list-mon-race-flags.h \
 list-mon-spells.h init.h datafile.h parser.h list-parser-errors.h \
 mon-make.h obj-init.h obj-pile.h obj-randart.h list-randart-properties.h \
 obj-tval.h obj-util.h ui-command.h wizard.h
./buildid.o: buildid.c buildid.h
./z-bitflag.o: z-bitflag.c z-bitflag.h h-basic.h z-form.h z-virt.h
./z-color.o: z-color.c h-basic.h z-color.h z-util.h
./z-dice.o: z-dice.c z-dice.h h-basic.h z-rand.h z-expression.h z-virt.h \
 z-util.h
./z-expression.o: z-expression.c z-expression.h h-basic.h z-virt.h z-util.h
./z-file.o: z-file.c h-basic.h z-file.h z-form.h z-util.h z-virt.h
./z-form.o: z-form.c z-form.h h-basic.h z-type.h z-util.h z-virt.h
./z-quark.o: z-quark.c z-virt.h h-basic.h z-quark.h init.h z-bitflag.h \
 z-form.h z-file.h z-rand.h datafile.h object.h z-type.h z-dice.h \
 z-expression.h obj-properties.h list-tvals.h list-object-flags.h \
 list-kind-flags.h list-stats.h list-object-modifiers.h list-elements.h \
 list-origins.h parser.h list-parser-errors.h
./z-queue.o: z-queue.c z-queue.h h-basic.h
./z-rand.o: z-rand.c z-rand.h h-basic.h
./z-set.o: z-set.c z-set.h h-basic.h z-rand.h z-virt.h
./z-textblock.o: z-textblock.c z-color.h h-basic.h z-textblock.h z-file.h \
 z-util.h z-virt.h z-form.h
./z-type.o: z-type.c z-rand.h h-basic.h z-type.h z-virt.h
./z-util.o: z-util.c z-util.h h-basic.h
./z-virt.o: z-virt.c z-virt.h h-basic.h z-util.h
