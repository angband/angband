-- Player code


-- Stat Table (CON) -- base regeneration rate
local adj_con_fix =
{
	1, -- 3
	1, -- 4
	1, -- 5
	1, -- 6
	1, -- 7
	1, -- 8
	1, -- 9
	1, -- 10
	1, -- 11
	1, -- 12
	1, -- 13
	2, -- 14
	2, -- 15
	2, -- 16
	2, -- 17
	3, -- 18/00-18/09
	3, -- 18/10-18/19
	3, -- 18/20-18/29
	3, -- 18/30-18/39
	3, -- 18/40-18/49
	4, -- 18/50-18/59
	4, -- 18/60-18/69
	4, -- 18/70-18/79
	4, -- 18/80-18/89
	4, -- 18/90-18/99
	5, -- 18/100-18/109
	5, -- 18/110-18/119
	6, -- 18/120-18/129
	7, -- 18/130-18/139
	7, -- 18/140-18/149
	8, -- 18/150-18/159
	8, -- 18/160-18/169
	9, -- 18/170-18/179
	9, -- 18/180-18/189
	9, -- 18/190-18/199
	10, -- 18/200-18/209
	10, -- 18/210-18/219
	10  -- 18/220+
}


-- Take damage from poison
function poison_damage_hook()
	if player.poisoned > 0 then
		-- Take damage
		take_hit(1, "poison")

		-- Reduce poison effect over time
		-- Note: +1 because of Lua's 1-based arrays
		local con = player.stat_ind[A_CON + 1] + 1
		local adjust = adj_con_fix[con]
		set_poisoned(player.poisoned - adjust)
	end
end

-- ToDo: Using the "player_turn" event might be better.
add_event_handler("process_world", poison_damage_hook)
