-- Use objects

function eat_food(object)
	local ident = FALSE

	if object.sval == SV_FOOD_RATION or
	   object.sval == SV_FOOD_BISCUIT or
	   object.sval == SV_FOOD_JERKY or
	   object.sval == SV_FOOD_SLIME_MOLD or
	   object.sval == SV_FOOD_PINT_OF_ALE or
	   object.sval == SV_FOOD_PINT_OF_WINE then
		msg_print("That tastes good.")
		ident = TRUE
	elseif object.sval == SV_FOOD_WAYBREAD then
		msg_print("That tastes good.")
		set_poisoned(0)
		hp_player(damroll(4, 8))
		ident = TRUE
	elseif object.sval == SV_FOOD_RESTORING then
		if do_res_stat(A_STR) then ident = TRUE end
		if do_res_stat(A_INT) then ident = TRUE end
		if do_res_stat(A_WIS) then ident = TRUE end
		if do_res_stat(A_DEX) then ident = TRUE end
		if do_res_stat(A_CON) then ident = TRUE end
		if do_res_stat(A_CHR) then ident = TRUE end
	elseif object.sval == SV_FOOD_RESTORE_CON then
		if do_res_stat(A_CON) then ident = TRUE end
	elseif object.sval == SV_FOOD_RESTORE_STR then
		if do_res_stat(A_STR) then ident = TRUE end
	elseif object.sval == SV_FOOD_CURE_SERIOUS then
		if hp_player(damroll(4, 8)) then ident = TRUE end
	elseif object.sval == SV_FOOD_CURE_CONFUSION then
		if set_confused(0) then ident = TRUE end
	elseif object.sval == SV_FOOD_CURE_PARANOIA then
		if set_afraid(0) then ident = TRUE end
	elseif object.sval == SV_FOOD_CURE_BLINDNESS then
		if set_blind(0) then ident = TRUE end
	elseif object.sval == SV_FOOD_CURE_POISON then
		if set_poisoned(0) then ident = TRUE end
	elseif object.sval == SV_FOOD_DISEASE then
		take_hit(damroll(10, 10), "poisonous food")
		do_dec_stat(A_STR)
		ident = TRUE
	elseif object.sval == SV_FOOD_UNHEALTH then
		take_hit(damroll(10, 10), "poisonous food")
		do_dec_stat(A_CON)
		ident = TRUE
	elseif object.sval == SV_FOOD_NAIVETY then
		take_hit(damroll(8, 8), "poisonous food")
		do_dec_stat(A_WIS)
		ident = TRUE
	elseif object.sval == SV_FOOD_STUPIDITY then
		take_hit(damroll(8, 8), "poisonous food")
		do_dec_stat(A_INT)
		ident = TRUE
	elseif object.sval == SV_FOOD_SICKNESS then
		take_hit(damroll(6, 6), "poisonous food")
		do_dec_stat(A_CON)
		ident = TRUE
	elseif object.sval == SV_FOOD_WEAKNESS then
		take_hit(damroll(6, 6), "poisonous food")
		do_dec_stat(A_STR)
		ident = TRUE
	elseif object.sval == SV_FOOD_PARALYSIS then
		if not player.free_act then
			if set_paralyzed(player.paralyzed + rand_int(10) + 10) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_FOOD_HALLUCINATION then
		if not player.resist_chaos then
			if set_image(player.image + rand_int(250) + 250) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_FOOD_CONFUSION then
		if  not player.resist_confu then
			if set_confused(player.confused + rand_int(10) + 10) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_FOOD_PARANOIA then
		if not player.resist_fear then
			if set_afraid(player.afraid + rand_int(10) + 10) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_FOOD_POISON then
		if not (player.resist_pois or player.oppose_pois) then
			if set_poisoned(player.poisoned + rand_int(10) + 10) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_FOOD_BLINDNESS then
		if not player.resist_blind then
			if set_blind(player.blind + rand_int(200) + 200) then
				ident = TRUE
			end
		end
	end

	-- Food can feed the player
	set_food(player.food + object.pval)

	return ident, TRUE
end


function quaff_potion(object)
	local ident = FALSE

	if object.sval == SV_POTION_WATER or
	   object.sval == SV_POTION_APPLE_JUICE or
	   object.sval == SV_POTION_SLIME_MOLD then
		msg_print("You feel less thirsty.")
		ident = TRUE
	elseif object.sval == SV_POTION_SLOWNESS then
		if set_slow(player.slow + randint(25) + 15) then ident = TRUE end
	elseif object.sval == SV_POTION_SALT_WATER then
		msg_print("The potion makes you vomit!")
		set_food(PY_FOOD_STARVE - 1)
		set_poisoned(0)
		set_paralyzed(player.paralyzed + 4)
		ident = TRUE
	elseif object.sval == SV_POTION_POISON then
		if not (player.resist_pois or player.oppose_pois) then
			if set_poisoned(player.poisoned + rand_int(15) + 10) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_POTION_BLINDNESS then
		if not player.resist_blind then
			if set_blind(player.blind + rand_int(100) + 100) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_POTION_CONFUSION then
		if not player.resist_confu then
			if set_confused(player.confused + rand_int(20) + 15) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_POTION_SLEEP then
		if not player.free_act then
			if set_paralyzed(player.paralyzed + rand_int(4) + 4) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_POTION_LOSE_MEMORIES then
		if not player.hold_life and (player.exp > 0) then
			msg_print("You feel your memories fade.")
			lose_exp(player.exp / 4)
			ident = TRUE
		end
	elseif object.sval == SV_POTION_RUINATION then
		msg_print("Your nerves and muscles feel weak and lifeless!")
		take_hit(damroll(10, 10), "a potion of Ruination")
		dec_stat(A_DEX, 25, TRUE)
		dec_stat(A_WIS, 25, TRUE)
		dec_stat(A_CON, 25, TRUE)
		dec_stat(A_STR, 25, TRUE)
		dec_stat(A_CHR, 25, TRUE)
		dec_stat(A_INT, 25, TRUE)
		ident = TRUE
	elseif object.sval == SV_POTION_DEC_STR then
		if do_dec_stat(A_STR) then ident = TRUE end
	elseif object.sval == SV_POTION_DEC_INT then
		if do_dec_stat(A_INT) then ident = TRUE end
	elseif object.sval == SV_POTION_DEC_WIS then
		if do_dec_stat(A_WIS) then ident = TRUE end
	elseif object.sval == SV_POTION_DEC_DEX then
		if do_dec_stat(A_DEX) then ident = TRUE end
	elseif object.sval == SV_POTION_DEC_CON then
		if do_dec_stat(A_CON) then ident = TRUE end
	elseif object.sval == SV_POTION_DEC_CHR then
		if do_dec_stat(A_CHR) then ident = TRUE end
	elseif object.sval == SV_POTION_DETONATIONS then
		msg_print("Massive explosions rupture your body!")
		take_hit(damroll(50, 20), "a potion of Detonation")
		set_stun(player.stun + 75)
		set_cut(player.cut + 5000)
		ident = TRUE
	elseif object.sval == SV_POTION_DEATH then
		msg_print("A feeling of Death flows through your body.")
		take_hit(5000, "a potion of Death")
		ident = TRUE
	elseif object.sval == SV_POTION_INFRAVISION then
		if set_tim_infra(player.tim_infra + 100 + randint(100)) then
			ident = TRUE
		end
	elseif object.sval == SV_POTION_DETECT_INVIS then
		if set_tim_invis(player.tim_invis + 12 + randint(12)) then
			ident = TRUE
		end
	elseif object.sval == SV_POTION_SLOW_POISON then
		if set_poisoned(player.poisoned / 2) then ident = TRUE end
	elseif object.sval == SV_POTION_CURE_POISON then
		if set_poisoned(0) then ident = TRUE end
	elseif object.sval == SV_POTION_BOLDNESS then
		if set_afraid(0) then ident = TRUE end
	elseif object.sval == SV_POTION_SPEED then
		if not player.fast then
			if set_fast(randint(25) + 15) then ident = TRUE end
		else
			set_fast(player.fast + 5)
		end
	elseif object.sval == SV_POTION_RESIST_HEAT then
		if set_oppose_fire(player.oppose_fire + randint(10) + 10) then
			ident = TRUE
		end
	elseif object.sval == SV_POTION_RESIST_COLD then
		if set_oppose_cold(player.oppose_cold + randint(10) + 10) then
			ident = TRUE
		end
	elseif object.sval == SV_POTION_HEROISM then
		if hp_player(10) then ident = TRUE end
		if set_afraid(0) then ident = TRUE end
		if set_hero(player.hero + randint(25) + 25) then ident = TRUE end
	elseif object.sval == SV_POTION_BERSERK_STRENGTH then
		if hp_player(30) then ident = TRUE end
		if set_afraid(0) then ident = TRUE end
		if set_shero(player.shero + randint(25) + 25) then ident = TRUE end
	elseif object.sval == SV_POTION_CURE_LIGHT then
		if hp_player(damroll(2, 8)) then ident = TRUE end
		if set_blind(0) then ident = TRUE end
		if set_cut(player.cut - 10) then ident = TRUE end
	elseif object.sval == SV_POTION_CURE_SERIOUS then
		if hp_player(damroll(4, 8)) then ident = TRUE end
		if set_blind(0) then ident = TRUE end
		if set_confused(0) then ident = TRUE end
		if set_cut((player.cut / 2) - 50) then ident = TRUE end
	elseif object.sval == SV_POTION_CURE_CRITICAL then
		if hp_player(damroll(6, 8)) then ident = TRUE end
		if set_blind(0) then ident = TRUE end
		if set_confused(0) then ident = TRUE end
		if set_poisoned(0) then ident = TRUE end
		if set_stun(0) then ident = TRUE end
		if set_cut(0) then ident = TRUE end
	elseif object.sval == SV_POTION_HEALING then
		if hp_player(300) then ident = TRUE end
		if set_blind(0) then ident = TRUE end
		if set_confused(0) then ident = TRUE end
		if set_poisoned(0) then ident = TRUE end
		if set_stun(0) then ident = TRUE end
		if set_cut(0) then ident = TRUE end
	elseif object.sval == SV_POTION_STAR_HEALING then
		if hp_player(1200) then ident = TRUE end
		if set_blind(0) then ident = TRUE end
		if set_confused(0) then ident = TRUE end
		if set_poisoned(0) then ident = TRUE end
		if set_stun(0) then ident = TRUE end
		if set_cut(0) then ident = TRUE end
	elseif object.sval == SV_POTION_LIFE then
		msg_print("You feel life flow through your body!")
		restore_level()
		set_poisoned(0)
		set_blind(0)
		set_confused(0)
		set_image(0)
		set_stun(0)
		set_cut(0)
		do_res_stat(A_STR)
		do_res_stat(A_CON)
		do_res_stat(A_DEX)
		do_res_stat(A_WIS)
		do_res_stat(A_INT)
		do_res_stat(A_CHR)

		-- Recalculate max. hitpoints
		update_stuff()

		hp_player(5000)

		ident = TRUE
	elseif object.sval == SV_POTION_RESTORE_MANA then
		if player.csp < player.msp then
			player.csp = player.msp
			player.csp_frac = 0
			msg_print("Your feel your head clear.")
			player.redraw = bOr(player.redraw, PR_MANA)
			player.window = bOr(player.window, bOr(PW_PLAYER_0, PW_PLAYER_1))
			ident = TRUE
		end
	elseif object.sval == SV_POTION_RESTORE_EXP then
		if restore_level() then ident = TRUE end
	elseif object.sval == SV_POTION_RES_STR then
		if do_res_stat(A_STR) then ident = TRUE end
	elseif object.sval == SV_POTION_RES_INT then
		if do_res_stat(A_INT) then ident = TRUE end
	elseif object.sval == SV_POTION_RES_WIS then
		if do_res_stat(A_WIS) then ident = TRUE end
	elseif object.sval == SV_POTION_RES_DEX then
		if do_res_stat(A_DEX) then ident = TRUE end
	elseif object.sval == SV_POTION_RES_CON then
		if do_res_stat(A_CON) then ident = TRUE end
	elseif object.sval == SV_POTION_RES_CHR then
		if do_res_stat(A_CHR) then ident = TRUE end
	elseif object.sval == SV_POTION_INC_STR then
		if do_inc_stat(A_STR) then ident = TRUE end
	elseif object.sval == SV_POTION_INC_INT then
		if do_inc_stat(A_INT) then ident = TRUE end
	elseif object.sval == SV_POTION_INC_WIS then
		if do_inc_stat(A_WIS) then ident = TRUE end
	elseif object.sval == SV_POTION_INC_DEX then
		if do_inc_stat(A_DEX) then ident = TRUE end
	elseif object.sval == SV_POTION_INC_CON then
		if do_inc_stat(A_CON) then ident = TRUE end
	elseif object.sval == SV_POTION_INC_CHR then
		if do_inc_stat(A_CHR) then ident = TRUE end
	elseif object.sval == SV_POTION_AUGMENTATION then
		if do_inc_stat(A_STR) then ident = TRUE end
		if do_inc_stat(A_INT) then ident = TRUE end
		if do_inc_stat(A_WIS) then ident = TRUE end
		if do_inc_stat(A_DEX) then ident = TRUE end
		if do_inc_stat(A_CON) then ident = TRUE end
		if do_inc_stat(A_CHR) then ident = TRUE end
	elseif object.sval == SV_POTION_ENLIGHTENMENT then
		msg_print("An image of your surroundings forms in your mind...")
		wiz_lite()
		ident = TRUE
	elseif object.sval == SV_POTION_STAR_ENLIGHTENMENT then
		msg_print("You begin to feel more enlightened...")
		message_flush()
		wiz_lite()
		do_inc_stat(A_INT)
		do_inc_stat(A_WIS)
		detect_traps()
		detect_doors()
		detect_stairs()
		detect_treasure()
		detect_objects_gold()
		detect_objects_normal()
		identify_pack()
		self_knowledge()
		ident = TRUE
	elseif object.sval == SV_POTION_SELF_KNOWLEDGE then
		msg_print("You begin to know yourself a little better...")
		message_flush()
		self_knowledge()
		ident = TRUE
	elseif object.sval == SV_POTION_EXPERIENCE then
		if (player.exp < PY_MAX_EXP) then
			local ee = (player.exp / 2) + 10
			if (ee > 100000) then ee = 100000 end
			msg_print("You feel more experienced.")
			gain_exp(ee)
			ident = TRUE
		end
	end

	-- Potions can feed the player
	set_food(player.food + object.pval)

	return ident, TRUE
end


function read_scroll(object)
	local ident = FALSE

	-- Assume the scroll will get used up
	local used_up = TRUE

	if object.sval == SV_SCROLL_DARKNESS then
		if not player.resist_blind then
			set_blind(player.blind + 3 + randint(5))
		end
		if unlite_area(10, 3) then ident = TRUE end
	elseif object.sval == SV_SCROLL_AGGRAVATE_MONSTER then
		msg_print("There is a high pitched humming noise.")
		aggravate_monsters(0)
		ident = TRUE
	elseif object.sval == SV_SCROLL_CURSE_ARMOR then
		if curse_armor() then ident = TRUE end
	elseif object.sval == SV_SCROLL_CURSE_WEAPON then
		if curse_weapon() then ident = TRUE end
	elseif object.sval == SV_SCROLL_SUMMON_MONSTER then
		for k = 0, randint(3) do
			if summon_specific(player.py, player.px, player.depth, 0) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_SCROLL_SUMMON_UNDEAD then
		for k = 0, randint(3) do
			if summon_specific(player.py, player.px, player.depth, SUMMON_UNDEAD) then
				ident = TRUE
			end
		end
	elseif object.sval == SV_SCROLL_TRAP_CREATION then
		if trap_creation() then ident = TRUE end
	elseif object.sval == SV_SCROLL_PHASE_DOOR then
		teleport_player(10)
		ident = TRUE
	elseif object.sval == SV_SCROLL_TELEPORT then
		teleport_player(100)
		ident = TRUE
	elseif object.sval == SV_SCROLL_TELEPORT_LEVEL then
		teleport_player_level()
		ident = TRUE
	elseif object.sval == SV_SCROLL_WORD_OF_RECALL then
		set_recall()
		ident = TRUE
	elseif object.sval == SV_SCROLL_IDENTIFY then
		ident = TRUE
		if not ident_spell() then used_up = FALSE end
	elseif object.sval == SV_SCROLL_STAR_IDENTIFY then
		ident = TRUE
		if not identify_fully() then used_up = FALSE end
	elseif object.sval == SV_SCROLL_REMOVE_CURSE then
		if remove_curse() then
			msg_print("You feel as if someone is watching over you.")
			ident = TRUE
		end
	elseif object.sval == SV_SCROLL_STAR_REMOVE_CURSE then
		remove_all_curse()
		ident = TRUE
	elseif object.sval == SV_SCROLL_ENCHANT_ARMOR then
		ident = TRUE
		if not enchant_spell(0, 0, 1) then used_up = FALSE end
	elseif object.sval == SV_SCROLL_ENCHANT_WEAPON_TO_HIT then
		if not enchant_spell(1, 0, 0) then used_up = FALSE end
		ident = TRUE
	elseif object.sval == SV_SCROLL_ENCHANT_WEAPON_TO_DAM then
		if not enchant_spell(0, 1, 0) then used_up = FALSE end
		ident = TRUE
	elseif object.sval == SV_SCROLL_STAR_ENCHANT_ARMOR then
		if not enchant_spell(0, 0, randint(3) + 2) then used_up = FALSE end
		ident = TRUE
	elseif object.sval == SV_SCROLL_STAR_ENCHANT_WEAPON then
		if not enchant_spell(randint(3), randint(3), 0) then used_up = FALSE end
		ident = TRUE
	elseif object.sval == SV_SCROLL_RECHARGING then
		if not recharge(60) then used_up = FALSE end
		ident = TRUE
	elseif object.sval == SV_SCROLL_LIGHT then
		if lite_area(damroll(2, 8), 2) then ident = TRUE end		
	elseif object.sval == SV_SCROLL_MAPPING then
		map_area()
		ident = TRUE
	elseif object.sval == SV_SCROLL_DETECT_GOLD then
		if detect_treasure() then ident = TRUE end
		if detect_objects_gold() then ident = TRUE end
	elseif object.sval == SV_SCROLL_DETECT_ITEM then
		if detect_objects_normal() then ident = TRUE end
	elseif object.sval == SV_SCROLL_DETECT_TRAP then
		if detect_traps() then ident = TRUE end
	elseif object.sval == SV_SCROLL_DETECT_DOOR then
		if detect_doors() then ident = TRUE end
		if detect_stairs() then ident = TRUE end
	elseif object.sval == SV_SCROLL_DETECT_INVIS then
		if detect_monsters_invis() then ident = TRUE end
	elseif object.sval == SV_SCROLL_SATISFY_HUNGER then
		if set_food(PY_FOOD_MAX - 1) then ident = TRUE end
	elseif object.sval == SV_SCROLL_BLESSING then
		if set_blessed(player.blessed + randint(12) + 6) then ident = TRUE end
	elseif object.sval == SV_SCROLL_HOLY_CHANT then
		if set_blessed(player.blessed + randint(24) + 12) then ident = TRUE end
	elseif object.sval == SV_SCROLL_HOLY_PRAYER then
		if set_blessed(player.blessed + randint(48) + 24) then ident = TRUE end
	elseif object.sval == SV_SCROLL_MONSTER_CONFUSION then
		if player.confusing == 0 then
			msg_print("Your hands begin to glow.")
			player.confusing = TRUE
			ident = TRUE
		end
	elseif object.sval == SV_SCROLL_PROTECTION_FROM_EVIL then
		local k = 3 * player.lev
		if set_protevil(player.protevil + randint(25) + k) then ident = TRUE end
	elseif object.sval == SV_SCROLL_RUNE_OF_PROTECTION then
		warding_glyph()
		ident = TRUE
	elseif object.sval == SV_SCROLL_TRAP_DOOR_DESTRUCTION then
		if destroy_doors_touch() then ident = TRUE end
	elseif object.sval == SV_SCROLL_STAR_DESTRUCTION then
		destroy_area(player.py, player.px, 15, TRUE)
		ident = TRUE
	elseif object.sval == SV_SCROLL_DISPEL_UNDEAD then
		if dispel_undead(60) then ident = TRUE end
	elseif object.sval == SV_SCROLL_GENOCIDE then
		genocide()
		ident = TRUE
	elseif object.sval == SV_SCROLL_MASS_GENOCIDE then
		mass_genocide()
		ident = TRUE
	elseif object.sval == SV_SCROLL_ACQUIREMENT then
		acquirement(player.py, player.px, 1, TRUE)
		ident = TRUE
	elseif object.sval == SV_SCROLL_STAR_ACQUIREMENT then
		acquirement(player.py, player.px, randint(2) + 1, TRUE)
		ident = TRUE
	end

	return ident, used_up
end


function use_object_hook(object)
	local ident = FALSE
	local used = FALSE

	if object.tval == TV_FOOD then
		ident, used = eat_food(object)
	elseif object.tval == TV_POTION then
		ident, used = quaff_potion(object)
	elseif object.tval == TV_SCROLL then
		ident, used = read_scroll(object)
	end

	return ident, used
end

