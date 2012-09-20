-- tolua: enumerate class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: enumerate.lua,v 1.1 2001/10/27 19:35:29 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 


-- Enumerate class
-- Represents enumeration
-- The following fields are stored:
--    {i} = list of constant names
classEnumerate = {
 _base = classFeature,
}
settag(classEnumerate,tolua_tag)

-- register enumeration
function classEnumerate:register ()
 local p = self:inclass() or self:inmodule()
 local i=1
 while self[i] do
  if p then
   if self:inclass() then
    output(' tolua_constant(tolua_S,"'..p..'","'..self.lnames[i]..'",'..p..'::'..self[i]..');')
   else
    output(' tolua_constant(tolua_S,"'..p..'","'..self.lnames[i]..'",'..self[i]..');')
   end
  else
   output(' tolua_constant(tolua_S,NULL,"'..self.lnames[i]..'",'..self[i]..');')
  end
  i = i+1
 end
end
-- register enumeration
function classEnumerate:unregister ()
 if self:inclass()==nil and self:inmodule()==nil then
  local i=1
  while self[i] do
   output(' lua_pushnil(tolua_S); lua_setglobal(tolua_S,"'..self.lnames[i]..'");')
   i = i+1
  end
 end
end

-- Print method
function classEnumerate:print (ident,close)
 print(ident.."Enumerate{")
 local i=1
 while self[i] do
  print(ident.." '"..self[i].."'("..self.lnames[i].."),")
  i = i+1
 end
 print(ident.."}"..close)
end

-- Internal constructor
function _Enumerate (t)
 t._base = classEnumerate
 settag(t,tolua_tag)
 append(t)
 return t
end

-- Constructor
-- Expects a string representing the enumerate body
function Enumerate (b)
 local t = split(strsub(b,2,-2),',') -- eliminate braces
 local i = 1
 local e = {n=0}
 while t[i] do
  local tt = split(t[i],'=')  -- discard initial value
  e.n = e.n + 1
  e[e.n] = tt[1]
  i = i+1
 end
 -- set lua names
 i  = 1
 e.lnames = {}
 while e[i] do
  local t = split(e[i],'@')
  e[i] = t[1]
  e.lnames[i] = t[2] or t[1]
  i = i+1
 end 
 return _Enumerate(e)
end


