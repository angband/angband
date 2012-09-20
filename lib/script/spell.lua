SPELL_MAGIC_MISSILE = 0
SPELL_DETECT_MONSTERS = 1
SPELL_PHASE_DOOR = 2
SPELL_LIGHT_AREA = 3
SPELL_FIND_TRAPS_DOORS = 4
SPELL_CURE_LIGHT_WOUNDS = 5
SPELL_TREASURE_DETECTION = 6
SPELL_OBJECT_DETECTION = 7
SPELL_IDENTIFY = 8
SPELL_DETECT_INVISIBLE = 9
SPELL_DETECT_ENCHANTMENT = 10
SPELL_STINKING_CLOUD = 11
SPELL_LIGHTNING_BOLT = 12
SPELL_CONFUSE_MONSTER = 13
SPELL_SLEEP_MONSTER = 14
SPELL_WONDER = 15
SPELL_FROST_BOLT = 16
SPELL_ACID_BOLT = 17
SPELL_FIRE_BOLT = 18
SPELL_TRAP_DOOR_DESTRUCTION = 19
SPELL_SPEAR_OF_LIGHT = 20
SPELL_TURN_STONE_TO_MUD = 21
SPELL_DOOR_CREATION = 22
SPELL_EARTHQUAKE = 23
SPELL_STAIR_CREATION = 24
SPELL_CURE_POISON = 25
SPELL_SATISFY_HUNGER = 26
SPELL_HEROISM = 27
SPELL_BERSERKER = 28
SPELL_HASTE_SELF = 29
SPELL_TELEPORT_SELF = 30
SPELL_SLOW_MONSTER = 31
SPELL_TELEPORT_OTHER = 32
SPELL_TELEPORT_LEVEL = 33
SPELL_WORD_OF_RECALL = 34
SPELL_POLYMORPH_OTHER = 35
SPELL_SHOCK_WAVE = 36
SPELL_EXPLOSION = 37
SPELL_CLOUD_KILL = 38
SPELL_MASS_SLEEP = 39
SPELL_BEDLAM = 40
SPELL_REND_SOUL = 41
SPELL_WORD_OF_DESTRUCTION = 42
SPELL_CHAOS_STRIKE = 43
SPELL_RESIST_COLD = 44
SPELL_RESIST_FIRE = 45
SPELL_RESIST_POISON = 46
SPELL_RESISTANCE = 47
SPELL_SHIELD = 48
SPELL_RUNE_OF_PROTECTION = 49
SPELL_RECHARGE_ITEM_I = 50
SPELL_ENCHANT_ARMOR = 51
SPELL_ENCHANT_WEAPON = 52
SPELL_RECHARGE_ITEM_II = 53 -- ToDo - replace one of the recharge spells
SPELL_ELEMENTAL_BRAND = 54
SPELL_FROST_BALL = 55
SPELL_ACID_BALL = 56
SPELL_FIRE_BALL = 57
SPELL_ICE_STORM = 58
SPELL_GENOCIDE = 59
SPELL_METEOR_SWARM = 60
SPELL_MASS_GENOCIDE = 61
SPELL_RIFT = 62
SPELL_MANA_STORM = 63

-- Beginners Handbook
PRAYER_DETECT_EVIL = 0
PRAYER_CURE_LIGHT_WOUNDS = 1
PRAYER_BLESS = 2
PRAYER_REMOVE_FEAR = 3
PRAYER_CALL_LIGHT = 4
PRAYER_FIND_TRAPS = 5
PRAYER_DETECT_DOORS_STAIRS = 6
PRAYER_SLOW_POISON = 7

-- Words of Wisdom
PRAYER_SCARE_MONSTER = 8
PRAYER_PORTAL = 9
PRAYER_CURE_SERIOUS_WOUNDS = 10
PRAYER_CHANT = 11
PRAYER_SANCTUARY = 12
PRAYER_SATISFY_HUNGER = 13
PRAYER_REMOVE_CURSE = 14
PRAYER_RESIST_HEAT_COLD = 15

-- Chants and Blessings
PRAYER_NEUTRALIZE_POISON = 16
PRAYER_ORB_OF_DRAINING = 17
PRAYER_CURE_CRITICAL_WOUNDS = 18
PRAYER_SENSE_INVISIBLE = 19
PRAYER_PROTECTION_FROM_EVIL = 20
PRAYER_EARTHQUAKE = 21
PRAYER_SENSE_SURROUNDINGS = 22
PRAYER_CURE_MORTAL_WOUNDS = 23
PRAYER_TURN_UNDEAD = 24

-- Exorcism and Dispelling
PRAYER_PRAYER = 25
PRAYER_DISPEL_UNDEAD = 26
PRAYER_HEAL = 27
PRAYER_DISPEL_EVIL = 28
PRAYER_GLYPH_OF_WARDING = 29
PRAYER_HOLY_WORD = 30

-- Godly Insights
PRAYER_DETECT_MONSTERS = 31
PRAYER_DETECTION = 32
PRAYER_PERCEPTION = 33
PRAYER_PROBING = 34
PRAYER_CLAIRVOYANCE = 35

-- Purifications and Healing
PRAYER_CURE_SERIOUS_WOUNDS2 = 36
PRAYER_CURE_MORTAL_WOUNDS2 = 37
PRAYER_HEALING = 38
PRAYER_RESTORATION = 39
PRAYER_REMEMBRANCE = 40

-- Wrath of God
PRAYER_DISPEL_UNDEAD2 = 41
PRAYER_DISPEL_EVIL2 = 42
PRAYER_BANISHMENT = 43
PRAYER_WORD_OF_DESTRUCTION = 44
PRAYER_ANNIHILATION = 45

-- Holy Infusions
PRAYER_UNBARRING_WAYS = 46
PRAYER_RECHARGING = 47
PRAYER_DISPEL_CURSE = 48
PRAYER_ENCHANT_WEAPON = 49
PRAYER_ENCHANT_ARMOUR = 50
PRAYER_ELEMENTAL_BRAND = 51

-- Ethereal openings
PRAYER_BLINK = 52
PRAYER_TELEPORT_SELF = 53
PRAYER_TELEPORT_OTHER = 54
PRAYER_TELEPORT_LEVEL = 55
PRAYER_WORD_OF_RECALL = 56
PRAYER_ALTER_REALITY = 57


magic_books = {
	-- An Introduction to Spellcasting
	[0] = {SPELL_MAGIC_MISSILE,
	       SPELL_DETECT_MONSTERS,
	       SPELL_PHASE_DOOR,
	       SPELL_LIGHT_AREA,
	       SPELL_FIND_TRAPS_DOORS,
	       SPELL_CURE_LIGHT_WOUNDS},
	-- The Art of Divination
	[1] = {SPELL_DETECT_MONSTERS,
	       SPELL_FIND_TRAPS_DOORS,
	       SPELL_TREASURE_DETECTION,
	       SPELL_OBJECT_DETECTION,
	       SPELL_IDENTIFY,
	       SPELL_DETECT_INVISIBLE,
	       SPELL_DETECT_ENCHANTMENT},
	-- Elementary Evocations
	[2] = {SPELL_MAGIC_MISSILE,
	       SPELL_STINKING_CLOUD,
	       SPELL_LIGHTNING_BOLT,
	       SPELL_CONFUSE_MONSTER,
	       SPELL_SLEEP_MONSTER,
	       SPELL_WONDER,
	       SPELL_FROST_BOLT,
	       SPELL_ACID_BOLT,
	       SPELL_FIRE_BOLT},
	-- Applied Conjuration
	[3] = {SPELL_LIGHT_AREA,
	       SPELL_TRAP_DOOR_DESTRUCTION,
	       SPELL_SPEAR_OF_LIGHT,
	       SPELL_TURN_STONE_TO_MUD,
	       SPELL_DOOR_CREATION,
	       SPELL_EARTHQUAKE,
	       SPELL_STAIR_CREATION},
	-- Anatomy and Physiology
	[4] = {SPELL_CURE_LIGHT_WOUNDS,
	       SPELL_CURE_POISON,
	       SPELL_SATISFY_HUNGER,
	       SPELL_HEROISM,
	       SPELL_BERSERKER,
	       SPELL_HASTE_SELF},
	-- The Space-Time Continuum
	[5] = {SPELL_PHASE_DOOR,
	       SPELL_TELEPORT_SELF,
	       SPELL_SLOW_MONSTER,
	       SPELL_TELEPORT_OTHER,
	       SPELL_TELEPORT_LEVEL,
	       SPELL_WORD_OF_RECALL},
	-- Grimoire of Black Magic
	[6] = {SPELL_POLYMORPH_OTHER,
	       SPELL_SHOCK_WAVE,
	       SPELL_EXPLOSION,
	       SPELL_CLOUD_KILL,
	       SPELL_MASS_SLEEP,
	       SPELL_BEDLAM,
	       SPELL_REND_SOUL,
	       SPELL_WORD_OF_DESTRUCTION,
	       SPELL_CHAOS_STRIKE},
	-- Ethereal Armor
	[7] = {SPELL_RESIST_COLD,
	       SPELL_RESIST_FIRE,
	       SPELL_RESIST_POISON,
	       SPELL_RESISTANCE,
	       SPELL_SHIELD,
	       SPELL_RUNE_OF_PROTECTION},
	-- Energy Transference
	[8] = {SPELL_RECHARGE_ITEM_I,
	       SPELL_ENCHANT_ARMOR,
	       SPELL_ENCHANT_WEAPON,
	       SPELL_RECHARGE_ITEM_II,
	       SPELL_ELEMENTAL_BRAND},
	-- Tome of Destruction
	[9] = {SPELL_FROST_BALL,
	       SPELL_ACID_BALL,
	       SPELL_FIRE_BALL,
	       SPELL_ICE_STORM,
	       SPELL_GENOCIDE,
	       SPELL_METEOR_SWARM,
	       SPELL_MASS_GENOCIDE,
	       SPELL_RIFT,
	       SPELL_MANA_STORM}}

prayer_books = {
	[0] = {PRAYER_DETECT_EVIL,
	       PRAYER_CURE_LIGHT_WOUNDS,
	       PRAYER_BLESS,
	       PRAYER_REMOVE_FEAR,
	       PRAYER_CALL_LIGHT,
	       PRAYER_FIND_TRAPS,
	       PRAYER_DETECT_DOORS_STAIRS,
	       PRAYER_SLOW_POISON},
	[1] = {PRAYER_SCARE_MONSTER,
	       PRAYER_PORTAL,
	       PRAYER_CURE_SERIOUS_WOUNDS,
	       PRAYER_CHANT,
	       PRAYER_SANCTUARY,
	       PRAYER_SATISFY_HUNGER,
	       PRAYER_REMOVE_CURSE,
	       PRAYER_RESIST_HEAT_COLD},
	[2] = {PRAYER_NEUTRALIZE_POISON,
	       PRAYER_ORB_OF_DRAINING,
	       PRAYER_CURE_CRITICAL_WOUNDS,
	       PRAYER_SENSE_INVISIBLE,
	       PRAYER_PROTECTION_FROM_EVIL,
	       PRAYER_EARTHQUAKE,
	       PRAYER_SENSE_SURROUNDINGS,
	       PRAYER_CURE_MORTAL_WOUNDS,
	       PRAYER_TURN_UNDEAD},
	[3] = {PRAYER_PRAYER,
	       PRAYER_DISPEL_UNDEAD,
	       PRAYER_HEAL,
	       PRAYER_DISPEL_EVIL,
	       PRAYER_GLYPH_OF_WARDING,
	       PRAYER_HOLY_WORD},
	[4] = {PRAYER_BLINK,
	       PRAYER_TELEPORT_SELF,
	       PRAYER_TELEPORT_OTHER,
	       PRAYER_TELEPORT_LEVEL,
	       PRAYER_WORD_OF_RECALL,
	       PRAYER_ALTER_REALITY},
	[5] = {PRAYER_DETECT_MONSTERS,
	       PRAYER_DETECTION,
	       PRAYER_PERCEPTION,
	       PRAYER_PROBING,
	       PRAYER_CLAIRVOYANCE},
	[6] = {PRAYER_CURE_SERIOUS_WOUNDS2,
	       PRAYER_CURE_MORTAL_WOUNDS2,
	       PRAYER_HEALING,
	       PRAYER_RESTORATION,
	       PRAYER_REMEMBRANCE},
	[7] = {PRAYER_UNBARRING_WAYS,
	       PRAYER_RECHARGING,
	       PRAYER_DISPEL_CURSE,
	       PRAYER_ENCHANT_WEAPON,
	       PRAYER_ENCHANT_ARMOUR,
	       PRAYER_ELEMENTAL_BRAND},
	[8] = {PRAYER_DISPEL_UNDEAD2,
	       PRAYER_DISPEL_EVIL2,
	       PRAYER_BANISHMENT,
	       PRAYER_WORD_OF_DESTRUCTION,
	       PRAYER_ANNIHILATION}}

function get_spell_index_hook(object, index)
	local book
	local spell

	-- Find the book
	if object.tval == TV_MAGIC_BOOK then
		book = magic_books[object.sval]
	elseif object.tval == TV_PRAYER_BOOK then
		book = prayer_books[object.sval]
	end

	-- Find the spell
	spell = book[index + 1]

	-- No spell at that index
	if not spell then return -1 end

	return spell
end


magic_name = {
	"Magic Missile",
	"Detect Monsters",
	"Phase Door",
	"Light Area",
	"Find Hidden Traps/Doors",
	"Cure Light Wounds",
	"Detect Treasure",
	"Detect Objects",
	"Identify",
	"Detect Invisible",
	"Detect Enchantment",
	"Stinking Cloud",
	"Lightning Bolt",
	"Confuse Monster",
	"Sleep Monster",
	"Wonder",
	"Frost Bolt",
	"Acid Bolt",
	"Fire Bolt",
	"Trap/Door Destruction",
	"Spear of Light",
	"Turn Stone to Mud",
	"Door Creation",
	"Earthquake",
	"Stair Creation",
	"Cure Poison",
	"Satisfy Hunger",
	"Heroism",
	"Berserker",
	"Haste Self",
	"Teleport Self",
	"Slow Monster",
	"Teleport Other",
	"Teleport Level",
	"Word of Recall",
	"Polymorph Other",
	"Shock Wave",
	"Explosion",
	"Cloudkill",
	"Mass Sleep",
	"Bedlam",
	"Rend Soul",
	"Word of Destruction",
	"Chaos Strike",
	"Resist Cold",
	"Resist Fire",
	"Resist Poison",
	"Resistance",
	"Shield",
	"Rune of Protection",
	"Lesser Recharging",
	"Enchant Armor",
	"Enchant Weapon",
	"Greater Recharging",
	"Elemental Brand",
	"Frost Ball",
	"Acid Ball",
	"Fire Ball",
	"Ice Storm",
	"Genocide",
	"Meteor Swarm",
	"Mass Genocide",
	"Rift",
	"Mana Storm"}

prayer_name = {
	"Detect Evil",
	"Cure Light Wounds",
	"Bless",
	"Remove Fear",
	"Call Light",
	"Find Traps",
	"Detect Doors/Stairs",
	"Slow Poison",
	"Scare Monster",
	"Portal",
	"Cure Serious Wounds",
	"Chant",
	"Sanctuary",
	"Satisfy Hunger",
	"Remove Curse",
	"Resist Heat and Cold",
	"Neutralize Poison",
	"Orb of Draining",
	"Cure Critical Wounds",
	"Sense Invisible",
	"Protection from Evil",
	"Earthquake",
	"Sense Surroundings",
	"Cure Mortal Wounds",
	"Turn Undead",
	"Prayer",
	"Dispel Undead",
	"Heal",
	"Dispel Evil",
	"Glyph of Warding",
	"Holy Word",
	"Detect Monsters",
	"Detection",
	"Perception",
	"Probing",
	"Clairvoyance",
	"Cure Serious Wounds",
	"Cure Mortal Wounds",
	"Healing",
	"Restoration",
	"Remembrance",
	"Dispel Undead",
	"Dispel Evil",
	"Banishment",
	"Word of Destruction",
	"Annihilation",
	"Unbarring Ways",
	"Recharging",
	"Dispel Curse",
	"Enchant Weapon",
	"Enchant Armour",
	"Elemental Brand",
	"Blink",
	"Teleport Self",
	"Teleport Other",
	"Teleport Level",
	"Word of Recall",
	"Alter Reality"}


function get_spell_name_hook(tval, index)
	local name

	-- Find the name
	if tval == TV_MAGIC_BOOK then
		name = magic_name[index + 1]
	elseif tval == TV_PRAYER_BOOK then
		name = prayer_name[index + 1]
	end

	-- No spell at that index
	if not name then return "(blank)" end

	return name
end


function get_spell_info_hook(tval, index)
	local info = ""

	local plev = player.lev

	if tval == TV_MAGIC_BOOK then
		if index == SPELL_MAGIC_MISSILE then
			info = format(" dam %dd4", 3 + ((plev - 1) / 5))
		elseif index == SPELL_PHASE_DOOR then
			info = " range 10"
		elseif index == SPELL_CURE_LIGHT_WOUNDS then
			info = " heal 2d8"
		elseif index == SPELL_STINKING_CLOUD then
			info = format(" dam %d", 10 + (plev / 2))
		elseif index == SPELL_LIGHTNING_BOLT then
			info = format(" dam %dd6", (3 + ((plev - 5) / 6)))
		elseif index == SPELL_FROST_BOLT then
			info = format(" dam %dd8", (5 + ((plev - 5) / 4)))
		elseif index == SPELL_ACID_BOLT then
			info = format(" dam %dd8", (6 + ((plev - 5) / 4)))
		elseif index == SPELL_FIRE_BOLT then
			info = format(" dam %dd8", (8 + ((plev - 5) / 4)))
		elseif index == SPELL_SPEAR_OF_LIGHT then
			info = " dam 6d8"
		elseif index == SPELL_HEROISM then
			info = " dur 25+d25"
		elseif index == SPELL_BERSERKER then
			info = " dur 25+d25"
		elseif index == HASTE_SELF then
			info = format(" dur %d+d20", plev)
		elseif index == SPELL_TELEPORT_SELF then
			info = format(" range %d", plev * 5)
		elseif index == SPELL_SHOCK_WAVE then
			info = format(" dam %d", 10 + plev)
		elseif index == SPELL_EXPLOSION then
			info = format(" dam %d", 20 + plev * 2)
		elseif index == SPELL_CLOUD_KILL then
			info = format(" dam %d", 40 + (plev / 2))
		elseif index == SPELL_REND_SOUL then
			info = format(" dam 11d%d", plev)
		elseif index == SPELL_CHAOS_STRIKE then
			info = format(" dam 13d%d", plev)
		elseif index == SPELL_RESIST_COLD then
			info = " dur 20+d20"
		elseif index == SPELL_RESIST_FIRE then
			info = " dur 20+d20"
		elseif index == SPELL_RESIST_POISON then
			info = " dur 20+d20"
		elseif index == SPELL_RESISTANCE then
			info = " dur 20+d20"
		elseif index == SPELL_SHIELD then
			info = " dur 30+d20"
		elseif index == SPELL_FROST_BALL then
			info = format(" dam %d", 30 + plev)
		elseif index == SPELL_ACID_BALL then
			info = format(" dam %d", 40 + plev)
		elseif index == SPELL_FIRE_BALL then
			info = format(" dam %d", 55 + plev)
		elseif index == SPELL_ICE_STORM then
			info = format(" dam %d", 75 + (plev * 3))
		elseif index == SPELL_METEOR_SWARM then
			info = format(" dam %d", 50 + (plev * 3)) -- ToDo
		elseif index == SPELL_RIFT then
			info = format(" dam 40+%dd7", plev)
		elseif index == SPELL_MANA_STORM then
			info = format(" dam %d", 300 + plev * 2)
		end
	elseif tval == TV_PRAYER_BOOK then
		if index == PRAYER_CURE_LIGHT_WOUNDS then
			info = " heal 2d10"
		elseif index == PRAYER_BLESS then
			info = " dur 12+d12"
		elseif index == PRAYER_PORTAL then
			info = format(" range %d", 3 * plev)
		elseif index == PRAYER_CURE_SERIOUS_WOUNDS then
			info = " heal 4d10"
		elseif index == PRAYER_CHANT then
			info = " dur 24+d24"
		elseif index == PRAYER_RESIST_HEAT_COLD then
			info = " dur 10+d10"
		elseif index == PRAYER_ORB_OF_DRAINING then
			local div
			if bAnd(cp_ptr.flags, CF_BLESS_WEAPON) ~= 0 then
				div = 2
			else
				div = 4
			end
			info = format(" %d+3d6", plev + (plev / div))
		elseif index == PRAYER_CURE_CRITICAL_WOUNDS then
			info = " heal 6d10"
		elseif index == PRAYER_SENSE_INVISIBLE then
			info = " dur 24+d24"
		elseif index == PRAYER_PROTECTION_FROM_EVIL then
			info = format(" dur %d+d25", 3 * plev)
		elseif index == PRAYER_CURE_MORTAL_WOUNDS then
			info = " heal 8d10"
		elseif index == PRAYER_PRAYER then
			info = " dur 48+d48"
		elseif index == PRAYER_DISPEL_UNDEAD then
			info = format(" dam d%d", 3 * plev)
		elseif index == PRAYER_HEAL then
			info = " heal 300"
		elseif index == PRAYER_DISPEL_EVIL then
			info = format(" dam d%d", 3 * plev)
		elseif index == PRAYER_HOLY_WORD then
			info = " heal 1000"
		elseif index == PRAYER_CURE_SERIOUS_WOUNDS2 then
			info = " heal 4d10"
		elseif index == PRAYER_CURE_MORTAL_WOUNDS2 then
			info = " heal 8d10"
		elseif index == PRAYER_HEALING then
			info = " heal 2000"
		elseif index == PRAYER_DISPEL_UNDEAD2 then
			info = format(" dam d%d", 4 * plev)
		elseif index == PRAYER_DISPEL_EVIL2 then
			info = format(" dam d%d", 4 * plev)
		elseif index == PRAYER_ANNIHILATION then
			info = " dam 200"
		elseif index == PRAYER_BLINK then
			info = " range 10"
		elseif index == PRAYER_TELEPORT_SELF then
			info = format(" range %d", 8 * plev)
		end
	end

	return info
end


-- Chance of "beam" instead of "bolt"
function beam_chance()
	if bAnd(cp_ptr.flags, CF_BEAM) ~= 0 then
		return player.lev
	else
		return player.lev / 2
	end
end


function spell_wonder()
	local success
	local dir
	local plev = player.lev
	local die = randint(100) + plev / 5
	local beam = beam_chance()

	success, dir = get_aim_dir()
	if not success then return FALSE end

	if (die > 100) then
		msg_print("You feel a surge of power!")
	end
				
	if (die < 8) then
		clone_monster(dir)
	elseif (die < 14) then
		speed_monster(dir)
	elseif (die < 26) then
		heal_monster(dir)
	elseif (die < 31) then
		poly_monster(dir)
	elseif (die < 36) then
		fire_bolt_or_beam(beam - 10, GF_MISSILE, dir,
		                  damroll(3 + ((plev - 1) / 5), 4))
	elseif (die < 41) then
		confuse_monster(dir, plev)
	elseif (die < 46) then
		fire_ball(GF_POIS, dir, 20 + (plev / 2), 3)
	elseif (die < 51) then
		lite_line(dir)
	elseif (die < 56) then
		fire_beam(GF_ELEC, dir, damroll(3 + ((plev - 5) / 6), 6))
	elseif (die < 61) then
		fire_bolt_or_beam(beam-10, GF_COLD, dir,
		                  damroll(5 + ((plev - 5) / 4), 8))
	elseif (die < 66) then
		fire_bolt_or_beam(beam, GF_ACID, dir,
		                  damroll(6 + ((plev - 5) / 4), 8))
	elseif (die < 71) then
		fire_bolt_or_beam(beam, GF_FIRE, dir,
		                  damroll(8 + ((plev - 5) / 4), 8))
	elseif (die < 76) then
		drain_life(dir, 75)
	elseif (die < 81) then
		fire_ball(GF_ELEC, dir, 30 + plev / 2, 2)
	elseif (die < 86) then
		fire_ball(GF_ACID, dir, 40 + plev, 2)
	elseif (die < 91) then
		fire_ball(GF_ICE, dir, 70 + plev, 3)
	elseif (die < 96) then
		fire_ball(GF_FIRE, dir, 80 + plev, 3)
	elseif (die < 101) then
		drain_life(dir, 100 + plev)
	elseif (die < 104) then
		earthquake(player.py, player.px, 12)
	elseif (die < 106) then
		destroy_area(player.py, player.px, 15, TRUE)
	elseif (die < 108) then
		genocide()
	elseif (die < 110) then
		dispel_monsters(120)
	else -- RARE
		dispel_monsters(150)
		slow_monsters()
		sleep_monsters()
		hp_player(300)
	end
end

function cast_spell(index)
	local plev = player.lev

	local success
	local dir

	local beam = beam_chance()

	if index == SPELL_MAGIC_MISSILE then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_bolt_or_beam(beam-10, GF_MISSILE, dir,
		                  damroll(3 + ((plev - 1) / 5), 4))
	elseif index == SPELL_DETECT_MONSTERS then
		detect_monsters_normal()
	elseif index == SPELL_PHASE_DOOR then
		teleport_player(10)
	elseif index == SPELL_LIGHT_AREA then
		lite_area(damroll(2, (plev / 2)), (plev / 10) + 1)
	elseif index == SPELL_TREASURE_DETECTION then
		detect_treasure()
		detect_objects_gold()
	elseif index == SPELL_CURE_LIGHT_WOUNDS then
		hp_player(damroll(2, 8))
		set_cut(player.cut - 15)
	elseif index == SPELL_OBJECT_DETECTION then
		detect_objects_normal()
	elseif index == SPELL_FIND_TRAPS_DOORS then
		detect_traps()
		detect_doors()
		detect_stairs()
	elseif index == SPELL_STINKING_CLOUD then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_POIS, dir, 10 + (plev / 2), 2)
	elseif index == SPELL_CONFUSE_MONSTER then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		confuse_monster(dir, plev)
	elseif index == SPELL_LIGHTNING_BOLT then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_bolt_or_beam(beam-10, GF_ELEC, dir, damroll(3+((plev-5)/6), 6))
	elseif index == SPELL_TRAP_DOOR_DESTRUCTION then
		destroy_doors_touch()
	elseif index == SPELL_SLEEP_MONSTER then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		sleep_monster(dir)
	elseif index == SPELL_CURE_POISON then
		set_poisoned(0)
	elseif index == SPELL_TELEPORT_SELF then
		teleport_player(plev * 5)
	elseif index == SPELL_SPEAR_OF_LIGHT then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		msg_print("A line of blue shimmering light appears.")
		lite_line(dir)
	elseif index == SPELL_FROST_BOLT then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_bolt_or_beam(beam-10, GF_COLD, dir, damroll(5+((plev-5)/4), 8))
	elseif index == SPELL_TURN_STONE_TO_MUD then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		wall_to_mud(dir)
	elseif index == SPELL_SATISFY_HUNGER then
		set_food(PY_FOOD_MAX - 1)
	elseif index == SPELL_RECHARGE_ITEM_I then
		recharge(2 + plev / 5)
	elseif index == SPELL_WONDER then
		spell_wonder()
	elseif index == SPELL_POLYMORPH_OTHER then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		poly_monster(dir)
	elseif index == SPELL_IDENTIFY then
		ident_spell()
	elseif index == SPELL_MASS_SLEEP then
		sleep_monsters()
	elseif index == SPELL_FIRE_BOLT then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_bolt_or_beam(beam, GF_FIRE, dir, damroll(8+((plev-5)/4), 8))
	elseif index == SPELL_SLOW_MONSTER then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		slow_monster(dir)
	elseif index == SPELL_FROST_BALL then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_COLD, dir, 30 + plev, 2)
	elseif index == SPELL_RECHARGE_ITEM_II then
		recharge(50 + plev)
	elseif index == SPELL_TELEPORT_OTHER then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		teleport_monster(dir)
	elseif index == SPELL_BEDLAM then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_OLD_CONF, dir, plev, 4)
	elseif index == SPELL_FIRE_BALL then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_FIRE, dir, 55 + plev, 2)
	elseif index == SPELL_WORD_OF_DESTRUCTION then
		destroy_area(player.py, player.px, 15, TRUE)
	elseif index == SPELL_GENOCIDE then
		genocide()
	elseif index == SPELL_DOOR_CREATION then
		door_creation()
	elseif index == SPELL_STAIR_CREATION then
		stair_creation()
	elseif index == SPELL_TELEPORT_LEVEL then
		teleport_player_level()
	elseif index == SPELL_EARTHQUAKE then
		earthquake(player.py, player.px, 10)
	elseif index == SPELL_WORD_OF_RECALL then
		set_recall()
	elseif index == SPELL_ACID_BOLT then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_bolt_or_beam(beam, GF_ACID, dir, damroll(6+((plev-5)/4), 8))
	elseif index == SPELL_CLOUD_KILL then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_POIS, dir, 40 + (plev / 2), 3)
	elseif index == SPELL_ACID_BALL then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_ACID, dir, 40 + plev, 2)
	elseif index == SPELL_ICE_STORM then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_ICE, dir, 50 + (2 * plev), 3)
	elseif index == SPELL_METEOR_SWARM then
		success, dir = get_aim_dir()
		if not success then return FALSE end
		-- XXX ToDo XXX
		fire_ball(GF_METEOR, dir, 50 + (plev * 3), 3)
	elseif index == SPELL_MANA_STORM then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_MANA, dir, 300 + (plev * 2), 3)
	elseif index == SPELL_DETECT_INVISIBLE then
		detect_monsters_invis()
	elseif index == SPELL_DETECT_ENCHANTMENT then
		detect_objects_magic()
	elseif index == SPELL_SHOCK_WAVE then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_SOUND, dir, 10 + plev, 2)
	elseif index == SPELL_EXPLOSION then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_ball(GF_SHARD, dir, 20 + (plev * 2), 2)
	elseif index == SPELL_MASS_GENOCIDE then
		mass_genocide()
	elseif index == SPELL_RESIST_FIRE then
		set_oppose_fire(player.oppose_fire + randint(20) + 20)
	elseif index == SPELL_RESIST_COLD then
		set_oppose_cold(player.oppose_cold + randint(20) + 20)
	elseif index == SPELL_ELEMENTAL_BRAND then
		-- ToDo: poison brand for rogues
		brand_ammo()
	elseif index == SPELL_RESIST_POISON then
		set_oppose_pois(player.oppose_pois + randint(20) + 20)
	elseif index == SPELL_RESISTANCE then
		local time = randint(20) + 20
		set_oppose_acid(player.oppose_acid + time)
		set_oppose_elec(player.oppose_elec + time)
		set_oppose_fire(player.oppose_fire + time)
		set_oppose_cold(player.oppose_cold + time)
		set_oppose_pois(player.oppose_pois + time)
	elseif index == SPELL_HEROISM then
		hp_player(10)
		set_hero(player.hero + randint(25) + 25)
		set_afraid(0)
	elseif index == SPELL_SHIELD then
		set_shield(player.shield + randint(20) + 30)
	elseif index == SPELL_BERSERKER then
		hp_player(30)
		set_shero(player.shero + randint(25) + 25)
		set_afraid(0)
	elseif index == SPELL_HASTE_SELF then
		if player.fast == 0 then
			set_fast(randint(20) + plev)
		else
			set_fast(player.fast + randint(5))
		end
	elseif index == SPELL_RIFT then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_beam(GF_GRAVITY, dir, 40 + damroll(plev, 7))
	elseif index == SPELL_REND_SOUL then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_bolt_or_beam(beam / 4, GF_NETHER, dir, damroll(11, plev))
	elseif index == SPELL_CHAOS_STRIKE then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fire_bolt_or_beam(beam, GF_CHAOS, dir, damroll(13, plev))
	elseif index == SPELL_RUNE_OF_PROTECTION then
		warding_glyph()
	elseif index == SPELL_ENCHANT_ARMOR then
		enchant_spell(0, 0, rand_int(3) + plev / 20)
	elseif index == SPELL_ENCHANT_WEAPON then
		enchant_spell(rand_int(4) + plev / 20, rand_int(4) + plev/20, 0)
	end

	return TRUE
end


function pray(index)
	local plev = player.lev

	local success
	local dir

	if index == PRAYER_DETECT_EVIL then
		detect_monsters_evil()
	elseif index == PRAYER_CURE_LIGHT_WOUNDS then
		hp_player(damroll(2, 10))
		set_cut(player.cut - 10)
	elseif index == PRAYER_BLESS then
		set_blessed(player.blessed + randint(12) + 12)
	elseif index == PRAYER_REMOVE_FEAR then
		set_afraid(0)
	elseif index == PRAYER_CALL_LIGHT then
		lite_area(damroll(2, (plev / 2)), (plev / 10) + 1)
	elseif index == PRAYER_FIND_TRAPS then
		detect_traps()
	elseif index == PRAYER_DETECT_DOORS_STAIRS then
		detect_doors()
		detect_stairs()
	elseif index == PRAYER_SLOW_POISON then
		set_poisoned(player.poisoned / 2)
	elseif index == PRAYER_SCARE_MONSTER then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		fear_monster(dir, plev)
	elseif index == PRAYER_PORTAL then
		teleport_player(plev * 3)
	elseif index == PRAYER_CURE_SERIOUS_WOUNDS then
		hp_player(damroll(4, 10))
		set_cut((player.cut / 2) - 20)
	elseif index == PRAYER_CHANT then
		set_blessed(player.blessed + randint(24) + 24)
	elseif index == PRAYER_SANCTUARY then
		sleep_monsters_touch()
	elseif index == PRAYER_SATISFY_HUNGER then
		set_food(PY_FOOD_MAX - 1)
	elseif index == PRAYER_REMOVE_CURSE then
		remove_curse()
	elseif index == PRAYER_RESIST_HEAT_COLD then
		set_oppose_fire(player.oppose_fire + randint(10) + 10)
		set_oppose_cold(player.oppose_cold + randint(10) + 10)
	elseif index == PRAYER_NEUTRALIZE_POISON then
		set_poisoned(0)
	elseif index == PRAYER_ORB_OF_DRAINING then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		local div
		if bAnd(cp_ptr.flags, CF_BLESS_WEAPON) ~= 0 then
			div = 2
		else
			div = 4
		end

		local rad
		if plev < 30 then rad = 2 else rad = 3 end

		fire_ball(GF_HOLY_ORB, dir, (damroll(3, 6) + plev + (plev / div)), rad)
	elseif index == PRAYER_CURE_CRITICAL_WOUNDS then
		hp_player(damroll(6, 10))
		set_cut(0)
	elseif index == PRAYER_SENSE_INVISIBLE then
		set_tim_invis(player.tim_invis + randint(24) + 24)
	elseif index == PRAYER_PROTECTION_FROM_EVIL then
		set_protevil(player.protevil + randint(25) + 3 * player.lev)
	elseif index == PRAYER_EARTHQUAKE then
		earthquake(player.py, player.px, 10)
	elseif index == PRAYER_SENSE_SURROUNDINGS then
		map_area()
	elseif index == PRAYER_CURE_MORTAL_WOUNDS then
		hp_player(damroll(8, 10))
		set_stun(0)
		set_cut(0)
	elseif index == PRAYER_TURN_UNDEAD then
		turn_undead()
	elseif index == PRAYER_PRAYER then
		set_blessed(player.blessed + randint(48) + 48)
	elseif index == PRAYER_DISPEL_UNDEAD then
		dispel_undead(randint(plev * 3))
	elseif index == PRAYER_HEAL then
		hp_player(300)
		set_stun(0)
		set_cut(0)
	elseif index == PRAYER_DISPEL_EVIL then
		dispel_evil(randint(plev * 3))
	elseif index == PRAYER_GLYPH_OF_WARDING then
		warding_glyph()
	elseif index == PRAYER_HOLY_WORD then
		dispel_evil(randint(plev * 4))
		hp_player(1000)
		set_afraid(0)
		set_poisoned(0)
		set_stun(0)
		set_cut(0)
	elseif index == PRAYER_DETECT_MONSTERS then
		detect_monsters_normal()
	elseif index == PRAYER_DETECTION then
		detect_all()
	elseif index == PRAYER_PERCEPTION then
		ident_spell()
	elseif index == PRAYER_PROBING then
		probing()
	elseif index == PRAYER_CLAIRVOYANCE then
		wiz_lite()
	elseif index == PRAYER_CURE_SERIOUS_WOUNDS2 then
		hp_player(damroll(4, 10))
		set_cut(0)
	elseif index == PRAYER_CURE_MORTAL_WOUNDS2 then
		hp_player(damroll(8, 10))
		set_stun(0)
		set_cut(0)
	elseif index == PRAYER_HEALING then
		hp_player(2000)
		set_stun(0)
		set_cut(0)
	elseif index == PRAYER_RESTORATION then
		do_res_stat(A_STR)
		do_res_stat(A_INT)
		do_res_stat(A_WIS)
		do_res_stat(A_DEX)
		do_res_stat(A_CON)
		do_res_stat(A_CHR)
	elseif index == PRAYER_REMEMBRANCE then
		restore_level()
	elseif index == PRAYER_DISPEL_UNDEAD2 then
		dispel_undead(randint(plev * 4))
	elseif index == PRAYER_DISPEL_EVIL2 then
		dispel_evil(randint(plev * 4))
	elseif index == PRAYER_BANISHMENT then
		if banish_evil(100) then
			msg_print("The power of your god banishes evil!")
		end
	elseif index == PRAYER_WORD_OF_DESTRUCTION then
		destroy_area(player.py, player.px, 15, TRUE)
	elseif index == PRAYER_ANNIHILATION then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		drain_life(dir, 200)
	elseif index == PRAYER_UNBARRING_WAYS then
		destroy_doors_touch()
	elseif index == PRAYER_RECHARGING then
		recharge(15)
	elseif index == PRAYER_DISPEL_CURSE then
		remove_all_curse()
	elseif index == PRAYER_ENCHANT_WEAPON then
		enchant_spell(rand_int(4) + 1, rand_int(4) + 1, 0)
	elseif index == PRAYER_ENCHANT_ARMOUR then
		enchant_spell(0, 0, rand_int(3) + 2)
	elseif index == PRAYER_ELEMENTAL_BRAND then
		brand_weapon()
	elseif index == PRAYER_BLINK then
		teleport_player(10)
	elseif index == PRAYER_TELEPORT_SELF then
		teleport_player(plev * 8)
	elseif index == PRAYER_TELEPORT_OTHER then
		success, dir = get_aim_dir()
		if not success then return FALSE end

		teleport_monster(dir)
	elseif index == PRAYER_TELEPORT_LEVEL then
		teleport_player_level()
	elseif index == PRAYER_WORD_OF_RECALL then
		set_recall()
	elseif index == PRAYER_ALTER_REALITY then
		msg_print("The world changes!")
		player.leaving = TRUE
	end

	return TRUE
end


function cast_spell_hook(tval, index)
	if tval == TV_MAGIC_BOOK then
		return cast_spell(index)
	elseif tval == TV_PRAYER_BOOK then
		return pray(index)
	else
		error(format("unknown tval (%d) in function cast_spell_hook()", tval))
	end
end
