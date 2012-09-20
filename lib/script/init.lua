-- Redirect error messages to Angband's msg_print()
_ALERT = function(text)
	msg_print(text)
	message_flush()
end

-- Detect access to undefined global variables
function safe_getglobal(x)
	local v = rawget(globals(), x)

	if v then
		return v
	else
		error("undefined global variable " .. x)
	end
end

settagmethod(tag(nil), "getglobal", safe_getglobal)

script_do_file(build_script_path("event.lua"))
script_do_file(build_script_path("object.lua"))
script_do_file(build_script_path("spell.lua"))
script_do_file(build_script_path("store.lua"))
script_do_file(build_script_path("birth.lua"))
