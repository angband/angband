-- tolua: class class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: class.lua,v 1.1 2001/10/27 19:35:28 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications.


-- Class class
-- Represents a class definition.
-- Stores the following fields:
--    name = class name
--    base = class base, if any (only single inheritance is supported)
--    {i}  = list of members
classClass = {
 _base = classContainer,
 type = 'class',
 name = '',
 base = '',
}
settag(classClass,tolua_tag)


-- register class
function classClass:register ()
 output(' tolua_cclass(tolua_S,"'..self.name..'","'..self.base..'");')
 local i=1
 while self[i] do
  self[i]:register()
  i = i+1
 end
end

-- unregister class
function classClass:unregister ()
 output(' lua_pushnil(tolua_S); lua_setglobal(tolua_S,"'..self.name..'");')
end

-- output tags
function classClass:decltag ()
 self.itype,self.tag = tagvar(self.name);
 self.citype,self.ctag = tagvar(self.name,'const');
 local i=1
 while self[i] do
  self[i]:decltag()
  i = i+1
 end
end


-- Print method
function classClass:print (ident,close)
 print(ident.."Class{")
 print(ident.." name = '"..self.name.."',")
 print(ident.." base = '"..self.base.."';")
 local i=1
 while self[i] do
  self[i]:print(ident.." ",",")
  i = i+1
 end
 print(ident.."}"..close)
end

-- Internal constructor
function _Class (t)
 t._base = classClass
 settag(t,tolua_tag)
 append(t)
 return t
end

-- Constructor
-- Expects the name, the base and the body of the class.
function Class (n,p,b)
 local c = _Class(_Container{name=n, base=p})
 push(c)
 c:parse(strsub(b,2,strlen(b)-1)) -- eliminate braces
 pop()
end


