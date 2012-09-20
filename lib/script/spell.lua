-- Spell-casting code


-----------------------------------------------------------
-- Available "schools" of magic
magic_schools = {}


-----------------------------------------------------------
-- Helper functions for spell-casting

-- Create a new "school" of magic
function add_school(tval)
	local school = {}
	school.spells = {}
	school.books = {}
	magic_schools[tval] = school
	return school
end


-- Adds a spell to a "school" of magic and
-- returns the zero-based index of the spell
function add_spell(school, spell)
	table.insert(school.spells, spell)
	return table.getn(school.spells) - 1
end


-- Add a new book to a "school" of magic
-- sval is the sval of the book to be added
-- spells is a table of spell-indices for that book
function add_book(school, sval, spells)
	school.books[sval] = spells
end


-- Get the index of the spell on a specific page of a spellbook
-- return -1 when the page doesn't exist
function get_spell_index(book, position)
	local spell = book[position + 1]

	-- No spell on that page
	if not spell then return -1 end

	return spell
end


-- Cast a spell
-- return false if the player canceled the casting
function cast_spell(school, index)
	return school.spells[index + 1].effect()
end


-- Get extra info about a spell (damage, duration, ...)
function get_spell_info(school, index)
	local info = school.spells[index + 1].info
	if not info then return "" end

	return info()
end


-- Get the name of a spell
function get_spell_name(school, index)
	return school.spells[index + 1].name
end


-----------------------------------------------------------
-- Magic spells

magic_spells = add_school(TV_MAGIC_BOOK)

function add_magic_spell(spell)
	return add_spell(magic_spells, spell)
end


-- Helper-function: Chance of "beam" instead of "bolt"
function beam_chance()
	if bitlib.bAnd(cp_ptr.flags, CF_BEAM) ~= 0 then
		return player.lev
	else
		return player.lev / 2
	end
end


SPELL_MAGIC_MISSILE = add_magic_spell
{
	name = "Magic Missile",
	info = function()
			return string.format(" dam %dd4", 3 + ((player.lev - 1) / 5))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_bolt_or_beam(beam_chance()-10, GF_MISSILE, dir,
		    	              damroll(3 + ((player.lev - 1) / 5), 4))
			return true
		end,
}

SPELL_DETECT_MONSTERS = add_magic_spell
{
	name = "Detect Monsters",
	effect = function()
			detect_monsters_normal()
			return true
		end,
}

SPELL_PHASE_DOOR = add_magic_spell
{
	name = "Phase Door",
	info = function()
			return " range 10"
		end,
	effect = function()
			teleport_player(10)
			return true
		end,
}


SPELL_LIGHT_AREA = add_magic_spell
{
	name = "Light Area",
	effect = function()
			lite_area(damroll(2, (player.lev / 2)), (player.lev / 10) + 1)
			return true
		end,
}

SPELL_FIND_TRAPS_DOORS = add_magic_spell
{
	name = "Find Hidden Traps/Doors",
	effect = function()
			detect_traps()
			detect_doors()
			detect_stairs()
			return true
		end,
}

SPELL_CURE_LIGHT_WOUNDS = add_magic_spell
{
	name = "Cure Light Wounds",
	info = function()
			return " heal 2d8"
		end,
	effect = function()
			hp_player(damroll(2, 8))
			set_cut(player.cut - 15)
			return true
		end,
}

SPELL_TREASURE_DETECTION = add_magic_spell
{
	name = "Detect Treasure",
	effect = function()
			detect_treasure()
			detect_objects_gold()
			return true
		end,
}

SPELL_OBJECT_DETECTION = add_magic_spell
{
	name = "Detect Objects",
	effect = function()
			detect_objects_normal()
			return true
		end,
}

SPELL_IDENTIFY = add_magic_spell
{
	name = "Identify",
	effect = function()
			ident_spell()
			return true
		end,
}

SPELL_DETECT_INVISIBLE = add_magic_spell
{
	name = "Detect Invisible",
	effect = function()
			detect_monsters_invis()
			return true
		end,
}

SPELL_DETECT_ENCHANTMENT = add_magic_spell
{
	name = "Detect Enchantment",
	effect = function()
			detect_objects_magic()
			return true
		end,
}

SPELL_STINKING_CLOUD = add_magic_spell
{
	name = "Stinking Cloud",
	info = function()
			return string.format(" dam %d", 10 + (player.lev / 2))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_POIS, dir, 10 + (player.lev / 2), 2)
			return true
		end,
}

SPELL_LIGHTNING_BOLT = add_magic_spell
{
	name = "Lightning Bolt",
	info = function()
			return string.format(" dam %dd6", (3 + ((player.lev - 5) / 6)))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_beam(GF_ELEC, dir, damroll(3 + ((player.lev - 5) / 6), 6))
			return true
		end,
}

SPELL_CONFUSE_MONSTER = add_magic_spell
{
	name = "Confuse Monster",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			confuse_monster(dir, player.lev)
			return true
		end,
}

SPELL_SLEEP_MONSTER = add_magic_spell
{
	name = "Sleep Monster",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			sleep_monster(dir)
			return true
		end,
}

SPELL_WONDER = add_magic_spell
{
	name = "Wonder",
	effect = function()
			local plev = player.lev
			local die = randint(100) + plev / 5
			local beam = beam_chance()

			local success, dir = get_aim_dir()
			if not success then return false end

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
				destroy_area(player.py, player.px, 15, true)
			elseif (die < 108) then
				banishment()
			elseif (die < 110) then
				dispel_monsters(120)
			else -- RARE
				dispel_monsters(150)
				slow_monsters()
				sleep_monsters()
				hp_player(300)
			end

			return true
		end,
}

SPELL_FROST_BOLT = add_magic_spell
{
	name = "Frost Bolt",
	info = function()
			return string.format(" dam %dd8", (5 + ((player.lev - 5) / 4)))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_bolt_or_beam(beam_chance() - 10, GF_COLD, dir,
			                  damroll(5 + ((player.lev - 5) / 4), 8))
			return true
		end,
}

SPELL_ACID_BOLT = add_magic_spell
{
	name = "Acid Bolt",
	info = function()
			return string.format(" dam %dd8", (8 + ((player.lev - 5) / 4)))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_bolt_or_beam(beam_chance(), GF_ACID, dir,
			                  damroll(8 + ((player.lev - 5) / 4), 8))
			return true
		end,
}

SPELL_FIRE_BOLT = add_magic_spell
{
	name = "Fire Bolt",
	info = function()
			return string.format(" dam %dd8", (6 + ((player.lev - 5) / 4)))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_bolt_or_beam(beam_chance(), GF_FIRE, dir,
			                  damroll(6 + ((player.lev - 5) / 4), 8))
			return true
		end,
}

SPELL_TRAP_DOOR_DESTRUCTION = add_magic_spell
{
	name = "Trap/Door Destruction",
	effect = function()
			destroy_doors_touch()
			return true
		end,
}

SPELL_SPEAR_OF_LIGHT = add_magic_spell
{
	name = "Spear of Light",
	info = function()
			return " dam 6d8"
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			msg_print("A line of blue shimmering light appears.")
			lite_line(dir)
			return true
		end,
}

SPELL_TURN_STONE_TO_MUD = add_magic_spell
{
	name = "Turn Stone to Mud",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			wall_to_mud(dir)
			return true
		end,
}

SPELL_DOOR_CREATION = add_magic_spell
{
	name = "Door Creation",
	effect = function()
			door_creation()
			return true
		end,
}

SPELL_EARTHQUAKE = add_magic_spell
{
	name = "Earthquake",
	effect = function()
			earthquake(player.py, player.px, 10)
			return true
		end,
}

SPELL_STAIR_CREATION = add_magic_spell
{
	name = "Stair Creation",
	effect = function()
			stair_creation()
			return true
		end,
}

SPELL_CURE_POISON = add_magic_spell
{
	name = "Cure Poison",
	effect = function()
			set_poisoned(0)
			return true
		end,
}

SPELL_SATISFY_HUNGER = add_magic_spell
{
	name = "Satisfy Hunger",
	effect = function()
			set_food(PY_FOOD_MAX - 1)
			return true
		end,
}

SPELL_HEROISM = add_magic_spell
{
	name = "Heroism",
	info = function()
			return " dur 25+d25"
		end,
	effect = function()
			hp_player(10)
			set_hero(player.hero + randint(25) + 25)
			set_afraid(0)
			return true
		end,
}

SPELL_BERSERKER = add_magic_spell
{
	name = "Berserker",
	info = function()
			return " dur 25+d25"
		end,
	effect = function()
			hp_player(30)
			set_shero(player.shero + randint(25) + 25)
			set_afraid(0)
			return true
		end,
}

SPELL_HASTE_SELF = add_magic_spell
{
	name = "Haste Self",
	info = function()
			return string.format(" dur %d+d20", player.lev)
		end,
	effect = function()
			if player.fast == 0 then
				set_fast(randint(20) + player.lev)
			else
				set_fast(player.fast + randint(5))
			end
			return true
		end,
}

SPELL_TELEPORT_SELF = add_magic_spell
{
	name = "Teleport Self",
	info = function()
			return string.format(" range %d", player.lev * 5)
		end,
	effect = function()
			teleport_player(player.lev * 5)
			return true
		end,
}

SPELL_SLOW_MONSTER = add_magic_spell
{
	name = "Slow Monster",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			slow_monster(dir)
			return true
		end,
}

SPELL_TELEPORT_OTHER = add_magic_spell
{
	name = "Teleport Other",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			teleport_monster(dir)
			return true
		end,
}

SPELL_TELEPORT_LEVEL = add_magic_spell
{
	name = "Teleport Level",
	effect = function()
			teleport_player_level()
			return true
		end,
}

SPELL_WORD_OF_RECALL = add_magic_spell
{
	name = "Word of Recall",
	effect = function()
			set_recall()
			return true
		end,
}

SPELL_POLYMORPH_OTHER = add_magic_spell
{
	name = "Polymorph Other",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			poly_monster(dir)
			return true
		end,
}

SPELL_SHOCK_WAVE = add_magic_spell
{
	name = "Shock Wave",
	info = function()
			return string.format(" dam %d", 10 + player.lev)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_SOUND, dir, 10 + player.lev, 2)
			return true
		end,
}

SPELL_EXPLOSION = add_magic_spell
{
	name = "Explosion",
	info = function()
			return string.format(" dam %d", 20 + player.lev * 2)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_SHARD, dir, 20 + (player.lev * 2), 2)
			return true
		end,
}

SPELL_CLOUD_KILL = add_magic_spell
{
	name = "Cloudkill",
	info = function()
			return string.format(" dam %d", 40 + (player.lev / 2))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_POIS, dir, 40 + (player.lev / 2), 3)
			return true
		end,
}

SPELL_MASS_SLEEP = add_magic_spell
{
	name = "Mass Sleep",
	effect = function()
			sleep_monsters()
			return true
		end,
}

SPELL_BEDLAM = add_magic_spell
{
	name = "Bedlam",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_OLD_CONF, dir, player.lev, 4)
			return true
		end,
}

SPELL_REND_SOUL = add_magic_spell
{
	name = "Rend Soul",
	info = function()
			return string.format(" dam 11d%d", player.lev)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_bolt_or_beam(beam_chance() / 4, GF_NETHER, dir,
			                  damroll(11, player.lev))
			return true
		end,
}

SPELL_WORD_OF_DESTRUCTION = add_magic_spell
{
	name = "Word of Destruction",
	effect = function()
			destroy_area(player.py, player.px, 15, true)
			return true
		end,
}

SPELL_CHAOS_STRIKE = add_magic_spell
{
	name = "Chaos Strike",
	info = function()
			return string.format(" dam 13d%d", player.lev)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_bolt_or_beam(beam_chance(), GF_CHAOS, dir,
			                  damroll(13, player.lev))
			return true
		end,
}

SPELL_RESIST_COLD = add_magic_spell
{
	name = "Resist Cold",
	info = function()
			return " dur 20+d20"
		end,
	effect = function()
			set_oppose_cold(player.oppose_cold + randint(20) + 20)
			return true
		end,
}

SPELL_RESIST_FIRE = add_magic_spell
{
	name = "Resist Fire",
	info = function()
			return " dur 20+d20"
		end,
	effect = function()
			set_oppose_fire(player.oppose_fire + randint(20) + 20)
			return true
		end,
}

SPELL_RESIST_POISON = add_magic_spell
{
	name = "Resist Poison",
	info = function()
			return " dur 20+d20"
		end,
	effect = function()
			set_oppose_pois(player.oppose_pois + randint(20) + 20)
			return true
		end,
}

SPELL_RESISTANCE = add_magic_spell
{
	name = "Resistance",
	info = function()
			return " dur 20+d20"
		end,
	effect = function()
			local time = randint(20) + 20
			set_oppose_acid(player.oppose_acid + time)
			set_oppose_elec(player.oppose_elec + time)
			set_oppose_fire(player.oppose_fire + time)
			set_oppose_cold(player.oppose_cold + time)
			set_oppose_pois(player.oppose_pois + time)
			return true
		end,
}

SPELL_SHIELD = add_magic_spell
{
	name = "Shield",
	info = function()
			return " dur 30+d20"
		end,
	effect = function()
			set_shield(player.shield + randint(20) + 30)
			return true
		end,
}

SPELL_RUNE_OF_PROTECTION = add_magic_spell
{
	name = "Rune of Protection",
	effect = function()
			warding_glyph()
			return true
		end,
}

SPELL_RECHARGE_ITEM_I = add_magic_spell
{
	name = "Lesser Recharging",
	effect = function()
			recharge(2 + player.lev / 5)
			return true
		end,
}

SPELL_ENCHANT_ARMOR = add_magic_spell
{
	name = "Enchant Armor",
	effect = function()
			enchant_spell(0, 0, rand_int(3) + player.lev / 20)
			return true
		end,
}

SPELL_ENCHANT_WEAPON = add_magic_spell
{
	name = "Enchant Weapon",
	effect = function()
			enchant_spell(rand_int(4) + player.lev / 20,
			              rand_int(4) + player.lev / 20, 0)
			return true
		end,
}

-- ToDo - replace one of the recharge spells
SPELL_RECHARGE_ITEM_II = add_magic_spell
{
	name = "Greater Recharging",
	effect = function()
			recharge(50 + player.lev)
			return true
		end,
}

SPELL_ELEMENTAL_BRAND = add_magic_spell
{
	name = "Elemental Brand",
	effect = function()
			-- ToDo: poison brand for rogues
			brand_ammo()
			return true
		end,
}

SPELL_FROST_BALL = add_magic_spell
{
	name = "Frost Ball",
	info = function()
			return string.format(" dam %d", 30 + player.lev)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_COLD, dir, 30 + player.lev, 2)
			return true
		end,
}

SPELL_ACID_BALL = add_magic_spell
{
	name = "Acid Ball",
	info = function()
			return string.format(" dam %d", 40 + player.lev)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_ACID, dir, 40 + player.lev, 2)
			return true
		end,
}

SPELL_FIRE_BALL = add_magic_spell
{
	name = "Fire Ball",
	info = function()
			return string.format(" dam %d", 55 + player.lev)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_FIRE, dir, 55 + player.lev, 2)
			return true
		end,
}

SPELL_ICE_STORM = add_magic_spell
{
	name = "Ice Storm",
	info = function()
			return string.format(" dam %d", 50 + (2 * player.lev))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_ICE, dir, 50 + (2 * player.lev), 3)
			return true
		end,
}

SPELL_BANISHMENT = add_magic_spell
{
	name = "Banishment",
	effect = function()
			banishment()
			return true
		end,
}

SPELL_METEOR_SWARM = add_magic_spell
{
	name = "Meteor Swarm",
	info = function()
			return string.format(" dam %dx%d", 30 + player.lev / 2,
			              2 + player.lev / 20)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end
			fire_swarm(2 + player.lev / 20, GF_METEOR, dir,
			           30 + player.lev / 2, 1);
			return true
		end,
}

SPELL_MASS_BANISHMENT = add_magic_spell
{
	name = "Mass Banishment",
	effect = function()
			mass_banishment()
			return true
		end,
}

SPELL_RIFT = add_magic_spell
{
	name = "Rift",
	info = function()
			return string.format(" dam 40+%dd7", player.lev)
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_beam(GF_GRAVITY, dir, 40 + damroll(player.lev, 7))
			return true
		end,
}

SPELL_MANA_STORM = add_magic_spell
{
	name = "Mana Storm",
	info = function()
			return string.format(" dam %d", 300 + (player.lev * 2))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fire_ball(GF_MANA, dir, 300 + (player.lev * 2), 3)
			return true
		end,
}


-----------------------------------------------------------
-- Add spells to the spell-books

-- Magic for Beginners
add_book(magic_spells, 0,
         {SPELL_MAGIC_MISSILE,
          SPELL_DETECT_MONSTERS,
          SPELL_PHASE_DOOR,
          SPELL_LIGHT_AREA,
          SPELL_TREASURE_DETECTION,
          SPELL_CURE_LIGHT_WOUNDS,
          SPELL_OBJECT_DETECTION,
          SPELL_FIND_TRAPS_DOORS,
          SPELL_STINKING_CLOUD})

-- Conjurings and Tricks
add_book(magic_spells, 1,
         {SPELL_CONFUSE_MONSTER,
          SPELL_LIGHTNING_BOLT,
          SPELL_TRAP_DOOR_DESTRUCTION,
          SPELL_CURE_POISON,
          SPELL_SLEEP_MONSTER,
          SPELL_TELEPORT_SELF,
          SPELL_SPEAR_OF_LIGHT,
          SPELL_FROST_BOLT,
          SPELL_WONDER})

-- Incantations and Illusions
add_book(magic_spells, 2,
         {SPELL_SATISFY_HUNGER,
          SPELL_RECHARGE_ITEM_I,
          SPELL_TURN_STONE_TO_MUD,
          SPELL_FIRE_BOLT,
          SPELL_POLYMORPH_OTHER,
          SPELL_IDENTIFY,
          SPELL_DETECT_INVISIBLE,
          SPELL_ACID_BOLT,
          SPELL_SLOW_MONSTER})

-- Sorcery and Evocations
add_book(magic_spells, 3,
         {SPELL_FROST_BALL,
          SPELL_TELEPORT_OTHER,
          SPELL_HASTE_SELF,
          SPELL_MASS_SLEEP,
          SPELL_FIRE_BALL,
          SPELL_DETECT_ENCHANTMENT})

-- Resistances of Scarabtarices
add_book(magic_spells, 4,
         {SPELL_RESIST_COLD,
          SPELL_RESIST_FIRE,
          SPELL_RESIST_POISON,
          SPELL_RESISTANCE,
          SPELL_SHIELD})

-- Raal's Tome of Destruction
add_book(magic_spells, 5,
         {SPELL_SHOCK_WAVE,
          SPELL_EXPLOSION,
          SPELL_CLOUD_KILL,
          SPELL_ACID_BALL,
          SPELL_ICE_STORM,
          SPELL_METEOR_SWARM,
          SPELL_RIFT})

-- Mordenkainen's Escapes
add_book(magic_spells, 6,
         {SPELL_DOOR_CREATION,
          SPELL_STAIR_CREATION,
          SPELL_TELEPORT_LEVEL,
          SPELL_WORD_OF_RECALL,
          SPELL_RUNE_OF_PROTECTION})

-- Tenser's transformations
add_book(magic_spells, 7,
         {SPELL_HEROISM,
          SPELL_BERSERKER,
          SPELL_ENCHANT_ARMOR,
          SPELL_ENCHANT_WEAPON,
          SPELL_RECHARGE_ITEM_II,
          SPELL_ELEMENTAL_BRAND})

-- Kelek's Grimoire of Power
add_book(magic_spells, 8,
         {SPELL_EARTHQUAKE,
          SPELL_BEDLAM,
          SPELL_REND_SOUL,
          SPELL_BANISHMENT,
          SPELL_WORD_OF_DESTRUCTION,
          SPELL_MASS_BANISHMENT,
          SPELL_CHAOS_STRIKE,
          SPELL_MANA_STORM})


-----------------------------------------------------------
-- Prayers

prayers = add_school(TV_PRAYER_BOOK)

function add_prayer(prayer)
	return add_spell(prayers, prayer)
end


PRAYER_DETECT_EVIL = add_prayer
{
	name = "Detect Evil",
	effect = function()
			detect_monsters_evil()
			return true
		end,
}

PRAYER_CURE_LIGHT_WOUNDS = add_prayer
{
	name = "Cure Light Wounds",
	info = function()
			return " heal 2d10"
		end,
	effect = function()
			hp_player(damroll(2, 10))
			set_cut(player.cut - 10)
			return true
		end,
}

PRAYER_BLESS = add_prayer
{
	name = "Bless",
	info = function()
			return " dur 12+d12"
		end,
	effect = function()
			set_blessed(player.blessed + randint(12) + 12)
			return true
		end,
}

PRAYER_REMOVE_FEAR = add_prayer
{
	name = "Remove Fear",
	effect = function()
			set_afraid(0)
			return true
		end,
}

PRAYER_CALL_LIGHT = add_prayer
{
	name = "Call Light",
	effect = function()
			lite_area(damroll(2, (player.lev / 2)), (player.lev / 10) + 1)
			return true
		end,
}

PRAYER_FIND_TRAPS = add_prayer
{
	name = "Find Traps",
	effect = function()
			detect_traps()
			return true
		end,
}

PRAYER_DETECT_DOORS_STAIRS = add_prayer
{
	name = "Detect Doors/Stairs",
	effect = function()
			detect_doors()
			detect_stairs()
			return true
		end,
}

PRAYER_SLOW_POISON = add_prayer
{
	name = "Slow Poison",
	effect = function()
			set_poisoned(player.poisoned / 2)
			return true
		end,
}

PRAYER_SCARE_MONSTER = add_prayer
{
	name = "Scare Monster",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			fear_monster(dir, player.lev)
			return true
		end,
}

PRAYER_PORTAL = add_prayer
{
	name = "Portal",
	info = function()
			return string.format(" range %d", 3 * player.lev)
		end,
	effect = function()
			teleport_player(3 * player.lev)
			return true
		end,
}

PRAYER_CURE_SERIOUS_WOUNDS = add_prayer
{
	name = "Cure Serious Wounds",
	info = function()
			return " heal 4d10"
		end,
	effect = function()
			hp_player(damroll(4, 10))
			set_cut((player.cut / 2) - 20)
			return true
		end,
}

PRAYER_CHANT = add_prayer
{
	name = "Chant",
	info = function()
			return " dur 24+d24"
		end,
	effect = function()
			set_blessed(player.blessed + randint(24) + 24)
			return true
		end,
}

PRAYER_SANCTUARY = add_prayer
{
	name = "Sanctuary",
	effect = function()
			sleep_monsters_touch()
			return true
		end,
}

PRAYER_SATISFY_HUNGER = add_prayer
{
	name = "Satisfy Hunger",
	effect = function()
			set_food(PY_FOOD_MAX - 1)
			return true
		end,
}

PRAYER_REMOVE_CURSE = add_prayer
{
	name = "Remove Curse",
	effect = function()
			remove_curse()
			return true
		end,
}

PRAYER_RESIST_HEAT_COLD = add_prayer
{
	name = "Resist Heat and Cold",
	info = function()
			return " dur 10+d10"
		end,
	effect = function()
			set_oppose_fire(player.oppose_fire + randint(10) + 10)
			set_oppose_cold(player.oppose_cold + randint(10) + 10)
			return true
		end,
}

PRAYER_NEUTRALIZE_POISON = add_prayer
{
	name = "Neutralize Poison",
	effect = function()
			set_poisoned(0)
			return true
		end,
}

PRAYER_ORB_OF_DRAINING = add_prayer
{
	name = "Orb of Draining",
	info = function()
			local div
			if bitlib.bAnd(cp_ptr.flags, CF_BLESS_WEAPON) ~= 0 then
				div = 2
			else
				div = 4
			end
			return string.format(" %d+3d6", player.lev + (player.lev / div))
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			local div
			if bitlib.bAnd(cp_ptr.flags, CF_BLESS_WEAPON) ~= 0 then
				div = 2
			else
				div = 4
			end

			local rad
			if player.lev < 30 then rad = 2 else rad = 3 end

			fire_ball(GF_HOLY_ORB, dir,
			          (damroll(3, 6) + player.lev + (player.lev / div)), rad)

			return true
		end,
}

PRAYER_CURE_CRITICAL_WOUNDS = add_prayer
{
	name = "Cure Critical Wounds",
	info = function()
			return " heal 6d10"
		end,
	effect = function()
			hp_player(damroll(6, 10))
			set_cut(0)
			return true
		end,
}

PRAYER_SENSE_INVISIBLE = add_prayer
{
	name = "Sense Invisible",
	info = function()
			return " dur 24+d24"
		end,
	effect = function()
			set_tim_invis(player.tim_invis + randint(24) + 24)
			return true
		end,
}

PRAYER_PROTECTION_FROM_EVIL = add_prayer
{
	name = "Protection from Evil",
	info = function()
			return string.format(" dur %d+d25", 3 * player.lev)
		end,
	effect = function()
			set_protevil(player.protevil + randint(25) + 3 * player.lev)
			return true
		end,
}

PRAYER_EARTHQUAKE = add_prayer
{
	name = "Earthquake",
	effect = function()
			earthquake(player.py, player.px, 10)
			return true
		end,
}

PRAYER_SENSE_SURROUNDINGS = add_prayer
{
	name = "Sense Surroundings",
	effect = function()
			map_area()
			return true
		end,
}

PRAYER_CURE_MORTAL_WOUNDS = add_prayer
{
	name = "Cure Mortal Wounds",
	info = function()
			return " heal 8d10"
		end,
	effect = function()
			hp_player(damroll(8, 10))
			set_stun(0)
			set_cut(0)
			return true
		end,
}

PRAYER_TURN_UNDEAD = add_prayer
{
	name = "Turn Undead",
	effect = function()
			turn_undead()
			return true
		end,
}

PRAYER_PRAYER = add_prayer
{
	name = "Prayer",
	info = function()
			return " dur 48+d48"
		end,
	effect = function()
			set_blessed(player.blessed + randint(48) + 48)
			return true
		end,
}

PRAYER_DISPEL_UNDEAD = add_prayer
{
	name = "Dispel Undead",
	info = function()
			return string.format(" dam d%d", 3 * player.lev)
		end,
	effect = function()
			dispel_undead(randint(3 * player.lev))
			return true
		end,
}

PRAYER_HEAL = add_prayer
{
	name = "Heal",
	info = function()
			return " heal 300"
		end,
	effect = function()
			hp_player(300)
			set_stun(0)
			set_cut(0)
			return true
		end,
}

PRAYER_DISPEL_EVIL = add_prayer
{
	name = "Dispel Evil",
	info = function()
			return string.format(" dam d%d", 3 * player.lev)
		end,
	effect = function()
			dispel_evil(randint(3 * player.lev))
			return true
		end,
}

PRAYER_GLYPH_OF_WARDING = add_prayer
{
	name = "Glyph of Warding",
	effect = function()
			warding_glyph()
			return true
		end,
}

PRAYER_HOLY_WORD = add_prayer
{
	name = "Holy Word",
	info = function()
			return " heal 1000"
		end,
	effect = function()
			dispel_evil(randint(player.lev * 4))
			hp_player(1000)
			set_afraid(0)
			set_poisoned(0)
			set_stun(0)
			set_cut(0)
			return true
		end,
}

PRAYER_DETECT_MONSTERS = add_prayer
{
	name = "Detect Monsters",
	effect = function()
			detect_monsters_normal()
			return true
		end,
}

PRAYER_DETECTION = add_prayer
{
	name = "Detection",
	effect = function()
			detect_all()
			return true
		end,
}

PRAYER_PERCEPTION = add_prayer
{
	name = "Perception",
	effect = function()
			ident_spell()
			return true
		end,
}

PRAYER_PROBING = add_prayer
{
	name = "Probing",
	effect = function()
			probing()
			return true
		end,
}

PRAYER_CLAIRVOYANCE = add_prayer
{
	name = "Clairvoyance",
	effect = function()
			wiz_lite()
			return true
		end,
}

PRAYER_CURE_SERIOUS_WOUNDS2 = add_prayer
{
	name = "Cure Serious Wounds",
	info = function()
			return " heal 4d10"
		end,
	effect = function()
			hp_player(damroll(4, 10))
			set_cut(0)
			return true
		end,
}

PRAYER_CURE_MORTAL_WOUNDS2 = add_prayer
{
	name = "Cure Mortal Wounds",
	info = function()
			return " heal 8d10"
		end,
	effect = function()
			hp_player(damroll(8, 10))
			set_stun(0)
			set_cut(0)
			return true
		end,
}

PRAYER_HEALING = add_prayer
{
	name = "Healing",
	info = function()
			return " heal 2000"
		end,
	effect = function()
			hp_player(2000)
			set_stun(0)
			set_cut(0)
			return true
		end,
}

PRAYER_RESTORATION = add_prayer
{
	name = "Restoration",
	effect = function()
			do_res_stat(A_STR)
			do_res_stat(A_INT)
			do_res_stat(A_WIS)
			do_res_stat(A_DEX)
			do_res_stat(A_CON)
			do_res_stat(A_CHR)
			return true
		end,
}

PRAYER_REMEMBRANCE = add_prayer
{
	name = "Remembrance",
	effect = function()
			restore_level()
			return true
		end,
}

PRAYER_DISPEL_UNDEAD2 = add_prayer
{
	name = "Dispel Undead",
	info = function()
			return string.format(" dam d%d", 4 * player.lev)
		end,
	effect = function()
			dispel_undead(randint(4 * player.lev))
			return true
		end,
}

PRAYER_DISPEL_EVIL2 = add_prayer
{
	name = "Dispel Evil",
	info = function()
			return string.format(" dam d%d", 4 * player.lev)
		end,
	effect = function()
			dispel_evil(randint(4 * player.lev))
			return true
		end,
}

PRAYER_BANISH_EVIL = add_prayer
{
	name = "Banish Evil",
	effect = function()
			if banish_evil(100) then
				msg_print("The power of your god banishes evil!")
			end
			return true
		end,
}

PRAYER_WORD_OF_DESTRUCTION = add_prayer
{
	name = "Word of Destruction",
	effect = function()
			destroy_area(player.py, player.px, 15, true)
			return true
		end,
}

PRAYER_ANNIHILATION = add_prayer
{
	name = "Annihilation",
	info = function()
			return " dam 200"
		end,
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			drain_life(dir, 200)
			return true
		end,
}

PRAYER_UNBARRING_WAYS = add_prayer
{
	name = "Unbarring Ways",
	effect = function()
			destroy_doors_touch()
			return true
		end,
}

PRAYER_RECHARGING = add_prayer
{
	name = "Recharging",
	effect = function()
			recharge(15)
			return true
		end,
}

PRAYER_DISPEL_CURSE = add_prayer
{
	name = "Dispel Curse",
	effect = function()
			remove_all_curse()
			return true
		end,
}

PRAYER_ENCHANT_WEAPON = add_prayer
{
	name = "Enchant Weapon",
	effect = function()
			enchant_spell(rand_int(4) + 1, rand_int(4) + 1, 0)
			return true
		end,
}

PRAYER_ENCHANT_ARMOUR = add_prayer
{
	name = "Enchant Armour",
	effect = function()
			enchant_spell(0, 0, rand_int(3) + 2)
			return true
		end,
}

PRAYER_ELEMENTAL_BRAND = add_prayer
{
	name = "Elemental Brand",
	effect = function()
			brand_weapon()
			return true
		end,
}

PRAYER_BLINK = add_prayer
{
	name = "Blink",
	info = function()
			return " range 10"
		end,
	effect = function()
			teleport_player(10)
			return true
		end,
}

PRAYER_TELEPORT_SELF = add_prayer
{
	name = "Teleport Self",
	info = function()
			return string.format(" range %d", 8 * player.lev)
		end,
	effect = function()
			teleport_player(player.lev * 8)
			return true
		end,
}

PRAYER_TELEPORT_OTHER = add_prayer
{
	name = "Teleport Other",
	effect = function()
			local success, dir = get_aim_dir()
			if not success then return false end

			teleport_monster(dir)
			return true
		end,
}

PRAYER_TELEPORT_LEVEL = add_prayer
{
	name = "Teleport Level",
	effect = function()
			teleport_player_level()
			return true
		end,
}

PRAYER_WORD_OF_RECALL = add_prayer
{
	name = "Word of Recall",
	effect = function()
			set_recall()
			return true
		end,
}

PRAYER_ALTER_REALITY = add_prayer
{
	name = "Alter Reality",
	effect = function()
			msg_print("The world changes!")
			player.leaving = true
			return true
		end,
}


-----------------------------------------------------------
-- Add spells to the prayer books

-- Beginners Handbook
add_book(prayers, 0,
         {PRAYER_DETECT_EVIL,
          PRAYER_CURE_LIGHT_WOUNDS,
          PRAYER_BLESS,
          PRAYER_REMOVE_FEAR,
          PRAYER_CALL_LIGHT,
          PRAYER_FIND_TRAPS,
          PRAYER_DETECT_DOORS_STAIRS,
          PRAYER_SLOW_POISON})

-- Words of Wisdom
add_book(prayers, 1,
         {PRAYER_SCARE_MONSTER,
          PRAYER_PORTAL,
          PRAYER_CURE_SERIOUS_WOUNDS,
          PRAYER_CHANT,
          PRAYER_SANCTUARY,
          PRAYER_SATISFY_HUNGER,
          PRAYER_REMOVE_CURSE,
          PRAYER_RESIST_HEAT_COLD})

-- Chants and Blessings
add_book(prayers, 2,
         {PRAYER_NEUTRALIZE_POISON,
          PRAYER_ORB_OF_DRAINING,
          PRAYER_CURE_CRITICAL_WOUNDS,
          PRAYER_SENSE_INVISIBLE,
          PRAYER_PROTECTION_FROM_EVIL,
          PRAYER_EARTHQUAKE,
          PRAYER_SENSE_SURROUNDINGS,
          PRAYER_CURE_MORTAL_WOUNDS,
          PRAYER_TURN_UNDEAD})

-- Exorcism and Dispelling
add_book(prayers, 3,
         {PRAYER_PRAYER,
          PRAYER_DISPEL_UNDEAD,
          PRAYER_HEAL,
          PRAYER_DISPEL_EVIL,
          PRAYER_GLYPH_OF_WARDING,
          PRAYER_HOLY_WORD})

-- Ethereal Openings
add_book(prayers, 4,
         {PRAYER_BLINK,
          PRAYER_TELEPORT_SELF,
          PRAYER_TELEPORT_OTHER,
          PRAYER_TELEPORT_LEVEL,
          PRAYER_WORD_OF_RECALL,
          PRAYER_ALTER_REALITY})

-- Godly Insights
add_book(prayers, 5,
         {PRAYER_DETECT_MONSTERS,
          PRAYER_DETECTION,
          PRAYER_PERCEPTION,
          PRAYER_PROBING,
          PRAYER_CLAIRVOYANCE})

-- Purifications and Healing
add_book(prayers, 6,
         {PRAYER_CURE_SERIOUS_WOUNDS2,
          PRAYER_CURE_MORTAL_WOUNDS2,
          PRAYER_HEALING,
          PRAYER_RESTORATION,
          PRAYER_REMEMBRANCE})

-- Holy Infusions
add_book(prayers, 7,
         {PRAYER_UNBARRING_WAYS,
          PRAYER_RECHARGING,
          PRAYER_DISPEL_CURSE,
          PRAYER_ENCHANT_WEAPON,
          PRAYER_ENCHANT_ARMOUR,
          PRAYER_ELEMENTAL_BRAND})

-- Wrath of God
add_book(prayers, 8,
         {PRAYER_DISPEL_UNDEAD2,
          PRAYER_DISPEL_EVIL2,
          PRAYER_BANISH_EVIL,
          PRAYER_WORD_OF_DESTRUCTION,
          PRAYER_ANNIHILATION})


-----------------------------------------------------------
-- Hooks for spellcasting

function get_spell_index_hook(object, index)
	local book = magic_schools[object.tval].books[object.sval]
	return get_spell_index(book, index)
end


function get_spell_info_hook(tval, index)
	return get_spell_info(magic_schools[tval], index)
end


function get_spell_name_hook(tval, index)
	return get_spell_name(magic_schools[tval], index)
end


function cast_spell_hook(tval, index)
	return cast_spell(magic_schools[tval], index)
end


-- Add event handlers
add_event_handler("get_spell_index", get_spell_index_hook)
add_event_handler("get_spell_info", get_spell_info_hook)
add_event_handler("get_spell_name", get_spell_name_hook)
add_event_handler("cast_spell", cast_spell_hook)
