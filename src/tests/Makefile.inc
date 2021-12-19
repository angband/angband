#
# File: Makefile.inc
#
# Holds dependency information for the test cases.  Expects TESTOBJS to
# be set by whatever includes this (src/tests/Makefile normally).
#

depgen:
	head -n 16 Makefile.inc > Makefile.new
	@for i in $(TESTOBJS:.o=.c); do \
		DIR=`echo ./$$i | sed 's|/[^/]*$$||'`; \
			$(CC) -MM -I. -I.. $$i | \
			sed "s|^\\([^ ]\\)|$$DIR/\\1|" >> Makefile.new; \
	done;

# Dependencies
./artifact/name.o: artifact/name.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../obj-properties.h ../obj-randart.h ../list-randart-properties.h \
 ../object.h
./cave/scatter.o: cave/scatter.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h test-utils.h ../cave.h ../z-type.h ../z-bitflag.h \
 ../z-form.h ../z-virt.h ../list-square-flags.h ../list-terrain-flags.h \
 ../init.h ../z-file.h ../z-rand.h ../datafile.h ../object.h ../z-quark.h \
 ../z-dice.h ../z-expression.h ../obj-properties.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../list-elements.h ../list-origins.h \
 ../parser.h ../list-parser-errors.h
./command/lookup.o: command/lookup.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../obj-properties.h ../z-file.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../object.h ../z-type.h \
 ../z-quark.h ../z-dice.h ../z-rand.h ../z-expression.h \
 ../obj-properties.h ../list-elements.h ../list-origins.h ../cmds.h \
 ../cave.h ../list-square-flags.h ../list-terrain-flags.h ../cmd-core.h \
 ../object.h ../ui-keymap.h ../ui-event.h ../ui-game.h ../game-event.h \
 ../ui-input.h ../ui-event.h ../ui-term.h ../z-virt.h
./effects/chain.o: effects/chain.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h test-utils.h ../cave.h ../z-type.h ../z-bitflag.h \
 ../z-form.h ../z-virt.h ../list-square-flags.h ../list-terrain-flags.h \
 ../effects.h ../source.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-rand.h ../z-expression.h ../obj-properties.h ../z-file.h \
 ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../list-elements.h \
 ../list-origins.h ../player-attack.h ../cmd-core.h ../cmds.h ../cave.h \
 ../list-effects.h ../effects-info.h ../z-textblock.h ../game-world.h \
 ../generate.h ../monster.h ../target.h ../mon-predicate.h ../mon-timed.h \
 ../list-mon-timed.h ../mon-blows.h ../player.h ../guid.h ../option.h \
 ../list-options.h ../list-player-flags.h ../list-mon-temp-flags.h \
 ../list-mon-race-flags.h ../list-mon-spells.h ../list-room-flags.h \
 ../init.h ../datafile.h ../parser.h ../list-parser-errors.h \
 ../mon-make.h ../player.h ../player-birth.h ../player-calcs.h \
 ../player-timed.h ../list-player-timed.h ../source.h ../z-dice.h
./effects/destruction.o: effects/destruction.c unit-test.h unit-test-types.h \
 ../z-util.h ../h-basic.h test-utils.h ../cave.h ../z-type.h \
 ../z-bitflag.h ../z-form.h ../z-virt.h ../list-square-flags.h \
 ../list-terrain-flags.h ../effects.h ../source.h ../object.h \
 ../z-quark.h ../z-dice.h ../z-rand.h ../z-expression.h \
 ../obj-properties.h ../z-file.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../list-elements.h ../list-origins.h ../player-attack.h ../cmd-core.h \
 ../cmds.h ../cave.h ../list-effects.h ../game-world.h ../init.h \
 ../datafile.h ../parser.h ../list-parser-errors.h ../obj-knowledge.h \
 ../angband.h ../z-color.h ../z-util.h ../config.h ../game-event.h \
 ../message.h ../list-message.h ../player.h ../guid.h ../option.h \
 ../list-options.h ../list-player-flags.h ../obj-make.h ../obj-pile.h \
 ../obj-util.h ../player-birth.h ../player-util.h
./effects/earthquake.o: effects/earthquake.c unit-test.h unit-test-types.h \
 ../z-util.h ../h-basic.h test-utils.h ../cave.h ../z-type.h \
 ../z-bitflag.h ../z-form.h ../z-virt.h ../list-square-flags.h \
 ../list-terrain-flags.h ../effects.h ../source.h ../object.h \
 ../z-quark.h ../z-dice.h ../z-rand.h ../z-expression.h \
 ../obj-properties.h ../z-file.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../list-elements.h ../list-origins.h ../player-attack.h ../cmd-core.h \
 ../cmds.h ../cave.h ../list-effects.h ../game-world.h ../init.h \
 ../datafile.h ../parser.h ../list-parser-errors.h ../obj-knowledge.h \
 ../angband.h ../z-color.h ../z-util.h ../config.h ../game-event.h \
 ../message.h ../list-message.h ../player.h ../guid.h ../option.h \
 ../list-options.h ../list-player-flags.h ../obj-make.h ../obj-pile.h \
 ../obj-util.h ../player-birth.h ../player-util.h
./effects/info.o: effects/info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h test-utils.h ../effects.h ../source.h ../object.h \
 ../z-type.h ../z-quark.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../z-dice.h ../z-rand.h ../z-expression.h ../obj-properties.h \
 ../z-file.h ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../list-elements.h \
 ../list-origins.h ../player-attack.h ../cmd-core.h ../cmds.h ../cave.h \
 ../list-square-flags.h ../list-terrain-flags.h ../list-effects.h \
 ../effects-info.h ../z-textblock.h ../init.h ../datafile.h ../parser.h \
 ../list-parser-errors.h ../z-dice.h
./game/basic.o: game/basic.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 test-utils.h ../cave.h ../cmd-core.h ../game-event.h ../game-world.h \
 ../generate.h ../list-room-flags.h ../mon-make.h ../savefile.h \
 ../player-birth.h ../cmd-core.h ../player-timed.h ../list-player-timed.h
./game/mage.o: game/mage.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 test-utils.h ../cave.h ../cmd-core.h ../game-event.h ../game-world.h \
 ../generate.h ../list-room-flags.h ../mon-make.h ../savefile.h \
 ../player-birth.h ../cmd-core.h ../player-timed.h ../list-player-timed.h
./message/message.o: message/message.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../message.h ../game-event.h ../z-color.h ../z-virt.h
./monster/attack.o: monster/attack.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../mon-attack.h ../option.h ../player-timed.h ../list-player-timed.h \
 ../ui-input.h ../cmd-core.h ../ui-event.h ../ui-term.h
./monster/monster.o: monster/monster.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 test-utils.h ../mon-util.h ../mon-msg.h ../list-mon-message.h
./object/alloc.o: object/alloc.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../object.h ../obj-make.h ../obj-properties.h
./object/attack.o: object/attack.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../object.h ../obj-make.h ../player-attack.h ../cmd-core.h
./object/util.o: object/util.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../object.h ../obj-make.h ../obj-pile.h ../obj-util.h
./object/pile.o: object/pile.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../object.h ../obj-pile.h
./object/slays.o: object/slays.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 test-utils.h ../mon-spell.h ../obj-slays.h ../player-birth.h \
 ../cmd-core.h ../player-timed.h ../list-player-timed.h ../z-color.h \
 ../z-virt.h
./parse/a-info.o: parse/a-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../effects.h ../player-attack.h ../cmd-core.h ../cmds.h \
 ../list-effects.h ../obj-util.h ../object.h
./parse/c-info.o: parse/c-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../init.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../z-file.h ../z-rand.h ../datafile.h ../object.h ../z-type.h \
 ../z-quark.h ../z-dice.h ../z-expression.h ../obj-properties.h \
 ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../list-elements.h \
 ../list-origins.h ../parser.h ../list-parser-errors.h \
 ../obj-properties.h ../object.h ../obj-tval.h ../obj-util.h ../player.h \
 ../guid.h ../option.h ../list-options.h ../list-player-flags.h
./parse/e-info.o: parse/e-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../object.h
./parse/f-info.o: parse/f-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h
./parse/flavor.o: parse/flavor.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../init.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../z-file.h ../z-rand.h ../datafile.h ../object.h ../z-type.h \
 ../z-quark.h ../z-dice.h ../z-expression.h ../obj-properties.h \
 ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../list-elements.h \
 ../list-origins.h ../parser.h ../list-parser-errors.h ../obj-tval.h \
 ../obj-properties.h ../object.h ../z-color.h
./parse/graphics.o: parse/graphics.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h test-utils.h ../game-event.h ../z-type.h ../init.h \
 ../z-bitflag.h ../z-form.h ../z-virt.h ../z-file.h ../z-rand.h \
 ../datafile.h ../object.h ../z-quark.h ../z-dice.h ../z-expression.h \
 ../obj-properties.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../list-elements.h ../list-origins.h ../parser.h ../list-parser-errors.h \
 ../message.h ../list-message.h ../grafmode.h ../player.h ../guid.h \
 ../option.h ../list-options.h ../list-player-flags.h ../player-birth.h \
 ../cmd-core.h ../ui-prefs.h ../cave.h ../list-square-flags.h \
 ../list-terrain-flags.h ../ui-keymap.h ../ui-term.h ../ui-event.h \
 ../mon-util.h ../monster.h ../target.h ../mon-predicate.h ../mon-timed.h \
 ../list-mon-timed.h ../mon-blows.h ../player.h ../list-mon-temp-flags.h \
 ../list-mon-race-flags.h ../list-mon-spells.h ../mon-msg.h \
 ../list-mon-message.h ../obj-tval.h
./parse/h-info.o: parse/h-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../init.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../z-file.h ../z-rand.h ../datafile.h ../object.h ../z-type.h \
 ../z-quark.h ../z-dice.h ../z-expression.h ../obj-properties.h \
 ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../list-elements.h \
 ../list-origins.h ../parser.h ../list-parser-errors.h ../player.h \
 ../guid.h ../option.h ../list-options.h ../list-player-flags.h
./parse/k-info.o: parse/k-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h
./parse/lore.o: parse/lore.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h test-utils.h ../init.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-file.h ../z-rand.h ../datafile.h ../object.h \
 ../z-type.h ../z-quark.h ../z-dice.h ../z-expression.h \
 ../obj-properties.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../list-elements.h ../list-origins.h ../parser.h ../list-parser-errors.h \
 ../mon-init.h
./parse/names.o: parse/names.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../init.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../z-file.h ../z-rand.h ../datafile.h ../object.h ../z-type.h \
 ../z-quark.h ../z-dice.h ../z-expression.h ../obj-properties.h \
 ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../list-elements.h \
 ../list-origins.h ../parser.h ../list-parser-errors.h ../randname.h
./parse/p-info.o: parse/p-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../init.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../z-file.h ../z-rand.h ../datafile.h ../object.h ../z-type.h \
 ../z-quark.h ../z-dice.h ../z-expression.h ../obj-properties.h \
 ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../list-elements.h \
 ../list-origins.h ../parser.h ../list-parser-errors.h ../player.h \
 ../guid.h ../option.h ../list-options.h ../list-player-flags.h
./parse/parse.o: parse/parse.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../parser.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../z-dice.h ../z-rand.h ../z-expression.h ../list-parser-errors.h
./parse/r-info.o: parse/r-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h
./parse/readstore.o: parse/readstore.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../object.h ../z-type.h ../z-quark.h ../z-bitflag.h \
 ../z-form.h ../z-virt.h ../z-dice.h ../z-rand.h ../z-expression.h \
 ../obj-properties.h ../z-file.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../list-elements.h ../list-origins.h ../store.h ../cave.h \
 ../list-square-flags.h ../list-terrain-flags.h ../cmd-core.h ../object.h \
 ../datafile.h ../parser.h ../list-parser-errors.h ../init.h
./parse/v-info.o: parse/v-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../init.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../z-file.h ../z-rand.h ../datafile.h ../object.h ../z-type.h \
 ../z-quark.h ../z-dice.h ../z-expression.h ../obj-properties.h \
 ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../list-elements.h \
 ../list-origins.h ../parser.h ../list-parser-errors.h ../cave.h \
 ../list-square-flags.h ../list-terrain-flags.h ../generate.h \
 ../monster.h ../cave.h ../target.h ../mon-predicate.h ../mon-timed.h \
 ../list-mon-timed.h ../mon-blows.h ../player.h ../guid.h ../option.h \
 ../list-options.h ../list-player-flags.h ../list-mon-temp-flags.h \
 ../list-mon-race-flags.h ../list-mon-spells.h ../list-room-flags.h
./parse/z-info.o: parse/z-info.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h
./player/birth.o: player/birth.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../player-birth.h ../cmd-core.h ../player-quest.h
./player/calc-inventory.o: player/calc-inventory.c unit-test.h unit-test-types.h \
 ../z-util.h ../h-basic.h test-utils.h ../cave.h ../z-type.h \
 ../z-bitflag.h ../z-form.h ../z-virt.h ../list-square-flags.h \
 ../list-terrain-flags.h ../game-world.h ../cave.h ../generate.h \
 ../monster.h ../z-rand.h ../target.h ../mon-predicate.h ../mon-timed.h \
 ../list-mon-timed.h ../mon-blows.h ../player.h ../guid.h \
 ../obj-properties.h ../z-file.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../object.h ../z-quark.h ../z-dice.h ../z-expression.h \
 ../list-elements.h ../list-origins.h ../option.h ../list-options.h \
 ../list-player-flags.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../list-room-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-make.h ../obj-gear.h \
 ../list-equip-slots.h ../obj-knowledge.h ../angband.h ../z-color.h \
 ../z-util.h ../config.h ../game-event.h ../message.h ../list-message.h \
 ../obj-make.h ../obj-pile.h ../obj-properties.h ../obj-tval.h \
 ../obj-util.h ../player-birth.h ../cmd-core.h ../player-calcs.h \
 ../z-quark.h
./player/combine-pack.o: player/combine-pack.c unit-test.h unit-test-types.h \
 ../z-util.h ../h-basic.h test-utils.h ../cave.h ../z-type.h \
 ../z-bitflag.h ../z-form.h ../z-virt.h ../list-square-flags.h \
 ../list-terrain-flags.h ../game-world.h ../cave.h ../generate.h \
 ../monster.h ../z-rand.h ../target.h ../mon-predicate.h ../mon-timed.h \
 ../list-mon-timed.h ../mon-blows.h ../player.h ../guid.h \
 ../obj-properties.h ../z-file.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../object.h ../z-quark.h ../z-dice.h ../z-expression.h \
 ../list-elements.h ../list-origins.h ../option.h ../list-options.h \
 ../list-player-flags.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../list-room-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-make.h ../obj-gear.h \
 ../list-equip-slots.h ../obj-knowledge.h ../angband.h ../z-color.h \
 ../z-util.h ../config.h ../game-event.h ../message.h ../list-message.h \
 ../obj-make.h ../obj-pile.h ../obj-tval.h ../obj-util.h \
 ../player-birth.h ../cmd-core.h ../player-calcs.h ../z-quark.h
./player/history.o: player/history.c ../player.h ../guid.h ../obj-properties.h \
 ../z-file.h ../h-basic.h ../z-bitflag.h ../z-form.h ../z-virt.h \
 ../list-tvals.h ../list-object-flags.h ../list-kind-flags.h \
 ../list-stats.h ../list-object-modifiers.h ../object.h ../z-type.h \
 ../z-quark.h ../z-dice.h ../z-rand.h ../z-expression.h \
 ../list-elements.h ../list-origins.h ../option.h ../list-options.h \
 ../list-player-flags.h ../player-birth.h ../cmd-core.h unit-test.h \
 unit-test-types.h ../z-util.h
./player/inven-carry-num.o: player/inven-carry-num.c test-utils.h unit-test.h \
 unit-test-types.h ../z-util.h ../h-basic.h ../init.h ../z-bitflag.h \
 ../z-form.h ../z-virt.h ../z-file.h ../z-rand.h ../datafile.h \
 ../object.h ../z-type.h ../z-quark.h ../z-dice.h ../z-expression.h \
 ../obj-properties.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../list-elements.h ../list-origins.h ../parser.h ../list-parser-errors.h \
 ../obj-gear.h ../player.h ../guid.h ../option.h ../list-options.h \
 ../list-player-flags.h ../list-equip-slots.h ../obj-knowledge.h \
 ../angband.h ../z-color.h ../z-util.h ../config.h ../game-event.h \
 ../message.h ../list-message.h ../obj-make.h ../cave.h \
 ../list-square-flags.h ../list-terrain-flags.h ../obj-pile.h \
 ../obj-util.h ../player-birth.h ../cmd-core.h ../player-calcs.h
./player/inven-wield.o: player/inven-wield.c unit-test.h unit-test-types.h \
 ../z-util.h ../h-basic.h test-utils.h ../cave.h ../z-type.h \
 ../z-bitflag.h ../z-form.h ../z-virt.h ../list-square-flags.h \
 ../list-terrain-flags.h ../effects.h ../source.h ../object.h \
 ../z-quark.h ../z-dice.h ../z-rand.h ../z-expression.h \
 ../obj-properties.h ../z-file.h ../list-tvals.h ../list-object-flags.h \
 ../list-kind-flags.h ../list-stats.h ../list-object-modifiers.h \
 ../list-elements.h ../list-origins.h ../player-attack.h ../cmd-core.h \
 ../cmds.h ../cave.h ../list-effects.h ../game-world.h ../generate.h \
 ../monster.h ../target.h ../mon-predicate.h ../mon-timed.h \
 ../list-mon-timed.h ../mon-blows.h ../player.h ../guid.h ../option.h \
 ../list-options.h ../list-player-flags.h ../list-mon-temp-flags.h \
 ../list-mon-race-flags.h ../list-mon-spells.h ../list-room-flags.h \
 ../init.h ../datafile.h ../parser.h ../list-parser-errors.h \
 ../mon-make.h ../obj-curse.h ../obj-gear.h ../list-equip-slots.h \
 ../obj-knowledge.h ../angband.h ../z-color.h ../z-util.h ../config.h \
 ../game-event.h ../message.h ../list-message.h ../obj-make.h \
 ../obj-pile.h ../obj-tval.h ../obj-util.h ../player-birth.h ../z-quark.h
./player/pathfind.o: player/pathfind.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../cave.h ../z-type.h ../z-bitflag.h ../z-form.h \
 ../z-virt.h ../list-square-flags.h ../list-terrain-flags.h \
 ../player-path.h
./player/playerstat.o: player/playerstat.c unit-test.h unit-test-types.h \
 ../z-util.h ../h-basic.h unit-test-data.h ../angband.h ../z-bitflag.h \
 ../z-form.h ../z-virt.h ../z-color.h ../z-util.h ../z-rand.h ../config.h \
 ../game-event.h ../z-type.h ../message.h ../list-message.h ../player.h \
 ../guid.h ../obj-properties.h ../z-file.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../object.h ../z-quark.h ../z-dice.h \
 ../z-expression.h ../list-elements.h ../list-origins.h ../option.h \
 ../list-options.h ../list-player-flags.h ../init.h ../datafile.h \
 ../parser.h ../list-parser-errors.h ../mon-lore.h ../z-textblock.h \
 ../monster.h ../cave.h ../list-square-flags.h ../list-terrain-flags.h \
 ../target.h ../mon-predicate.h ../mon-timed.h ../list-mon-timed.h \
 ../mon-blows.h ../list-mon-temp-flags.h ../list-mon-race-flags.h \
 ../list-mon-spells.h ../monster.h ../obj-tval.h ../player.h \
 ../player-calcs.h ../project.h ../source.h ../list-projections.h \
 ../player-birth.h ../cmd-core.h
./trivial/trivial.o: trivial/trivial.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h
./z-dice/dice.o: z-dice/dice.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../z-dice.h ../z-rand.h ../z-expression.h
./z-expression/expression.o: z-expression/expression.c unit-test.h unit-test-types.h \
 ../z-util.h ../h-basic.h ../z-expression.h
./z-quark/quark.o: z-quark/quark.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../z-quark.h
./z-textblock/textblock.o: z-textblock/textblock.c unit-test.h unit-test-types.h \
 ../z-util.h ../h-basic.h ../z-color.h ../z-textblock.h ../z-file.h
./z-util/util.o: z-util/util.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../z-quark.h
./z-virt/mem.o: z-virt/mem.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../z-virt.h
./z-virt/string.o: z-virt/string.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h ../z-virt.h
./test-utils.o: test-utils.c ../h-basic.h ../config.h ../init.h \
 ../h-basic.h ../z-bitflag.h ../z-form.h ../z-virt.h ../z-file.h \
 ../z-rand.h ../datafile.h ../object.h ../z-type.h ../z-quark.h \
 ../z-dice.h ../z-expression.h ../obj-properties.h ../list-tvals.h \
 ../list-object-flags.h ../list-kind-flags.h ../list-stats.h \
 ../list-object-modifiers.h ../list-elements.h ../list-origins.h \
 ../parser.h ../list-parser-errors.h test-utils.h ../z-util.h
./unit-test.o: unit-test.c unit-test.h unit-test-types.h ../z-util.h \
 ../h-basic.h
