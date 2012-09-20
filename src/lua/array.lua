-- tolua: array class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1999
-- $Id: array.lua,v 1.1 2001/10/27 19:35:28 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 


-- Array class
-- Represents a extern array variable or a public member of a class.
-- Stores all fields present in a declaration.
classArray = {
 _base = classDeclaration,
}

settag(classArray,tolua_tag)

-- Print method
function classArray:print (ident,close)
 print(ident.."Array{")
 print(ident.." mod  = '"..self.mod.."',")
 print(ident.." type = '"..self.type.."',")
 print(ident.." ptr  = '"..self.ptr.."',")
 print(ident.." name = '"..self.name.."',")
 print(ident.." def  = '"..self.def.."',")
 print(ident.." dim  = '"..self.dim.."',")
 print(ident.." ret  = '"..self.ret.."',")
 print(ident.."}"..close)
end

-- get variable value
function classArray:getvalue (class,static)
 if class and static then
  return class..'::'..self.name..'[toluaI_index]'
 elseif class then
  return 'self->'..self.name..'[toluaI_index]'
 else
  return self.name..'[toluaI_index]'
 end
end

-- Write binding functions
function classArray:supcode ()
 local class = self:inclass()

 -- get function ------------------------------------------------
 if class then
  output("/* get function:",self.name," of class ",class," */")
 else
  output("/* get function:",self.name," */")
 end
 self.cgetname = self:cfuncname("toluaI_get")
 output("static int",self.cgetname,"(lua_State* tolua_S)") 
 output("{")

 -- declare index
 output(' int toluaI_index;')

 -- declare self, if the case
 local _,_,static = strfind(self.mod,'^%s*(static)')
 if class and static==nil then
  output(' ',class,'*','self;')
  output(' lua_pushstring(tolua_S,".self");')
  output(' lua_rawget(tolua_S,1);')
  output(' self = ')
  output('(',class,'*) ')
  output('lua_touserdata(tolua_S,-1);')
 elseif static then
  _,_,self.mod = strfind(self.mod,'^%s*static%s%s*(.*)')
 end

 -- check index
 output(' if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))')
 output('  tolua_error(tolua_S,"invalid type in array indexing.");')
 output(' toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;')
 output(' if (toluaI_index<0 || toluaI_index>='..self.dim..')')
 output('  tolua_error(tolua_S,"array indexing out of range.");')

 -- return value
 local t,ct = isbasic(self.type)
 if t then
  output(' tolua_push'..t..'(tolua_S,(',ct,')'..self:getvalue(class,static)..');')
 else
  if self.ptr == '&' or self.ptr == '' then
   output(' tolua_pushusertype(tolua_S,(void*)&'..self:getvalue(class,static)..',',self.tag,');')
  else
   output(' tolua_pushusertype(tolua_S,(void*)'..self:getvalue(class,static)..',',self.tag,');')
  end
 end
 output(' return 1;')
 output('}')
 output('\n')

 -- set function ------------------------------------------------
 if not strfind(self.mod,'const') then
  if class then
   output("/* set function:",self.name," of class ",class," */")
  else
   output("/* set function:",self.name," */")
  end
  self.csetname = self:cfuncname("toluaI_set")
  output("static int",self.csetname,"(lua_State* tolua_S)")
  output("{")

  -- declare index
  output(' int toluaI_index;')

  -- declare self, if the case
  local _,_,static = strfind(self.mod,'^%s*(static)')
  if class and static==nil then
   output(' ',class,'*','self;')
   output(' lua_pushstring(tolua_S,".self");')
   output(' lua_rawget(tolua_S,1);')
   output(' self = ')
   output('(',class,'*) ')
   output('lua_touserdata(tolua_S,-1);')
  elseif static then
   _,_,self.mod = strfind(self.mod,'^%s*static%s%s*(.*)')
  end
 
  -- check index
  output(' if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))')
  output('  tolua_error(tolua_S,"invalid type in array indexing.");')
  output(' toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;')
  output(' if (toluaI_index<0 || toluaI_index>='..self.dim..')')
  output('  tolua_error(tolua_S,"array indexing out of range.");')

  -- assign value
  local ptr = ''
  if self.ptr~='' then ptr = '*' end
  output(' ')
  if class and static then
   output(class..'::'..self.name..'[toluaI_index]')
  elseif class then
   output('self->'..self.name..'[toluaI_index]')
  else
   output(self.name..'[toluaI_index]')
  end
  local t = isbasic(self.type)
  output(' = ')
  if not t and ptr=='' then output('*') end
  output('((',self.mod,self.type)
  if not t then
   output('*')
  end
  output(') ')
  local def = 0
  if self.def ~= '' then def = self.def end
  if t then
   output('tolua_get'..t,'(tolua_S,3,',def,'));')
  else
   output('tolua_getusertype(tolua_S,3,',def,'));')
  end
  output(' return 0;')
  output('}')
  output('\n')
 end 

end

function classArray:register ()
 local parent = self:inclass() or self:inmodule()
 if parent then
  if self.csetname then
   output(' tolua_tablearray(tolua_S,"'..parent..'","'..self.lname..'",'..self.cgetname..','..self.csetname..');')
  else
   output(' tolua_tablearray(tolua_S,"'..parent..'","'..self.lname..'",'..self.cgetname..',NULL);')
  end
 else
  if self.csetname then
   output(' tolua_globalarray(tolua_S,"'..self.lname..'",'..self.cgetname..','..self.csetname..');')
  else
   output(' tolua_globalarray(tolua_S,"'..self.lname..'",'..self.cgetname..',NULL);')
  end
 end
end

function classArray:unregister ()
 if self:inclass()==nil and self:inmodule()==nil then
  output(' lua_pushnil(tolua_S); lua_setglobal(tolua_S,"'..self.lname..'");') 
 end
end 
 

-- Internal constructor
function _Array (t)
 t._base = classArray
 settag(t,tolua_tag)
 append(t)
 return t
end

-- Constructor
-- Expects a string representing the variable declaration.
function Array (s)
 return _Array (Declaration(s,'var'))
end


