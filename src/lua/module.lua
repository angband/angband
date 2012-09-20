-- tolua: module class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: module.lua,v 1.1 2001/10/27 19:35:29 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 



-- Module class
-- Represents module.
-- The following fields are stored:
--    {i} = list of objects in the module.
classModule = {
 _base = classContainer,
 type = 'module'
}
settag(classModule,tolua_tag)

-- register module
function classModule:register ()
 output(' tolua_module(tolua_S,"'..self.name..'");')
 local i=1
 while self[i] do
  self[i]:register()
  i = i+1
 end
end

-- unregister module
function classModule:unregister ()
 output(' lua_pushnil(tolua_S); lua_setglobal(tolua_S,"'..self.name..'");') 
end

-- Print method
function classModule:print (ident,close)
 print(ident.."Module{")
 print(ident.." name = '"..self.name.."';")
 local i=1
 while self[i] do
  self[i]:print(ident.." ",",")
  i = i+1
 end
 print(ident.."}"..close)
end

-- Internal constructor
function _Module (t)
 t._base = classModule
 settag(t,tolua_tag)
 append(t)
 return t
end

-- Constructor
-- Expects two string representing the module name and body.
function Module (n,b)
 local t = _Module(_Container{name=n})
 push(t)
 t:parse(strsub(b,2,strlen(b)-1)) -- eliminate braces
 pop()
 return t
end


