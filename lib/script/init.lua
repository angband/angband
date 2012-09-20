-- Redirect error messages to Angband's msg_print()
_ALERT = function(text)
	msg_print(text)
end

dofile(build_script_path("object.lua"))
dofile(build_script_path("spell.lua"))

