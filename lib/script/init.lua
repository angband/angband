-- Redirect error messages to Angband's msg_print()
_ALERT = function(text)
	msg_print(text)
	message_flush()
end

_TRACEBACK = function(text)
	msg_print(text)
	msg_print(debug.traceback())
	message_flush()
end

-- Detect access to undefined global variables
function safe_getglobal(table, key)
	local v = rawget(table, key)
	if v then return v end

	if old_getglobal then
		v = old_getglobal(table, key)
		if v then return v end
	end

	print("undefined variable " .. key)
end

old_getglobal = getmetatable(getfenv(0))["__index"]
getmetatable(getfenv(0))["__index"] = safe_getglobal

-- Load the modules
script_do_file(angband.build_script_path("event.lua"))
script_do_file(angband.build_script_path("player.lua"))
script_do_file(angband.build_script_path("object.lua"))
script_do_file(angband.build_script_path("spell.lua"))
script_do_file(angband.build_script_path("store.lua"))
script_do_file(angband.build_script_path("birth.lua"))
