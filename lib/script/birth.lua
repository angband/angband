-- Player birth code


-- Equip the player after being born
function equip_player_birth()
	local o_idx
	local object

	-- Hack - Get a dummy object
	o_idx = o_pop()
	object = o_list[o_idx]

	-- Create some food
	object_prep(object, lookup_kind(TV_FOOD, SV_FOOD_RATION))
	object.number = rand_range(3, 7)
	object_aware(object)
	object_known(object)
	inven_carry(object)

	-- Create some torches
	object_prep(object, lookup_kind(TV_LITE, SV_LITE_TORCH))
	object.number = rand_range(3, 7)
	object.pval = rand_range(3, 7) * 500
	object_aware(object)
	object_known(object)
	inven_carry(object)

	-- Remove the dummy object
	delete_object_idx(o_idx)
end


add_event_handler("player_birth_done", equip_player_birth)
