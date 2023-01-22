MKPATH=../../mk/
include $(MKPATH)buildsys.mk

FILES = activation.txt artifact.txt body.txt blow_methods.txt \
 blow_effects.txt brand.txt chest_trap.txt class.txt constants.txt curse.txt \
 dungeon_profile.txt ego_item.txt flavor.txt hints.txt history.txt \
 monster.txt monster_base.txt monster_spell.txt names.txt object.txt \
 object_base.txt object_property.txt p_race.txt pain.txt pit.txt \
 player_property.txt player_timed.txt projection.txt quest.txt realm.txt \
 room_template.txt shape.txt slay.txt store.txt summon.txt terrain.txt \
 trap.txt ui_entry.txt ui_entry_base.txt ui_entry_renderer.txt \
 ui_knowledge.txt vault.txt visuals.txt world.txt

R1 = $(GAMEDATA_IN_LIB:true=$(FILES))
DATA = $(R1:false=)
R2 = $(GAMEDATA_IN_LIB:false=$(FILES))
CONFIG = $(R2:true=)

PACKAGE = gamedata
