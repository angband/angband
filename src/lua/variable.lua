-- tolua: variable class
-- Written by Waldemar Celes
-- TeCGraf/PUC-Rio
-- Jul 1998
-- $Id: variable.lua,v 1.2 2001/10/28 16:18:38 angband Exp $

-- This code is free software; you can redistribute it and/or modify it.
-- The software provided hereunder is on an "as is" basis, and
-- the author has no obligation to provide maintenance, support, updates,
-- enhancements, or modifications. 


-- Variable class
-- Represents a extern variable or a public member of a class.
-- Stores all fields present in a declaration.
classVariable = {
 _base = classDeclaration,
}

settag(classVariable,tolua_tag)

-- Print method
function classVariable:print (ident,close)
 print(ident.."Variable{")
 print(ident.." mod  = '"..self.mod.."',")
 print(ident.." type = '"..self.type.."',")
 print(ident.." ptr  = '"..self.ptr.."',")
 print(ident.." name = '"..self.name.."',")
 print(ident.." def  = '"..self.def.."',")
 print(ident.." ret  = '"..self.ret.."',")
 print(ident.."}"..close)
end

-- get variable value
function classVariable:getvalue (class,static)
 if class and static then
  return class..'::'..self.name
 elseif class then
  return 'self->'..self.name
 else
  return self.name
 end
end

-- Write binding functions
function classVariable:supcode ()
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

 -- declare self, if the case
 local _,_,static = strfind(self.mod,'^%s*(static)')
 if class and static==nil then
  output(' ',class,'*','self = ')
  output('(',class,'*) ')
  output('tolua_getusertype(tolua_S,1,0);')
 elseif static then
  _,_,self.mod = strfind(self.mod,'^%s*static%s%s*(.*)')
 end


 -- check self value
 if class and static==nil then
  output('  if (!self) TOLUA_ERR_SELF;');
 end

 -- return value
 local t,ct = isbasic(self.type)
 if t then
  output('  tolua_push'..t..'(tolua_S,(',ct,')'..self:getvalue(class,static)..');')
 else
  if self.ptr == '&' or self.ptr == '' then
   output('  tolua_pushusertype(tolua_S,(void*)&'..self:getvalue(class,static)..',',self.tag,');')
  else
   output('  tolua_pushusertype(tolua_S,(void*)'..self:getvalue(class,static)..',',self.tag,');')
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

  -- declare self, if the case
  local narg=1
  if class and static==nil then
   output(' ',class,'*','self = ')
   output('(',class,'*) ')
   output('tolua_getusertype(tolua_S,1,0);')
   -- check self value
   output('  if (!self) TOLUA_ERR_SELF;');
   narg = narg+1
  elseif static then
   _,_,self.mod = strfind(self.mod,'^%s*static%s%s*(.*)')
   narg = narg+1
  end

  -- check type
  output('  if (!'..self:outchecktype(narg)..')')
  output('   TOLUA_ERR_ASSIGN;')
 
  -- assign value
  local ptr = ''
  if self.ptr~='' then ptr = '*' end
  output(' ')
  if class and static then
   output(class..'::'..self.name)
  elseif class then
   output('self->'..self.name)
  else
   output(self.name)
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
   output('tolua_get'..t,'(tolua_S,',narg,',',def,'));')
  else
   output('tolua_getusertype(tolua_S,',narg,',',def,'));')
  end
  output(' return 0;')
  output('}')
  output('\n')
 end 

end

function classVariable:register ()
 local parent = self:inclass() or self:inmodule()
 if parent then
  if self.csetname then
   output(' tolua_tablevar(tolua_S,"'..parent..'","'..self.lname..'",'..self.cgetname..','..self.csetname..');')
  else
   output(' tolua_tablevar(tolua_S,"'..parent..'","'..self.lname..'",'..self.cgetname..',NULL);')
  end
 else
  if self.csetname then
   output(' tolua_globalvar(tolua_S,"'..self.lname..'",'..self.cgetname..','..self.csetname..');')
  else
   output(' tolua_globalvar(tolua_S,"'..self.lname..'",'..self.cgetname..',NULL);')
  end
 end
end

function classVariable:unregister ()
 if self:inclass()==nil and self:inmodule()==nil then
  output(' lua_getglobals(tolua_S);') 
  output(' lua_pushstring(tolua_S,"',self.lname,'"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);')
  output(' lua_pop(tolua_S,1);')
 end
end 
 

-- Internal constructor
function _Variable (t)
 t._base = classVariable
 settag(t,tolua_tag)
 append(t)
 return t
end

-- Constructor
-- Expects a string representing the variable declaration.
function Variable (s)
 return _Variable (Declaration(s,'var'))
end


